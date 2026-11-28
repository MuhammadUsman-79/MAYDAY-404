#ifndef GAME_H
#define GAME_H
#include "TreeIndex.h"
#include "Dispatch.h"
#include "History.h"
#include "DataFiles.h"
#include <cstdlib>

//one missing flight waiting to be found (node of a singly linked list)
// window = ticks before it breaches airspace, minCred = credibility needed to search
struct MissingNode{
    string id, sector;
    int since, window, minCred;
    MissingNode* next;

    MissingNode(string a, string b, int t, int w, int c){
        id= a;
        sector= b;
        since= t;
        window = w;
        minCred= c;
        next= NULL;
    }
};

// The game engine: it owns every data structure and runs the turn logic
class GameEngine{

    // ---- Data structures from all three members ----
    Registration registry; // linked list of flights and crew
    FlightBST bst; //BST index of flights
    FlightAVL avl; // AVL index of flights
    EmergencyLanding dispatch; // my queue + heap
    CommandHistory history; // my log + undo stack
    SpaceGraph graph; //airspace map
    RadarSweep radar; // BFS / DFS
    RouteFinder routes; // shortest route
    RadarMST mst; // minimum spanning tree
    LeaderBoard board; //saves results in a file

    // Data loaded from the text files
    Flight templates[80];
    ScenarioEvent events[80];
    int templateCount, eventCount;

    MissingNode* missing; // list of missing flights

    // ---- Game state ----
    string player, failCause, cyberSector;
    int tick, actions, cred, maxTicks, landed, resolved, crashes, hijacks, breaches;
    int cyberSince;
    bool cyberActive, cyberCorridorDisabled, towerDegraded, finished;

    int getNumber(){
        int n;
        if(!(cin>>n)){
            cin.clear();
            cin.ignore(10000, '\n');
            return -999; //impossible value, so invalid
        }
        return n;
    }

    //turn short runway code like 12 into runway name used in the graph
    string runwayCode(string code){
        if(code=="12" || code=="Runway 12") {
            return "Runway 12 (West)";
        }
        if(code=="09" || code=="9" || code=="Runway 09") {
            return "Runway 09 (North)";
        }
        if(code=="04" || code=="4" || code=="Runway 04") {
            return "Runway 04 (East)";
        }
        if(code=="17" || code=="Runway 17") {
            return "Runway 17 (South)";
        }

        return code;
    }

    // Only runways 12 and 09 are meant for emergency flights
    bool isEmergencyRunway(string name){
        return name=="Runway 12 (West)" || name=="Runway 09 (North)";
    }

    // Change credibility, keep it between 0 and 200, and end the game at 0
    void adjustCred(int value){
        cred+= value;
        if(cred>200) {
            cred= 200;
        }
        if(cred<=0){
            cred= 0;
            finished= true;
            failCause= "Credibility reached zero";
        }
    }

    // Wrong input costs credibility...double when the tower is degraded
    void invalid(string why){
        cout << "[ATC Error] " << why << ". Credibility -5" << endl;
        adjustCred(towerDegraded ? -10 : -5);
    }

    //end the game with a reason
    void fail(string cause){
        finished= true;
        failCause= cause;
        cout << "*** MISSION FAILED: " << cause << " ***" << endl;
    }

    //put flight in the correct structure again
    //remove it everywhere, then priority 9 -> normal queue, otherwise -> emergency heap
    void refreshDispatch(Flight* f){
        if(f==NULL) {
            return;
        }
        dispatch.removeFlight(f);
        if(f->cleared) {
            return; //cleared flights are not waiting any more
        }
        if(f->priorityLevel==9) {
            dispatch.enqueueNormal(f);
        }
        else {
            dispatch.insertEmergency(f);
        }
    }

    //add a new flight to the registry, both trees and the dispatch structures
    Flight* addToGame(Flight f){
        if(graph.sectorId(f.currentSector)<0 || graph.sectorId(f.assignedRunway)<0){
            cout << "[ATC Error] Flight has unknown sector or runway: " << f.callSign << endl;
            return NULL;
        }

        Flight* p= registry.addFlight(f);
        if(p==NULL) {
            return NULL;
        }

        bst.insert(p);
        avl.insert(p);
        refreshDispatch(p);
        return p;
    }

