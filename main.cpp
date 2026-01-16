#include <iostream>
#include <string>
#include "parkingsystem.h"
#include "vehicle.h"
using namespace std;

const string ADMIN_USER = "admin";
const string ADMIN_PASS = "secure123";

int main()
{
    string u, p;
    cout << "USER: ";
    cin >> u;
    cout << "PASS: ";
    cin >> p;
    if (u != ADMIN_USER || p != ADMIN_PASS)
        return 0;

    ParkingSystem sys(3, 2); // 3 Zones, 2 slots per area

    int choice;
    while (true)
    {
        cout << "\n1. Park  2. Remove  3. Exit\nSelect: ";
        cin >> choice;
        if (choice == 1)
        {
            string vID;
            int zID;
            cout << "VehID: ";
            cin >> vID;
            cout << "PrefZone: ";
            cin >> zID;
            Vehicle *v = new Vehicle(vID, zID);
            sys.parkVehicle(v);
        }
        else if (choice == 2)
        {
            int z, s;
            cout << "Zone: ";
            cin >> z;
            cout << "Slot: ";
            cin >> s;
            sys.removeVehicle(z, s);
        }
        else
            break;
    }
    return 0;
}