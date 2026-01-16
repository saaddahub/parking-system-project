#ifndef VEHICLE_H
#define VEHICLE_H
#include <string>
using namespace std;

class Vehicle
{
public:
    string vehId;
    int preferredZoneID;

    Vehicle(string id, int zone);
};
#endif