    //remove flight from every structure....queue/heap,BST, AVL,registry
    void removeFromGame(Flight* f){
        if(f==NULL) {
            return;
        }

        string id= f->callSign; //copy the ID first, because the flight is deleted below
        dispatch.removeFlight(f);
        bst.erase(id);
        avl.erase(id);
        registry.removeFlight(id);
    }

    //how many ticks the player has to solve an incident of this priority
    int incidentWindow(int priority){
        if(priority==1) {
            return 2;
        }
        if(priority==2) {
            return 3;
        }
        if(priority==5) {
            return 5;
        }
        if(priority==6) {
            return 4;
        }
        if(priority==7) {
            return 4;
        }
        if(priority==8) {
            return 5;
        }

        return 100;
    }

    //find a preloaded flight template by callSign
    Flight* getTemplate(string id){
        for(int i=0; i<templateCount; i++) {
            if(templates[i].callSign==id) {
                return &templates[i];
            }
        }

        return NULL;
    }

    // Pick a random active flight..used by events like fuel or medical
    Flight* randomActive(){
        int n= registry.countFlights();
        if(n==0) {
            return NULL;
        }

        int index= rand()%n;
        Flight* p= registry.allFlights();
        while(index>0 && p!=NULL) {
            p= p->next;
            index--;
        }
        return p;
    }

    // Create a missing flight record..ignoring if it already exists
    void makeMissing(string id){
        for(MissingNode* p= missing; p!=NULL; p=p->next) {
            if(p->id==id) {
                return;
            }
        }

        // Default values, then special values for some known flights
        string sector= "Syntax Sector (West)";
        int window= 9, minCred= 54;
        if(id=="MH370") {
            sector= "Restricted Zone";
            window= 6;
            minCred= 63;
        }
        if(id=="FT739") {
            sector= "Cache Fly (East)";
            window= 13;
            minCred= 40;
        }
        if(id=="GA542") {
            sector= "Air Segment (North)";
            window= 15;
            minCred= 38;
        }

        //insert at the head of the missing list
        MissingNode* n= new MissingNode(id, sector, tick, window, minCred);
        n->next= missing;
        missing= n;
        cout << "*** MISSING FLIGHT " << id << " over " << sector
        <<". Window: " <<window << " ticks ***" << endl;
    }

    //runs everything scheduled for the current tick....new flights and events
    void applyScenario(){

        //preloaded flight templates spawn only when their arrival tick arrives
        for(int i=0; i<templateCount; i++){
            if(templates[i].arrivalTick==tick && tick>0 && templates[i].priorityLevel!=1){
                if(addToGame(templates[i])) {
                    cout << "New flight: " << templates[i].callSign << endl;
                }
            }
        }

        //scheduled events from scenario.txt
        for(int i=0; i<eventCount; i++){
            if(events[i].tick!=tick) {
                continue;
            }
            string kind= events[i].eventType, id= events[i].target;

            if(kind== "Missing"){
                makeMissing(id);
                continue;
            }

            if(kind == "Cyber"){
                cyberActive= true;
                cyberCorridorDisabled= false;
                cyberSector= id;
                cyberSince= tick;
                cout << "*** CYBER ALERT at " << id << " - use option 11 ***" << endl;
                continue;
            }

            if(kind == "Spawn"){
                Flight* t= getTemplate(id);
                Flight f;
                if(t!=NULL) {
                    f= *t;
                }
                else {
                    f= BasicNewFlight(id, "Holding Pattern Delta", "Runway 12 (West)", 12, 9, tick);
                }

                f.arrivalTick= tick;
                if(addToGame(f)) {
                    cout << "Spawned flight " << id << endl;
                }
                continue;
            }

            //all other events change the priority of one flight
            Flight* f= registry.find(id);

            //war aircraft are future templates
            //don't revive a flight that already landed
            if(f == NULL && kind == "War"){
                Flight* t= getTemplate(id);
                if(t!=NULL) {
                    f= addToGame(*t);
                }
            }

            if(f==NULL && (kind=="Hijack" || kind=="Fuel" || kind =="Medical" || kind== "Weather")) {
                f= randomActive();
            }
            if(f==NULL){
                cout << "[ATC] Event " << kind << " skipped (no flight available)" << endl;
                continue;
            }

            // Each event type maps to a priority level
            int priority= f->priorityLevel;
            if(kind=="War") {
                priority= 1;
            }
            else if(kind=="Hijack") {
                priority= 2;
            }
            else if(kind=="Fuel") {
                priority= 5;
            }
            else if(kind=="Medical") {
                priority= 6;
            }
            else if(kind=="Weather") {
                priority= 7;
            }
            else if(kind=="Mechanical") {
                priority= 8;
            }

            f->priorityLevel= priority;
            f->incidentTick= tick;
            f->resolved= false;
            f->cleared= false;
            refreshDispatch(f); // priority changed, so move it to the right structure
            cout << "*** " << kind << " ALERT: " << f->callSign 
            << " (priority " << priority << ") ***" << endl;
        }
    }

