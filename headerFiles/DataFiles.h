#ifndef DATAFILES_H
#define DATAFILES_H
#include "Registry.h"
#include "Graph.h"
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <iomanip>

struct ScenarioEvent{
    int tick;
    string eventType, target;
};

// Basic delimiter parser text files have fields separated with |
int splitFields(string line, string parts[], int maxParts){
    int count = 0;
    int start = 0;
    for (int i = 0; i <= (int)line.length() && count < maxParts; i++){
        if (i == (int)line.length() || line[i] == '|'){
            parts[count++] = line.substr(start, i - start);
            start = i + 1;
        }
    }
    return count;
}
int toNumber(string s) { return atoi(s.c_str()); }

// Load each flight into a fixed template array the game spawns it at arrivalTick.
int loadFlights(string file, Flight records[], int maxRecords){
    ifstream in(file.c_str());
    if (!in){
        cout << "[ATC Error] Cannot open " << file << endl;
        return 0;
    }
    
    string line, part[12];
    getline(in, line);
    
    int n = 0;
    while (getline(in, line) && n < maxRecords){
        if (splitFields(line, part, 12) < 11)
            continue;

        Flight f;
        f.callSign = part[0];
        f.aircraftType = part[1];
        f.airline = part[2];
        f.origin = part[3];
        f.destination = part[4];
        f.passengerCount = toNumber(part[5]);
        f.fuel = toNumber(part[6]);
        f.currentSector = part[7];
        f.assignedRunway = part[8];
        f.priorityLevel = toNumber(part[9]);
        f.arrivalTick = toNumber(part[10]);
        records[n++] = f;
    }
    return n;
}

int loadCrew(string file, Registration &registry){
    ifstream in(file.c_str());
    if (!in){
        cout << "[ATC Error] Cannot open " << file << endl;
        return 0;
    }

    string line, part[6];
    getline(in, line);
    int n = 0;
    
    while (getline(in, line)){
        if (splitFields(line, part, 6) < 4)
            continue;

        Crew c;
        c.employeeID = part[0];
        c.name = part[1];
        c.role = part[2];
        c.assignedCallSign = part[3];
        if (registry.addCrew(c))
            n++;
    }
    return n;
}
int loadAirspace(string file, SpaceGraph &graph){
    ifstream in(file.c_str());
    if (!in){
        cout << "[ATC Error] Cannot open " << file << endl;
        return 0;
    }
    string line, part[4];
    getline(in, line);
    int n = 0;

    while (getline(in, line)){
        if (splitFields(line, part, 4) < 3)
            continue;
        if (graph.addCorridor(part[0], part[1], toNumber(part[2])))
            n++;
    }
    return n;
}
int loadScenario(string file, ScenarioEvent events[], int capacity){
    ifstream in(file.c_str());
    if (!in){
        cout << "[ATC Error] Cannot open " << file << endl;
        return 0;
    }
    string line, part[4];
    getline(in, line);

    int n = 0;
    while (getline(in, line) && n < capacity){
        if (splitFields(line, part, 4) < 3)
            continue;
        events[n].tick = toNumber(part[0]);
        events[n].eventType = part[1];
        events[n].target = part[2];
        n++;
    }
    return n;
}
void printTextFile(string file){
    ifstream in(file.c_str());
    if (!in){
        cout << "[ATC Error] Cannot open " << file << endl;
        return;
    }
    string line;

    while (getline(in, line))
        cout << line << endl;
}

struct ScoreEntry{
    int no, cred, ticks, crash, hijack, breach;
    string cro, result, date;
};

class LeaderBoard{
    string filename;
    int dateNumber(string s){
        if (s.length() == 10 && s[4] == '-')
            return toNumber(s.substr(0, 4)) * 10000 + toNumber(s.substr(5, 2)) * 100 + toNumber(s.substr(8, 2));
        if (s.length() >= 8 && s[2] == '-')
            return (2000 + toNumber(s.substr(6, 2))) * 10000 + toNumber(s.substr(3, 2)) * 100 + toNumber(s.substr(0, 2));
        return 0;
    }
    bool higher(ScoreEntry a, ScoreEntry b){
        if (a.result != b.result)
            return a.result == "WIN";
        if (a.cred != b.cred)
            return a.cred > b.cred;
        if (a.ticks != b.ticks)
            return a.ticks > b.ticks;
        return dateNumber(a.date) > dateNumber(b.date);
    }
    int load(ScoreEntry entries[], int capacity){
        ifstream in(filename.c_str());
        if (!in)
            return 0;
        string line, part[10];
        getline(in, line);
        int n = 0;
        while (getline(in, line) && n < capacity){
            if (splitFields(line, part, 10) < 9)
                continue;
            entries[n].no = toNumber(part[0]);
            entries[n].cro = part[1];
            entries[n].result = part[2];
            entries[n].cred = toNumber(part[3]);
            entries[n].ticks = toNumber(part[4]);
            entries[n].crash = toNumber(part[5]);
            entries[n].hijack = toNumber(part[6]);
            entries[n].breach = toNumber(part[7]);
            entries[n].date = part[8];
            n++;
        }
        return n;
    }

public:
    LeaderBoard(string file = "data/leaderboard.txt") { filename = file; }
    void saveResult(string cro, string result, int cred, int ticks, int crashes, int hijacks, int breaches){
        ScoreEntry e[250];
        int n = load(e, 250);
        ofstream out(filename.c_str(), ios::app);
        if (!out){
            cout << "[ATC Error] Cannot save leaderboard." << endl;
            return;
        }
        if (n == 0){
            ifstream check(filename.c_str(), ios::binary | ios::ate);
            if (check && check.tellg() == 0)
                out << "#|CRO|Result|Cred|Ticks|Crash|Hijack|Breach|Date" << endl;
        }
        time_t now = time(NULL);
        tm *day = localtime(&now);
        char stamp[16];
        if (day != NULL)
            strftime(stamp, 16, "%Y-%m-%d", day);
        else{
            stamp[0] = '2';
            stamp[1] = '0';
            stamp[2] = '2';
            stamp[3] = '6';
            stamp[4] = 0;
        }
        out << n + 1 << "|" << cro << "|" << result << "|" << cred << "|" << ticks << "|"
            << crashes << "|" << hijacks << "|" << breaches << "|" << stamp << endl;
        cout << "Result saved to " << filename << endl;
    }
    void displayLeaderboard(){
        ScoreEntry e[250];
        int n = load(e, 250);
        if (n == 0){
            cout << "No leaderboard entries." << endl;
            return;
        }
        for (int i = 1; i < n; i++){
            ScoreEntry x = e[i];
            int j = i - 1;
            while (j >= 0 && higher(x, e[j]))
            {
                e[j + 1] = e[j];
                j--;
            }
            e[j + 1] = x;
        }
        cout << "---- LEADERBOARD ----" << endl;
        for (int i = 0; i < n; i++)
            cout << i + 1 << ". " << e[i].cro << "  " << e[i].result << "  Cred " << e[i].cred
                 << "  Ticks " << e[i].ticks << "  " << e[i].date << endl;
    }
};
#endif