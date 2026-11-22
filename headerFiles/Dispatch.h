#ifndef DISPATCH_H
#define DISPATCH_H
#include "Flight.h"

//One node of the normal landing queue (singly linked list node)
struct QueueNode{
    Flight* flight;
    QueueNode* next;

    QueueNode(Flight* f) {
        flight= f;
        next =NULL;
    }
};

class EmergencyLanding{

    QueueNode* front;
    QueueNode* rear;

    //Emergency priority queue (binary min-heap, stored in an array)
    Flight* heap[300];
    int heapSize;

    // Decides which flight is more urgent.
     
    bool earlier(Flight* a, Flight* b){

        if(a->priorityLevel != b->priorityLevel){ // 1) lower priorityLevel wins
            return (a->priorityLevel < b->priorityLevel);
        }

        if(a->arrivalTick != b->arrivalTick){ //2) earlier arrivalTick wins
            return a->arrivalTick < b->arrivalTick;
        }

        return a->callSign < b->callSign; //3) smaller callSign wins
    }

    // Swaps two positions in the heap array
    void swapFlights(int a, int b){
        Flight* temp= heap[a];
        heap[a]= heap[b];
        heap[b]= temp;
    }

    // Heapify up: moves the element at index i upward until its parent is more urgent
    void up(int i){

        while(i>0){
            int p= (i-1)/2; // parent index
            if(!earlier(heap[i], heap[p])){
                break;
            }

            swapFlights(i, p);
            i= p;
        }
    }

    // Heapify down: moves the element at index i downward until both children are less urgent
    void down(int i){
        while(true){
            int left= i*2+1;
            int right= i*2+2;
            int best= i;

            if(left<heapSize && earlier(heap[left], heap[best])) {
                best= left;
            }
            if(right<heapSize && earlier(heap[right], heap[best])) {
                best= right;
            }
            if(best==i){
                break;
            }

            swapFlights(i, best);
            i= best;
        }
    }

public:
    EmergencyLanding() {
        front=rear=NULL;
        heapSize=0;
    }

    //the heap only holds pointers, no new memory
    ~EmergencyLanding(){
        while(front!=NULL){
            QueueNode* n= front;
            front= front->next;
            delete n;
        }
    }

    bool isEmptyNormal() {
        return front==NULL;
    }

    bool isEmptyHeap() {
        return heapSize ==0;
    }

    //=== Normal queue operations ===

    // Add a flight at the rear
    void enqueueNormal(Flight* f){
        if(f==NULL) {
            return;
        }

        QueueNode* n= new QueueNode(f);
        if(rear==NULL){
            front=n;
        }

        else rear->next=n;
        rear=n;
    }

    // Remove and return the flight at the front
    Flight* dequeueNormal(){
        if(front==NULL) {
            return NULL;
        }

        QueueNode* n= front;
        Flight* f= n->flight;
        front= front->next;
        if(front==NULL) rear=NULL;
        delete n;
        return f;
    }

    Flight* frontNormal() {
        return front==NULL ? NULL : front->flight;
    }

    //=== Emergency heap operations ===
    // Insert at the end of the array, then bubble up
    void insertEmergency(Flight* f){
        if(f==NULL || heapSize>=300) {
            return;
        }

        heap[heapSize]= f;
        up(heapSize);
        heapSize++;
    }

    // Look at the most urgent flight without removing it
    Flight* peekMin() {
        return heapSize==0 ? NULL : heap[0];
    }

    // Remove the most urgent flight
    //move last element to the root, then sift down
    Flight* extractMin(){
        if(heapSize==0) {
            return NULL;
        }

        Flight* f= heap[0];
        heapSize--;
        if(heapSize>0) {
            heap[0]= heap[heapSize];
            down(0);
        }
        return f;
    }

    // Remove all references before a flight record is deleted or re prioritized.
    void removeFlight(Flight* f){

        // 1) remove from the normal queue
        QueueNode* p= front;
        QueueNode* prev= NULL;
        while(p!=NULL){

            if(p->flight == f){
                QueueNode* del= p;
                if(prev==NULL) {
                    front= p->next;
                }
                else{
                    prev->next= p->next;
                }
                if(rear==p) rear= prev;
                p= p->next;
                delete del;
            }

            else {//check next if not found
                prev= p;
                p= p->next;
            }
        }

        // 2) remove from the heap, then rebuild the heap order
        for(int i=0; i<heapSize;){

            if(heap[i]==f){
                heapSize--;
                if(i<heapSize){
                    heap[i]= heap[heapSize];
                    for(int k=heapSize/2; k>=0; k--) {
                        down(k);
                    }
                }
            }
            else i++;
        }
    }

    // The manual processes the normal queue first if it is not empty
    Flight* processNext(){
        if(!isEmptyNormal()){
            return dequeueNormal();
        }
        
        return extractMin();
    }

    void displayNormalQueue(){
        cout << "Normal FIFO: ";
        for(QueueNode* p= front; p!=NULL; p=p->next)
            cout << p->flight->callSign << " ";
        cout << endl;
    }

    void displayHeap(){
        cout << "Emergency heap: ";
        for(int i=0; i<heapSize; i++)
            cout << heap[i]->callSign << "(P" << heap[i]->priorityLevel << ") ";
        cout << endl;
    }
};
#endif