    //a flight reached its runway
    void landFlight(Flight* f){
        if(f->priorityLevel==1 && !f->resolved){
            f->resolved= true;
            resolved++;
            adjustCred(30);
            cout << "Hostile flight cleared safely. +30 credibility" << endl;
        }
        cout << f->callSign << " LANDED safely at " << f->assignedRunway << ". +2 credibility" << endl;
        landed++;
        adjustCred(2);
        history.logCommand("LANDED", f->callSign);
        removeFromGame(f);
    }

    // Every tick: burn fuel, move cleared flights along their route, land or crash them
    void advanceFlights(){

        Flight* f= registry.allFlights();

        while(f!=NULL && !finished){
            Flight* next= f->next; //save it, because f may be deleted below
            f->fuel--;

            if(f->cleared){
                Route r = routes.findFastestRoute(f->currentSector, f->assignedRunway);
                if(r.found){
                    if(r.size>1){
                        int w = graph.getWeight(r.nodes[0], r.nodes[1]);
                        f->travelProgress++;
                        //move to the next sector when the corridor travel time is over
                        if(f->travelProgress>=w){
                            f->currentSector = graph.sectorName(r.nodes[1]);
                            f->travelProgress = 0;
                            cout << f->callSign << " moved to " << f->currentSector << endl;
                        }
                    }
                    if(f->currentSector == f->assignedRunway){
                        landFlight(f);
                        f= next;
                        continue;
                    }
                }
                else {
                    cout << "[ATC] Route blocked for " << f->callSign << endl;
                }
            }

            //out of fuel means a crash and the mission fails
            if(f->fuel <= 0){
                crashes++;
                adjustCred(-50);
                failCause= f->callSign + " ran out of fuel";
                history.logCommand("CRASH", f->callSign);
                removeFromGame(f);
                fail(failCause);
                break;
            }
            f= next;
        }
    }

    //checks every unresolved problem and applies the penalty when time runs out
    void escalation(){

        //1. Flight incidents
        for(Flight* f= registry.allFlights(); f!=NULL && !finished; f=f->next){
            if(f->incidentTick<0 || f->resolved) {
                continue;
            }
            int age= tick - f->incidentTick;
            if(age<incidentWindow(f->priorityLevel)) {
                continue;
            }

            if(f->priorityLevel==1) {
                fail("War aircraft was not intercepted");
            }
            else if(f->priorityLevel==2){
                hijacks++;
                fail("Hijack was not intercepted");
            }
            else if(f->priorityLevel==5 || f->priorityLevel==7 || f->priorityLevel==8){
                crashes++;
                fail("Emergency escalated for " + f->callSign);
            }
            else if(f->priorityLevel==6){
                cout << "Medical casualty on " << f->callSign << ". Credibility -20" << endl;
                adjustCred(-20);
                f->resolved= true;
            }
        }

        //2. Missing flights that were not found in time
        for(MissingNode* p= missing; p!=NULL && !finished; p=p->next){
            if(tick - p->since >= p->window){
                breaches++;
                fail("Missing flight " + p->id + " breached restricted airspace");
            }
        }

        //3 Cyber attack gets worse with time
        if(cyberActive && !finished){
            int age= tick - cyberSince;

            //after 2 ticks, one corridor is disabled
            if(age>=2 && !cyberCorridorDisabled){
                int at= graph.sectorId(cyberSector);
                RoadNode* p= graph.neighbors(at);
                while(p!=NULL && p->cost==0) {
                    p= p->next;
                }
                if(p!=NULL){
                    string to= graph.sectorName(p->to);
                    graph.disableCorridor(cyberSector, to);
                    cout << "Cyber disabled corridor: " << cyberSector << " -> " << to << endl;
                }
                cyberCorridorDisabled= true;
            }

            //after 3 ticks, the tower is degraded
            if(age>=3 && !towerDegraded){
                towerDegraded= true;
                cout << "Tower degraded! Routing actions now cost 2 actions." << endl;
            }

            //after 4 ticks, the game is lost
            if(age>=4) {
                breaches++;
                fail("Cyber breach disabled the Control Tower");
            }
        }
    }

