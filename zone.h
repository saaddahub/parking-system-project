#ifndef ZONE_H
#define ZONE_H
#include "parkingarea.h"

class Zone
{
public:
    int zoneID;
    int capacity;
    int CurCount; // Using your PascalCase variable
    ParkingArea **areas;

    Zone(int id, int cap);
    ~Zone();
    void addArea(int areaID, int capacityPerArea);
    bool isFull();
};
#endif