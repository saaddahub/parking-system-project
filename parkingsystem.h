#ifndef PARKINGSYSTEM_H
#define PARKINGSYSTEM_H

#include "zone.h"
#include "vehicle.h"
#include "allocationengine.h"
#include "parkingrequest.h"
#include "rollbackmanager.h" // <--- ADDED THIS

class ParkingSystem
{
public:
    Zone **zones;
    int totalZones;
    AllocationEngine *engine;
    RollbackManager *rbManager; // <--- ADDED THIS

    ParkingSystem(int numZones, int slotsPerZone);
    ~ParkingSystem();

    bool parkVehicle(ParkingRequest *req);
    bool removeVehicle(int zoneID, int slotNum);

    void undoLastAction(); // <--- THIS FIXES YOUR SECOND ERROR

    void showStatus();
    void exportToHTML();
};

#endif