    //move the game one tick forward
    void nextTick(){
        tick++;
        if(tick>maxTicks) {
            tick= maxTicks;
        }
        cout << endl << "-- Advancing to tick " << tick << " --" << endl;
        advanceFlights();
        if(!finished) {
            escalation();
        }
        if(!finished) {
            applyScenario();
        }
        actions= 2; //fresh actions for the new tick

        if(tick==maxTicks && !finished){
            finished= true;
            cout << "30 ticks survived. Mission complete!" << endl;
        }
    }

    //print the current status screen
    void showState(){
        cout << endl << "========== MAYDAY-404  TICK " << tick << "/" << maxTicks << " ==========" << endl;
        cout << "CRO: " << player << " | Credibility: " << cred << " | Actions left: " << actions << endl;
        cout << "Active flights: " << registry.countFlights() << " | Landed: " << landed
        << " | Emergencies resolved: " << resolved << endl;

        if(cyberActive) {
            cout << "CYBER ACTIVE: " << cyberSector << endl;
        }

        if(missing!=NULL) {
            cout << "MISSING FLIGHTS pending. Use option 19." << endl;
        }

        for(Flight* f= registry.allFlights(); f!=NULL; f=f->next){
            cout << "  " << f->callSign << " " << f->currentSector << " | fuel " << f->fuel
            << " | priority " << f->priorityLevel << (f->cleared ? " | CLEARED" : "") << endl;
        }

        cout << "==========================================" << endl;
    }

    void showMenu(){
        //read the text file each time
        printTextFile("data/gamemenu.txt");
    }

    void showSectors(){
        for(int i=0; i < graph.getSectorCount(); i++) {
            cout<< i <<" = " << graph.sectorName(i) <<endl;
        }
    }

    //ask the player to pick a sector by number
    string askSector(){
        showSectors();
        cout << "Sector number: ";
        return graph.sectorName(getNumber());
    }

    //ask the player for a runway code
    string askRunway(){
        cout << "Runway number (12, 09, 04, 17): ";
        string s;
        cin >> s;
        return runwayCode(s);
    }

    //clear (or reroute) a flight to a runway
    // returns true if it worked
    bool clearForRunway(Flight* f, string runway, bool isReroute){
        if(f==NULL) {
            invalid("Flight not found");
            return false;
        }
        if(graph.sectorId(runway)<0) {
            invalid("Invalid runway");
            return false;
        }

        Route r = routes.findFastestRoute(f->currentSector, runway);
        if(!r.found) {
            invalid("Runway unreachable");
            return false;
        }

        history.pushSnapshot(f, cyberActive, cred); //save old state BEFORE changing (for undo)
        f->assignedRunway= runway;
        f->cleared= true;
        f->travelProgress= 0;

        if(f->priorityLevel==1 && !f->resolved){
            f->resolved= true;
            resolved++;
            adjustCred(30);
            cout << "War aircraft controlled, +30 credibility." << endl;
        }

        if(f->priorityLevel>2 && f->priorityLevel<9 && !f->resolved){
            //a hijack must be intercepted separately
            if(f->priorityLevel==7 && isReroute) {
                f->resolved= true;
            }
        }
        if(f->priorityLevel<9 && !isEmergencyRunway(runway)){
            adjustCred(-10);
            cout << "Non-emergency runway penalty: -10 credibility." << endl;
        }

        refreshDispatch(f);
        history.logCommand(isReroute ? "REROUTE" : "CLEAR", f->callSign);
        cout << "Roger " << f->callSign << ", cleared for " << runway << endl;
        routes.printRoute(r);
        return true;
    }

