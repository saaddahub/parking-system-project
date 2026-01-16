#include <iostream>
#include <string>
#include "parkingsystem.h"
#include "vehicle.h"
#include "parkingrequest.h"

using namespace std;

const string ADMIN_USER = "saad";
const string ADMIN_PASS = "dsatheory";

int main()
{
    string u, p;
    cout << "=== UMT PARKING SYSTEM ===" << endl;
    cout << "USER: ";
    cin >> u;
    cout << "PASS: ";
    cin >> p;

    if (u != ADMIN_USER || p != ADMIN_PASS)
    {
        cout << "Access Denied." << endl;
        return 0;
    }

    ParkingSystem sys(3, 2);

    int choice;
    while (true)
    {
        cout << "\n1. Park Vehicle" << endl;
        cout << "2. Remove Vehicle" << endl;
        cout << "3. UNDO Last Action" << endl;
        cout << "4. Exit" << endl;
        cout << "Select: ";
        cin >> choice;

        if (choice == 1)
        {
            string vID;
            int zID;
            cout << "Enter Vehicle ID: ";
            cin >> vID;
            cout << "Preferred Zone (1-3): ";
            cin >> zID;

            Vehicle *v = new Vehicle(vID, zID);
            ParkingRequest *req = new ParkingRequest(v);
            sys.parkVehicle(req);

        } // <--- THIS WAS THE MISSING BRACKET
        else if (choice == 2)
        {
            int z, s;
            cout << "Enter Zone ID: ";
            cin >> z;
            cout << "Enter Slot ID: ";
            cin >> s;
            sys.removeVehicle(z, s);
        }
        else if (choice == 3)
        {
            sys.undoLastAction(); // Now this will work!
        }
        else if (choice == 4)
        {
            cout << "Exiting..." << endl;
            break;
        }
    }
    return 0;
}