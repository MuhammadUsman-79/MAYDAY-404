#ifndef FLIGHT_H
#define FLIGHT_H
#include<iostream>
#include<string>

using namespace std;

struct Flight{
    string callSign , airCraftType , airLine , origin , destination;
    string currentSector , assignedRunway;
    int passengersCount , remaningFuel;
    int arrivalTick , priorityLevel;
    int incidentTick;
    int travelProgress;
    bool cleared , resolved;
    Flight* next;

    Flight(){
        passengersCount = 0;
        remaningFuel = 12;
        arrivalTick = 0;
        priorityLevel = 9;
        cleared = false;
        resolved = false;
        incidentTick = -1;
        travelProgress = 0;
        next = nullptr;
    }

};

struct Crew{
    string employeeID , name , role , assignedCallSign;
    Crew* next;

    Crew(){
        next = nullptr;
    }

};

//setting a flight to manual values
Flight BasicNewFlight(string id , string sector , string runway , int fuel , int priority , int tick){
    Flight f;
    f.callSign = id;
    f.airCraftType = "B-747";
    f.airLine = "PIA";
    f.origin = "Karachi";
    f.destination = "Iran";
    f.passengersCount = 67;
    f.remaningFuel = fuel;
    f.currentSector = sector;
    f.assignedRunway = runway;
    f.priorityLevel = priority;
    f.arrivalTick = tick;

    return f; 
}



#endif
