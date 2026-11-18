#ifndef DISPATCH_H
#define DISPATCH_H
#include "Flight.h"

struct QueueNode
{
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

public:
    EmergencyLanding() {
        front=rear=NULL;
    }
    
    ~EmergencyLanding(){
        while(front!=NULL){
            QueueNode* p= front;
            front= front->next;
            delete p;
        }
    }

    bool isEmptyNormal() {
        return front==NULL;
    }

    void enqueueNormal(Flight* f){
        if(f==NULL) {
            return;}

        QueueNode* n= new QueueNode(f);
        if(rear==NULL){
            front=n;
        }
        else rear->next=n;
        rear=n;
    }

    Flight* dequeueNormal(){
        if(front==NULL) {
            return NULL;
        }

        QueueNode* p= front;
        Flight* f= p->flight;
        front= front->next;
        if(front==NULL) rear=NULL;
        delete p;
        return f;
    }

    Flight* frontNormal() {
        return front==NULL ? NULL:front->flight;
    }

    void displayNormalQueue(){
        for(QueueNode* p= front; p!=NULL; p=p->next)
            cout<<p->flight->callSign<<" ";
        cout<<endl;
    }
    
};
#endif
