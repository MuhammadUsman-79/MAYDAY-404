#ifndef HISTORY_H
#define HISTORY_H
#include "Registry.h"

// One entry of the command log (node of a doubly linked list)
// next -> newer command, prev -> older command
struct LogNode{
    string action, callSign;
    LogNode* next;
    LogNode* prev;

    LogNode(string a, string c) {
        action= a;
        callSign= c;
        next= prev= NULL;
    }
};

// A saved copy of a flight's old state (node of a stack, built as a singly linked list)
// Taken BEFORE a change is made, so undo can put the old values back
struct Snapshot{
    string callSign;
    string oldRunway, oldSector;
    int oldPriority, oldCred;
    bool oldCleared, oldResolved, oldCyber;
    Snapshot* next; // points to the snapshot below it in the stack

    Snapshot(){
        oldPriority= 9;
        oldCred= 100;
        oldCleared= oldResolved= oldCyber= false;
        next= NULL;
    }
};

class CommandHistory{

    // Command log is doubly linked list
    LogNode* first;
    LogNode* last;

    // Undo stack: top is the newest snapshot (LIFO)
    Snapshot* top;

public:
    CommandHistory() {
        first= last= NULL;
        top= NULL;
    }

    // Frees every log node & every snapshot that was created with new (DMA)
    ~CommandHistory(){
        while(first!=NULL){
            LogNode* p= first;
            first= first->next;
            delete p;
        }

        while(top!=NULL){
            Snapshot* p= top;
            top= top->next;
            delete p;
        }
    }

    // The command log is doubly linked so the newest commands can be printed backward
    //Add a new command at the end of the log - O(1) because we keep the 'last' pointer
    void logCommand(string action, string id){
        LogNode* n= new LogNode(action, id);
        if(first==NULL) {
            first= n; // log was empty
        }
        else {
            last->next= n;
            n->prev= last;
        }
        last= n;
    }

    // Store the current state before changing a flight or countering cyber
    // This is the stack PUSH... the new snapshot goes on top - O(1)
    void pushSnapshot(Flight* f, bool cyber, int credibility){
        Snapshot* s= new Snapshot;
        s->oldCred= credibility;
        s->oldCyber= cyber;

        if(f!=NULL){
            s->callSign =f->callSign;
            s->oldRunway= f->assignedRunway;
            s->oldSector = f->currentSector;
            s->oldPriority= f->priorityLevel;
            s->oldCleared = f->cleared;
            s->oldResolved= f->resolved;
        }

        s->next= top;
        top= s;
    }

    // Undo is LIFO, restore the earlier state of the affected record
    // This is the stack POP... take the top snapshot and copy its old values back - O(1)
    // (finding the flight in the registry is a search, so that part depends on the registry)
    bool undo(Registration& registry, bool& cyber, int& credibility){
        if(top==NULL) {
            return false; // nothing to undo
        }

        // The flight may have been removed after the snapshot was taken
        if(!top->callSign.empty()  &&  registry.find(top->callSign)== NULL){
            cout<<"[ATC Error] Cannot undo: flight already removed."<<endl;
            return false;
        }

        Snapshot* s= top;
        top= top->next;
        credibility= s->oldCred; //passed by reference, so the caller's values change
        cyber= s->oldCyber;

        if(!s->callSign.empty()){
            Flight* f= registry.find(s->callSign);
            if(f!=NULL){
                f->assignedRunway= s->oldRunway;
                f->currentSector= s->oldSector;
                f->priorityLevel = s->oldPriority;
                f->cleared =s->oldCleared;
                f->resolved= s->oldResolved;
            }
        }

        delete s; // free the popped snapshot
        return true;
    }

    // Show only the newest command
    void displayLastCommand(){
        if(last==NULL) {
            cout<<"No commands yet."<<endl;
        }
        else {
            cout<<last->action<<" : "<<last->callSign<<endl;
        }
    }

    // Show the newest 'count' commands, walking backward using prev
    void displayHistory(int count){
        LogNode* p= last;
        while(p!=NULL && count>0){
            cout<<p->action<<" : "<<p->callSign<<endl;
            p= p->prev;
            count--;
        }
    }
};

#endif