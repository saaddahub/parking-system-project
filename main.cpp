#include <iostream>
#include <fstream>
#include <string>
// REMOVED <cstdlib> - You don't need it!
#include "parkingsystem.h"
#include "vehicle.h"
#include "parkingrequest.h"

using namespace std;

// Helper function to check if a file exists
bool fileExists(const string& name) {
    ifstream f(name.c_str());
    return f.good();
}

int main()
{
    // Create System with 3 Zones, 2 Slots per Zone
    ParkingSystem sys(3, 2); 

    cout << "============================================" << endl;
    cout << "   LAHORE PARKING SYSTEM - BRIDGE ONLINE      " << endl;
    cout << "============================================" << endl;
    cout << "1. Open 'dashboard.html' in your browser manually." << endl;
    cout << "2. Use the UI buttons to Park/Remove." << endl;
    cout << "3. When asked, SAVE 'command.txt' to this folder." << endl;
    cout << "   (Waiting for commands...)" << endl;

    // Infinite loop to listen for commands
    while (true)
    {
        if (fileExists("command.txt"))
        {
            ifstream infile("command.txt");
            string action;
            infile >> action;

            if (action == "PARK") {
                string vID;
                int zID;
                infile >> vID >> zID;
                cout << "[RECV] Park Request: " << vID << " -> Zone " << zID << endl;
                
                Vehicle *v = new Vehicle(vID, zID);
                ParkingRequest *req = new ParkingRequest(v);
                sys.parkVehicle(req);
            }
            else if (action == "REMOVE") {
                int zID, sID;
                infile >> zID >> sID;
                cout << "[RECV] Remove Request: Zone " << zID << ", Slot " << sID << endl;
                sys.removeVehicle(zID, sID);
            }
            else if (action == "UNDO") {
                cout << "[RECV] Undo Request" << endl;
                sys.undoLastAction();
            }

            infile.close();
            
            // Delete the file so we don't process it twice
            remove("command.txt");
        }
        
        // Small delay to stop the CPU from working too hard
        for(int i=0; i<10000000; i++) {} 
    }

    return 0;
}