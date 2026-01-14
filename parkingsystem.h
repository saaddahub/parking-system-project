#ifndef PARKINGSYSTEM_H
#define PARKINGSYSTEM_H

#include "zone.h"
#include "vehicle.h"
#include "allocationengine.h"

class ParkingSystem
{
public:
    zone **zones;
    int totalZones;
    AllocationEngine *engine;

    ParkingSystem(int numZones, int slotsPerZone);
    ~ParkingSystem();

    bool parkVehicle(vehicle *v);
    bool removeVehicle(int zoneID, int slotNum);
    void showStatus();
};

#endif