    //show why AVL is better than a plain BST
    void showIndexHeights(){
        cout << "Live BST height: " << bst.height() << " | AVL height: " << avl.height() << endl;
        cout << "BST inorder: ";
        bst.inorderTraversal();

        //separate 15 sorted insertions show balancing even with few active flights
        Flight demo[15];
        FlightBST plain;
        FlightAVL balanced;
        for(int i=0; i<15; i++){
            string code= "DEMO-";
            code+= char('A' +i);
            demo[i].callSign = code;
            plain.insert(&demo[i]);
            balanced.insert(&demo[i]);
        }
        cout << "15 alphabetically inserted flights: BST height " << plain.height()
        <<" | AVL height "<< balanced.height() <<endl;
    }

    //does the action for menu options 1 to 21
    void doAction(int choice){
        string id;

        // 1, 4, 5: register a new flight (4 = normal, 5 = emergency)
        if(choice==1 || choice==4 || choice==5){
            cout << "Callsign (no spaces): ";
            cin >> id;
            cout << "Choose current sector:" << endl;
            string sector= askSector();
            string runway= askRunway();

            if(graph.sectorId(sector)<0){
                invalid("Invalid sector");
                return;
            }

            cout << "Fuel in ticks (1-99): ";
            int fuel= getNumber();
            if(fuel<1 || fuel>99){
                invalid("Invalid fuel");
                return;
            }

            int p= 9;
            if(choice==5){
                cout << "Priority 1 War, 2 Hijack, 5 Fuel, 6 Medical, 7 Weather, 8 Mechanical: ";
                p= getNumber();
                if(p!=1 && p!=2 && p!=5 && p!=6 && p!=7 && p!=8){
                    invalid("Invalid priority");
                    return;
                }
            }

            Flight f= BasicNewFlight(id, sector, runway, fuel, p, tick);
            if(p<9) {
                f.incidentTick= tick;
            }

            if(!addToGame(f)) {
                invalid("Duplicate callsign or invalid flight");
            }
            else {
                cout << id << " registered." << endl;
            }
        }
        // 2. add a crew member
        else if(choice==2){
            Crew c;
            cout << "Employee ID: ";
            cin >> c.employeeID;
            cout << "Name (use underscore instead of spaces): ";
            cin >> c.name;
            cout << "Role: ";
            cin >> c.role;
            cout << "Assigned callsign: ";
            cin >> c.assignedCallSign;

            if(!registry.addCrew(c)) {
                invalid("Duplicate crew ID");
            }
            else {
                cout << "Crew member added." << endl;
            }
        }
        //3:search one flight
        else if(choice==3){
            cout << "Callsign: ";
            cin >> id;
            Flight* f= registry.find(id);

            if(f==NULL) {
                invalid("Flight not found");
            }
            else {
                cout << f->callSign << " | " << f->currentSector << " | fuel " << f->fuel << endl;
            }
        }
        // 6: process the next landing (normal queue first, then the heap)
        else if(choice==6){
            Flight* f= dispatch.processNext();

            if(f==NULL){
                invalid("No arrivals in queues");
                return;
            }
            cout << "Next flight: " << f->callSign << " (priority " << f->priorityLevel << ")"<<endl;
            clearForRunway(f, f->assignedRunway, false);
        }
        // 7, 8: clear (7) or reroute (8) a flight to a chosen runway
        else if(choice == 7 || choice ==8){
            cout << "Callsign: ";
            cin >> id;
            Flight* f= registry.find(id);
            string runway= askRunway();
            clearForRunway(f, runway, choice==8);
        }
        // 9) intercept a war or hijack flight
        else if(choice == 9){
            cout << "Hostile/hijacked callsign: ";
            cin >> id;
            Flight* f= registry.find(id);

            if(f==NULL || (f->priorityLevel!=1 && f->priorityLevel!=2) || f->resolved){
                invalid("No active hijack/war alert for that flight");
                return;
            }

            history.pushSnapshot(f, cyberActive, cred);
            f->resolved= true;
            resolved++;
            int bonus= f->priorityLevel==1 ? 30 : 20;
            adjustCred(bonus);
            refreshDispatch(f);
            history.logCommand("INTERCEPT", id);
            cout << "Intercept completed. Credibility +" << bonus << endl;
        }
        // 10th: change the priority of a flight
        else if(choice==10){
            cout << "Callsign: ";
            cin >> id;
            Flight* f= registry.find(id);

            if(f==NULL){
                invalid("Flight not found");
                return;
            }

            cout << "New priority 1-9: ";
            int p= getNumber();
            if(p<1 || p>9){
                invalid("Invalid priority");
                return;
            }

            history.pushSnapshot(f, cyberActive, cred);
            f->priorityLevel= p;
            f->incidentTick= tick;
            f->resolved= false;
            refreshDispatch(f);
            history.logCommand("ESCALATE", id);
            cout << id << " priority changed to " << p << endl;
        }
        // 11: counter a cyber attack
        else if(choice==11){
            if(!cyberActive){
                invalid("No active cyber alert");
                return;
            }

            history.pushSnapshot(NULL, cyberActive, cred); // NULL because no single flight changes
            cyberActive= false;
            towerDegraded= false;
            resolved++;
            adjustCred(15);
            history.logCommand("COUNTER CYBER", cyberSector);
            cout << "Cyber attack countered. Credibility +15" << endl;
        }
        // 12: undo the last action
        else if(choice==12){
            bool ok= history.undo(registry, cyberActive, cred);
            if(!ok) {
                invalid("No snapshot available");
            }
            else {
                //rebuild the pointers in queue/heap after undoing a priority change
                for(Flight* p= registry.allFlights(); p!=NULL; p=p->next) {
                    refreshDispatch(p);
                }
                cout << "Previous state restored." << endl;
            }
        }
        //17. BFS radar scan
        else if(choice==17){
            cout << "BFS starting sector:" << endl;
            radar.bfsFrom(askSector());
            radar.withinTwoHops();
        }
        // 18: DFS radar scan
        else if(choice==18){
            cout << "DFS starting sector:" << endl;
            radar.dfsFrom(askSector());
        }
        // 19: search for a missing flight
        else if(choice==19){
            cout << "Missing flight callsign: ";
            cin >> id;

            MissingNode* p= missing;
            MissingNode* prev= NULL;
            while(p!=NULL && p->id!=id){
                prev= p;
                p= p->next;
            }

            if(p==NULL){
                invalid("No missing record for this flight");
                return;
            }
            if(cred<p->minCred){
                invalid("Not enough credibility to search");
                return;
            }
            if(!radar.searchMissingFlight(p->sector)){
                cout << "Last known sector unreachable. Search failed." << endl;
                return;
            }
            if(p->sector=="Restricted Zone"){
                hijacks++;
                fail("Missing flight located inside Restricted Zone");
                return;
            }

            cout << "Found " << id << " via radar BFS. Credibility +20" << endl;
            adjustCred(20);
            resolved++;
            //remove the found record from the missing list
            if(prev==NULL) {
                missing= p->next;
            }
            else {
                prev->next= p->next;
            }
            delete p;
        }
        // 20: shortest route between two places
        else if(choice==20){
            cout << "Route source:" << endl;
            string from= askSector();
            string to= askRunway();
            Route r= routes.findFastestRoute(from, to);

            if(!r.found) {
                invalid("No route");
            }
            else {
                routes.printRoute(r);
            }
        }
        // 21: minimum spanning tree
        else if(choice==21){
            mst.computeMST();
            mst.displayMST();
            cout << "Each MST edge is a bridge: " << (mst.verifyConnectivity() ? "YES" : "NO") <<endl;
        }
    }

