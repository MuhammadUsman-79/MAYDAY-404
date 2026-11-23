#ifndef GRAPH_H
#define GRAPH_H

#include<iostream>
#include<string>
using namespace std;

int const MAX_SECTORS = 30;
int const BIG_DIST = 100000000;

struct RoadNode{
    int to , cost;
    RoadNode* next;

    RoadNode(int t , int c){
        to = t;
        cost = c;
        next = nullptr;
    }
};

class SpaceGraph{
    string names[MAX_SECTORS];
    RoadNode* edges[MAX_SECTORS];
    int** matrix;
    int total;

    void setRoad(int a, int b , int cost){
        RoadNode* p = edges[a];

        while(p!=nullptr){
            if(p->to == b){
                p->cost = cost;
                return;
            }
            p = p->next;
        }
        RoadNode* n = new RoadNode(b , cost);
        n->next = edges[a];
        edges[a] = n;
    }

public:
    SpaceGraph(){
        total = 0;
        matrix = new int*[MAX_SECTORS];
        for(int i = 0; i < MAX_SECTORS; i++){
            edges[i] = nullptr;
            matrix[i] = new int[MAX_SECTORS];
                
                for(int j = 0; j < MAX_SECTORS; j++){
                    matrix[i][j] = 0;
                }
        }
    }

    ~SpaceGraph(){
        for(int i = 0; i < MAX_SECTORS; i++){
            while(edges[i] != nullptr){
                RoadNode* p = edges[i];
                edges[i] = p->next;
                delete p;
            }
            delete[] matrix;
        }
    }

    int getSectorCount(){
        return total;
    }
    int sectorId(string name){
        for(int i = 0; i < total; i++){
            if(names[i] == name)
                return i;
        }   
        return -1;
    }
    string sectorName(int i){
        if(i < 0 || i>= total)
            return "Unknown";

        return names[i];
    }
    int addSector(string name){
        int id = sectorId(name);
        if(id != -1)
            return id;

        if(name.empty() || total>= MAX_SECTORS)
            return -1;

        names[total] = name;
        total++;
        return total - 1;
    }
    bool addCorridor(string from , string to , int cost){
        if(cost <= 0)
            return false;

        int a = addSector(from);
        int b = addSector(to);
        if(a < 0 || b < 0 || a==b)
            return false;

        matrix[a][b] = matrix[b][a] = cost;

        setRoad(a , b, cost);
        setRoad(b , a , cost);

        return true;
    }
    bool disableCorridor(string from ,string to){
        int a = sectorId(from);
        int b = sectorId(to);

        if(a < 0 || b < 0 || matrix[a][b] == 0)
            return false;

        matrix[a][b] = matrix[b][a] = 0;
        setRoad(a , b , 0);
        setRoad(b , a , 0);

        return true;
    }
    int getWeight(int a , int b){
        if(a < 0 || b < 0 || a >= total || b >= total)
            return 0;

        return matrix[a][b];
    }
    int getWeight(string from , string to){
        return getWeight(sectorId(from) , sectorId(to));
    }
   RoadNode* neighbors(int id) {
    if (id < 0 || id >= total) {
        return nullptr;
    } 
    else {
        return edges[id];
    }
}
    void displayAdjacencyList(){
        for(int i = 0; i < total; i++){
            cout << i << ". " << names[i] << " : ";
            for(RoadNode* p=edges[i]; p!=nullptr; p=p->next){
                if(p->cost > 0)
                    cout << sectorName(p->to) << "(" << p->cost << ")    " << endl; 
            }
        }
    }
    void displayAdjacencyMatrix(){
        cout << "Sector indices are the same as in adjacency list." << endl << "    ";

        for(int i = 0; i < total; i++)
            cout << i << "\t" << endl;

        for(int i = 0; i < total; i++){
            cout << i << " : ";
            for(int j = 0; j < total; j++){
                cout << matrix[i][j] << "\t";
                cout << endl;
            }
        }
    }
};












#endif