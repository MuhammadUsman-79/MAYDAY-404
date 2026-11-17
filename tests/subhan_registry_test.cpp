#include <iostream>
#include "../headerFiles/Registry.h"
using namespace std;
int main()
{
    Registration r;
    Flight a = makeFlight("AA01","A","B",10,9,0);
    Flight b = makeFlight("BB02","A","B",8,5,0);
    if(r.addFlight(a)==NULL || r.addFlight(b)==NULL) return 1;
    if(r.addFlight(a)!=NULL) return 2; // duplicate rejected
    if(r.find("BB02")==NULL || r.countFlights()!=2) return 3;
    if(!r.removeFlight("AA01") || r.find("AA01")!=NULL) return 4;
    cout << "REGISTRY TEST PASSED" << endl;
    return 0;
}