    //print the final result and save it to the leaderboard file
    void finishGame(){

        bool win= (failCause.empty() && tick>=maxTicks);
        cout << endl << "+============= MISSION " << (win ? "WIN" : "LOSE") << " =============+" << endl;
        cout << "CRO: " << player << " | Tick: " << tick << " | Credibility: " << cred << endl;
        cout << "Landed: " << landed << " | Resolved: " << resolved << " | Crashes: " << crashes << endl;

        if(!win) {
            cout << "Cause: " << (failCause.empty() ? "Player ended game" : failCause) << endl;
        }
        cout << "+======================================+" << endl;

        board.saveResult(player, win ? "WIN" : "LOSE", cred, tick, crashes, hijacks, breaches);
    }

public:
    //Starting values of the game
    GameEngine() : radar(graph), routes(graph), mst(graph), board("data/leaderboard.txt"){
        templateCount= eventCount= 0;
        missing= NULL;
        tick= 0;
        actions= 2;
        cred= 100;
        maxTicks= 30;
        landed= resolved= crashes= hijacks= breaches= 0;
        cyberSince= -1;
        cyberActive= cyberCorridorDisabled= towerDegraded= finished= false;
    }

    // Free the missing flight list ..created with new
    ~GameEngine(){
        while(missing!=NULL){
            MissingNode* p= missing;
            missing= missing->next;
            delete p;
        }
    }

