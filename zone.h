#ifndef ZONE_H
#define ZONE_H
#include "parkingarea.h"

class Zone
{
public:
    int zoneID;
    int capacity;
    int CurCount; // Number of areas
    ParkingArea **areas;

    // Adjacency for Graph - Array of pointers to neighbor Zones
    Zone **neighbors;
    int neighborCount;
    int neighborCapacity;

    Zone(int id, int cap);
    ~Zone();
    void addArea(int areaID, int capacityPerArea);
    void addNeighbor(Zone *neighbor);
    bool isFull();
};
#endif