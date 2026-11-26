#include <iostream>
using namespace std;
// Prototype console menu: it only prints messages for now
// Later each option will be connected to the real game
int main(){

    int choice= 0;
    // Keep showing the menu until the user picks 5 (Exit)
    while(choice!=5){

        cout << "==== MAYDAY 404 ====" << endl;
        cout << "1. Start game" << endl;
        cout << "2. Leaderboard" << endl;
        cout << "3. About" << endl;
        cout << "4. Rules" << endl;
        cout << "5. Exit" << endl;
        cout << "Choice: ";

        // Input validation: if the user types a letter instead of a number,
        //cin goes into a fail state and clears it etc
        // and set choice to 0 so the menu simply shows again
        if(!(cin>>choice)) {
            cin.clear();
            cin.ignore(10000, '\n');
            choice= 0;
        }

        if(choice==1) {
            cout << "Game is being connected" << endl;
        }
        else if(choice==2) {
            cout << "Leaderboard is being connected" << endl;
        }
        else if(choice==3) {
            cout << "ATC console project" << endl;
        }
        else if(choice==4) {
            cout << "Use two actions per tick" << endl;
        }
        else if(choice!=5) {
            cout << "Invalid! Enter 1 to 5" << endl;
        }
    }

    return 0;
}