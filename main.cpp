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
    cout << "\tUMT PARKING SYSTEM" << endl;
    cout << "Enter Username: ";
    cin >> u;
    cout << "Enter your Password: ";
    cin >> p;

    if (u != ADMIN_USER || p != ADMIN_PASS)
    {
        cout << "Access Denied." << endl;
        cout << "Failed to fetch login credentials" << endl;
        return 0;
    }

    ParkingSystem sys(3, 2);

    int choice;
    while (true)
    {
        cout << "\n1. Vehicle Parking" << endl;
        cout << "2. Remove Vehicle from parking" << endl;
        cout << "3. UNDO previous Action" << endl;
        cout << "4. Exit" << endl;
        cout << "Select: ";
        cin >> choice;

        if (choice == 1)
        {
            string vID;
            int zID;
            cout << "Enter Vehicle Number: ";
            cin >> vID;
            cout << "Preferred Parking Zone (1-3): ";
            cin >> zID;

            Vehicle *v = new Vehicle(vID, zID);
            ParkingRequest *req = new ParkingRequest(v);
            sys.parkVehicle(req);

        } // <--- THIS WAS THE MISSING BRACKET
        else if (choice == 2)
        {
            int z, s;
            cout << "Enter Zone you want to enter: ";
            cin >> z;
            cout << "Enter Slot Number: ";
            cin >> s;
            sys.removeVehicle(z, s);
        }
        else if (choice == 3)
        {
            sys.undoLastAction(); // Now this will work!
        }
        else if (choice == 4)
        {
            cout << "Exiting the program in 3,2,1..." << endl;
            break;
        }
    }
    return 0;
}