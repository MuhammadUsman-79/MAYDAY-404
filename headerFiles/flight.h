#ifndef FLIGHT_H
#define FLIGHT_H
#include<iostream>
#include<string>

using namespace std;

struct Flight{
    string callSign , aircraftType , airline , origin , destination;
    string currentSector , assignedRunway;
    int passengerCount , fuel;
    int arrivalTick , priorityLevel;
    int incidentTick;
    int travelProgress;
    bool cleared , resolved;
    Flight* next;

    Flight(){
        passengerCount = 0;
        fuel = 12;
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
Flight BasicNewFlight(string id , string sector , string runway , int fuelAmount , int priority , int time){
    Flight f;
    f.callSign = id;
    f.aircraftType = "B-747";
    f.airline = "PIA";
    f.origin = "Karachi";
    f.destination = "Iran";
    f.passengerCount = 67;
    f.fuel = fuelAmount;
    f.currentSector = sector;
    f.assignedRunway = runway;
    f.priorityLevel = priority;
    f.arrivalTick = time;

    return f; 
}

#endif
