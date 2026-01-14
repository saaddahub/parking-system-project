#include <iostream>
#include "parkingsystem.h"
#include "vehicle.h"

using namespace std;

int main()
{
    ParkingSystem cityParking(3, 2);

    cityParking.showStatus();

    vehicle *v1 = new vehicle("ABC-123", 1);
    vehicle *v2 = new vehicle("XYZ-789", 1);
    vehicle *v3 = new vehicle("LHR-555", 1);
    vehicle *v4 = new vehicle("DXB-999", 2);

    cityParking.parkVehicle(v1);
    cityParking.parkVehicle(v2);
    cityParking.parkVehicle(v3);
    cityParking.parkVehicle(v4);

    cityParking.showStatus();

    cityParking.removeVehicle(1, 1);

    cityParking.showStatus();

    return 0;
}