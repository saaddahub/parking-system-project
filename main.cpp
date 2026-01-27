#include <iostream>
#include <string>
#include <cstdlib>
#include <limits> // For numeric_limits
#include "parkingsystem.h"
#include "vehicle.h"
#include "parkingrequest.h"

using namespace std;

void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int main()
{
    // 1. Initialize System
    ParkingSystem sys(3, 2);

    cout << "============================================" << endl;
    cout << "   NEON PARKING SYSTEM - TERMINAL CONTROL   " << endl;
    cout << "============================================" << endl;
    cout << "Launching Dashboard..." << endl;
    
    // 2. Auto-Launch Browser
    system("start dashboard.html");

    int choice = 0;
    while (true)
    {
        cout << "\n--- COMMAND MENU ---" << endl;
        cout << "1. Park Vehicle" << endl;
        cout << "2. Remove Vehicle" << endl;
        cout << "3. Undo Last Action" << endl;
        cout << "4. Show Grid Status (Terminal)" << endl;
        cout << "5. Exit" << endl;
        cout << "Select Option: ";
        
        if (!(cin >> choice)) {
            cout << "Invalid input. Please enter a number." << endl;
            clearInput();
            continue;
        }

        if (choice == 1) {
            string vID;
            int zID;
            cout << "Enter Vehicle ID (e.g., CAR-01): ";
            cin >> vID;
            cout << "Enter Preferred Zone (1-" << sys.totalZones << "): ";
            if (!(cin >> zID)) {
                cout << "Invalid Zone ID." << endl;
                clearInput();
                continue;
            }

            Vehicle *v = new Vehicle(vID, zID);
            ParkingRequest *req = new ParkingRequest(v);
            
            if (sys.parkVehicle(req)) {
                cout << "SUCCESS: Vehicle parked." << endl;
            } else {
                cout << "FAILED: Could not park vehicle." << endl;
                // Cleanup if failed
                delete req; // Vehicle is deleted by Request destructor usually, or we need to check ownership
            }
        }
        else if (choice == 2) {
            int zID, sID;
            cout << "Enter Zone ID: ";
            cin >> zID;
            cout << "Enter Slot ID: ";
            if(cin >> sID) {
                if (sys.removeVehicle(zID, sID)) {
                    cout << "SUCCESS: Vehicle removed." << endl;
                } else {
                    cout << "FAILED: Slot empty or invalid." << endl;
                }
            } else {
                clearInput();
            }
        }
        else if (choice == 3) {
            cout << "Undoing last action..." << endl;
            sys.undoLastAction();
            cout << "Done." << endl;
        }
        else if (choice == 4) {
            sys.showStatus();
        }
        else if (choice == 5) {
            cout << "Exiting..." << endl;
            break;
        }
        else {
            cout << "Invalid option." << endl;
        }
        
        // Refresh dashboard after every command
        // Note: Code handles exportToHTML inside the methods
        cout << "Dashboard updated. Refresh browser to see changes." << endl;
    }

    return 0;
}