    // Load all text files: airspace, flights, scenario, crew and rules
    void loadGameFiles(){
        loadAirspace("data/airspace.txt", graph);
        templateCount= loadFlights("data/flights.txt", templates, 80);

        for(int i=0; i<templateCount; i++) {
            templates[i].assignedRunway= runwayCode(templates[i].assignedRunway);
        }

        eventCount= loadScenario("data/scenario.txt", events, 80);

        //flights with arrival tick 0 start in the game immediately
        int loaded= 0;
        for(int i=0; i<templateCount; i++){
            if(templates[i].arrivalTick==0 && addToGame(templates[i])!=NULL) {
                loaded++;
            }
        }
        loadCrew("data/crew.txt", registry);

        //read Max Ticks and Starting Credibility from rules.txt
        ifstream in("data/rules.txt");
        string line;
        while(getline(in, line)){
            if(line.find("Max Ticks")!=string::npos){
                int at= (int)line.find('=');
                if(at>=0){
                    int n= toNumber(line.substr(at+1));
                    if(n>0 && n<=100) {
                        maxTicks= n;
                    }
                }
            }
            if(line.find("Starting Credibility")!=string::npos){
                int at= (int)line.find('=');
                if(at>=0){
                    int n= toNumber(line.substr(at+1));
                    if(n>0 && n<=200) {
                        cred= n;
                    }
                }
            }
        }
        cout << "Loaded " << loaded << " flights, " << graph.getSectorCount()
             << " sectors and " << eventCount << " scheduled events." << endl;
        registry.displayStats();
    }

    //the main game loop
    void run(){
        cout << "Enter CRO name (no spaces, use _ or .): ";
        cin >> player;

        if(player.empty()){
            cout << "Invalid name." << endl;
            return;
        }

        loadGameFiles();
        if(graph.getSectorCount()==0){
            cout << "[ATC Error] Missing airspace data. Cannot start." << endl;
            return;
        }
        cout << "Rules: up to 2 actions per tick. View choices 13-16 and 22 are free." << endl;

        while(!finished){
            showState();
            showMenu();
            cout << "Choose option (1-24): ";
            int choice= getNumber();

            //23 = end the tick
            if(choice==23) {
                nextTick();
                continue;
            }
            if(choice==24){ //24 =quit
                cout << "You ended the mission." << endl;
                failCause= "Quit to main menu";
                finished= true;
                break;
            }

            //free view options...they cost no action
            if(choice==13){
                dispatch.displayNormalQueue();
                dispatch.displayHeap();
                continue;
            }
            if(choice==14){
                registry.displayAllCrew();
                continue;
            }
            if(choice==15){
                registry.displayAllFlights();
                continue;
            }
            if(choice==16){
                graph.displayAdjacencyList();
                graph.displayAdjacencyMatrix();
                continue;
            }
            if(choice==22){
                showIndexHeights();
                continue;
            }
            if(choice<1 || choice>24){
                invalid("Invalid menu choice");
                if(finished) {
                    break;
                }
                continue;
            }

            //other options are actions
            //When tower is degraded, routing costs 2
            int cost= (towerDegraded && (choice==7 || choice==8 || choice==20)) ? 2 : 1;
            if(actions<cost){
                invalid("Not enough actions left");
                continue;
            }
            doAction(choice);
            if(finished) {
                break;
            }
            actions-= cost;
            if(actions<=0) {
                nextTick(); //out of actions, so the tick advances automatically
            }
        }
        
        finishGame();
    }
};
#endif