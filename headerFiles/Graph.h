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

class RadarSweep{

    SpaceGraph& graph;
    bool visited[MAX_SECTORS];
    
    void dfs(int i){
        visited[i] = true;
        cout << graph.sectorName(i) << " ";
        for(RoadNode* p=graph.neighbors(i); p!=nullptr; p=p->next){
            if(p->cost > 0 && !visited[p->to])
                dfs(p->to);
        }
    }

public:
    RadarSweep(SpaceGraph& g): graph(g){}

    //using simple array as queue
    int bfsFrom(string source){
        int start = graph.sectorId(source);
        if(start < 0){
            cout << "[ATC Error !!] Unknown Sector !!\n";
                return 0;
        }
        for(int i = 0; i < MAX_SECTORS; i++){
            visited[i] = false;
        }
        
        int q[MAX_SECTORS];
        int dist[MAX_SECTORS];
        int front = 0, rear = 0;
        
        for(int i = 0; i < MAX_SECTORS; i++){
            dist[i] = -1;
        }

        q[rear++] = start;
        visited[start] = true;
        dist[start] = 0;

        cout << "BFS From " << source << ":" << endl;

        while(front < rear){
            int here = q[front++];
            cout << "Hop " << dist[here] << ": " << graph.sectorName(here) << endl;
            
            for(RoadNode* p=graph.neighbors(here); p!=nullptr; p=p->next){
                if(p->cost > 0 && !visited[p->to]){
                    visited[p->to] = true;
                    dist[p->to] = dist[here] + 1;
                    q[rear++] = p->to;
                }
            }
        
        }
        cout << "Visited: " << rear << endl;
        return rear;
    }

    void dfsFrom(string source){
        int start = graph.sectorId(source);
        if(start < 0){
            cout << "[ATC Error !!] Unknown Sector !!" << endl;
            return;
        }
        for(int i = 0; i < MAX_SECTORS; i++){
            visited[i] = false;
        }
        cout << "DFS: ";
        dfs(start);
        cout << "\n";
    }

    bool isReachable(string from , string to){
        int a = graph.sectorId(from);
        int b = graph.sectorId(to);

        if(a < 0 || b < 0)
            return false;
        
        bool seen[MAX_SECTORS] = {false};
        int q[MAX_SECTORS];
        int front = 0 , rear = 0;
        q[rear++] = a;
        seen[a] = true;

        while(front < rear){
            int here = q[front++];
            if(here == b)
                return true;

            for(RoadNode* p=graph.neighbors(here); p!=nullptr; p=p->next){
                if(p->cost > 0 && !seen[p->to]){
                    if(p->cost > 0 && !seen[p->to]){
                        seen[p->to] = true;
                        q[rear++] = p->to;
                    }
                }
            }
        }
        return false;    
    }

    bool searchMissingFlight(string lastKnown){
        return isReachable("Control Tower" , lastKnown);
    }

    void withinTwoHops(){
        int start = graph.sectorId("Control Tower");
        if(start < 0)
            return;

        int dist[MAX_SECTORS];
        int q[MAX_SECTORS];
        int front = 0 , rear = 0;

        for(int i = 0; i < MAX_SECTORS; i++){
            dist[i] = -1;
        }

        dist[start] = 0;
        q[rear++] = start;

        cout << "Sector within 2 Hops of Control Tower: " << endl;

        while(front < rear){
            int a = q[front++];

            cout << graph.sectorName(a) << " (" << dist[a] << ")" << endl;
            if(dist[a] == 2)
                continue;
                
            for(RoadNode* p=graph.neighbors(a); p!=nullptr; p=p->next){
                if(p->cost > 0 && dist[p->to] == -1){
                    dist[p->to] = dist[a] + 1;
                    q[rear++] = p->to;
                }
            }    
        }
    }
    
};


struct Route{
    int nodes[MAX_SECTORS];
    int size, cost;
    bool found;
    Route(){ 
        size=0; 
        cost=0; 
        found = false; 
    }
};

struct DistItem { 
    int node;
    int dist; 
};
class DistanceHeap{
    DistItem data[2000];
    int size;
public:
    DistanceHeap(){ 
        size = 0; 
    }
    void push(int node,int dist){
        if(size>=2000) 
            return;

        int i=size++;
        data[i].node=node; 
        data[i].dist=dist;
        
        while(i>0){
            int p = (i-1) / 2;
            if(data[p].dist<=data[i].dist) 
                break;

            DistItem t = data[p];
            data[p] = data[i];
            data[i] = t;
            i = p;
        }
    }
    bool pop(DistItem& item){
        if(size==0) 
            return false;
        item = data[0];
        data[0] = data[--size];
        int i = 0;

        while(true){
            int a = i*2+1;
            int b = i*2+2;
            int best = i;
            if(a<size&&data[a].dist<data[best].dist)
                best = a;

            if(b<size&&data[b].dist<data[best].dist)
                best = b;

            if(best == i)
                break;

            DistItem t = data[i];
            data[i] = data[best];
            data[best] = t;
            i = best;
        }
        return true;
    }
};

class RouteFinder{
    SpaceGraph& graph;

public:
    RouteFinder(SpaceGraph& g):graph(g) {}
    // Dijkstra: heap chooses thee smallest tentative distance parent[] restores the route
    Route findFastestRoute(string from , string to){
        Route result;
        int start=graph.sectorId(from);
        int end=graph.sectorId(to);

        if(start < 0 || end < 0) 
            return result;

        int dist[MAX_SECTORS];
        int parent[MAX_SECTORS];

        for(int i = 0; i < MAX_SECTORS; i++){
            dist[i] = BIG_DIST;
            parent[i] = -1; 
        }

        DistanceHeap h;
        dist[start] = 0;
        h.push(start , 0);

        DistItem item;

        while(h.pop(item)){
            if(item.dist != dist[item.node]) 
                continue;

            if(item.node == end) 
                break;

            for(int j = 0; j < graph.getSectorCount(); j++){
                int w = graph.getWeight(item.node , j);

                if(w > 0 && dist[item.node]+w < dist[j]){
                    dist[j] = dist[item.node] + w;
                    parent[j] = item.node;
                    h.push(j , dist[j]);
                }
            }
        }

        if(dist[end] == BIG_DIST) 
            return result;

        result.found = true;
        result.cost = dist[end];

        int reverse[MAX_SECTORS];
        int n=0
        ;
        for(int p = end; p != -1; p=parent[p]) 
            reverse[n++] = p;

        result.size = n;

        for(int i = 0; i < n; i++) 
            result.nodes[i] = reverse[n-1-i];

        return result;
    }
    void printRoute(Route r){
        if(!r.found){
             cout << "No route found." << endl;
            return; 
        }
        for(int i = 0; i < r.size; i++){
            if(i == 0)
                cout << ":";
            else    
                cout << "->";

            cout << graph.sectorName(r.nodes[i]);
        }            
        
        cout << " total cost : " << r.cost << " ticks" << endl;
        cout << "Parent pointer trace : " << endl;

        for(int i=1;i<r.size;i++)
            cout << graph.sectorName(r.nodes[i]) << " <- " << graph.sectorName(r.nodes[i-1]) << endl;
    }
};

#endif