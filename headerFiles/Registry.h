#ifndef REGISTRY_H
#define REGISTRY_H
#include "flight.h"

class Registration
{
    Flight* first;
    Flight* last;
    Crew* crewFirst;
    Crew* crewLast;
    Flight* hashTable[101];
    int hashState[101]; // 0 empty, 1 used, 2 deleted
    int collisions;
    int longestProbe;

    int hashCallsign(string id){
        unsigned int h = 0;
        for(int i=0; i<(int)id.length(); i++)
            h = (h*31 + (unsigned char)id[i]) % 101;
        return (int)h;
    }
    bool putHash(Flight* f){
        int start = hashCallsign(f->callSign);
        int firstDeleted = -1;
        for(int i=0; i<101; i++)
        {
            int at = (start+i)%101;
            if(hashState[at] == 2 && firstDeleted == -1) firstDeleted = at;
            if(hashState[at] == 0)
            {
                if(firstDeleted != -1) at = firstDeleted;
                hashTable[at] = f;
                hashState[at] = 1;
                collisions += i;
                if(i > longestProbe) longestProbe = i;
                return true;
            }
        }
        if(firstDeleted != -1)
        {
            hashState[firstDeleted] = 1;
            hashTable[firstDeleted] = f;
            return true;
        }
        return false;
    }
    void deleteHash(string id)
    {
    int start = hashCallsign(id);
        for(int i=0; i<101; i++)
        {
            int at = (start+i)%101;
            if(hashState[at] == 0) return;
            if(hashState[at] == 1 && hashTable[at]->callSign == id)
            {
                hashTable[at] = NULL;
                hashState[at] = 2;
                return;
            }
        }
    }

public:
    Registration()
    {
        first = last = NULL;
        crewFirst = crewLast = NULL;
    }
    ~Registration()
    {
        while(first != NULL)
        {
            Flight* p = first;
            first = first->next;
            delete p;
        }
        while(crewFirst != NULL)
        {
            Crew* p = crewFirst;
            crewFirst = crewFirst->next;
            delete p;
        }
    }
    Flight* allFlights() { return first; }
    int countFlights()
    {
        int n=0;
        for(Flight* p=first; p!=NULL; p=p->next) n++;
        return n;
    }
    Flight* find(string id)
    {
        for(Flight* p=first;p!=NULL;p=p->next)
            if(p->callSign==id) return p;
        return NULL;
    }
    Flight* addFlight(Flight f)
    {
        if(f.callSign.empty() || find(f.callSign)!=NULL || countFlights()>=100) return NULL;
        Flight* p = new Flight;
        *p = f;
        p->next = NULL;
        if(first==NULL) first=p;
        else last->next=p;
        last=p;
        return p;
    }
    bool removeFlight(string id)
    {
        Flight* p=first;
        Flight* prev=NULL;
        while(p!=NULL && p->callSign!=id) {prev=p;p=p->next;}
        if(p==NULL) return false;
        if(prev==NULL) first=p->next;
        else prev->next=p->next;
        if(last==p) last=prev;
        delete p;
        return true;
    }
    bool addCrew(Crew c)
    {
        for(Crew* p=crewFirst;p!=NULL;p=p->next)
            if(p->employeeID==c.employeeID) return false;
        Crew* n=new Crew; *n=c;n->next=NULL;
        if(crewFirst==NULL) crewFirst=n;
        else crewLast->next=n;
        crewLast=n;
        return true;
    }
    void displayAllCrew()
    {
        if(crewFirst==NULL) cout<<"No crew available."<<endl;
        for(Crew* p=crewFirst;p!=NULL;p=p->next)
            cout<<p->employeeID<<" | "<<p->name<<" | "<<p->role
                <<" | "<<p->assignedCallSign<<endl;
    }
    void displayAllFlights()
    {
        if(first==NULL) {cout<<"No active flights."<<endl;return;}
        for(Flight* p=first;p!=NULL;p=p->next)
            cout<<p->callSign<<"  fuel: "<<p->fuel<<endl;
    }
    void displayStats()
    {
        cout<<"Flights registered: "<<countFlights()<<endl;
    }
};
#endif
