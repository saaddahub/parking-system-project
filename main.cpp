#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib> // for remove()
#include "parkingsystem.h"
#include "vehicle.h"
#include "parkingrequest.h"

using namespace std;

// Helper to check if file exists
bool fileExists(const string &name)
{
    ifstream f(name.c_str());
    return f.good();
}

int main()
{
    ParkingSystem sys(3, 2);

    cout << "============================================" << endl;
    cout << "   NEON PARKING SYSTEM - BRIDGE ONLINE      " << endl;
    cout << "============================================" << endl;
    cout << "1. Open 'dashboard.html' in your browser." << endl;
    cout << "2. Use the UI buttons to Park/Remove." << endl;
    cout << "3. When asked, SAVE 'command.txt' to this folder." << endl;
    cout << "   (Waiting for commands...)" << endl;

    while (true)
    {
        // 1. WATCH FOR FILE
        if (fileExists("command.txt"))
        {
            // 2. READ COMMAND
            ifstream infile("command.txt");
            string action;
            infile >> action;

            if (action == "PARK")
            {
                string vID;
                int zID;
                infile >> vID >> zID;
                cout << "[RECV] Park Request: " << vID << " -> Zone " << zID << endl;

                Vehicle *v = new Vehicle(vID, zID);
                ParkingRequest *req = new ParkingRequest(v);
                sys.parkVehicle(req);
            }
            else if (action == "REMOVE")
            {
                int zID, sID;
                infile >> zID >> sID;
                cout << "[RECV] Remove Request: Zone " << zID << ", Slot " << sID << endl;
                sys.removeVehicle(zID, sID);
            }
            else if (action == "UNDO")
            {
                cout << "[RECV] Undo Request" << endl;
                sys.undoLastAction();
            }

            infile.close();

            // 3. DELETE FILE (So we don't read it twice)
            remove("command.txt");
        }

        // Tiny manual delay loop to prevent high CPU usage
        for (int i = 0; i < 10000000; i++)
        {
        }
    }

    return 0;
}