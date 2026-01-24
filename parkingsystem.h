#ifndef PARKINGSYSTEM_H
#define PARKINGSYSTEM_H

#include "zone.h"
#include "vehicle.h"
#include "allocationengine.h"
#include "parkingrequest.h"
#include "rollbackmanager.h"
#include "History.h"

class ParkingSystem
{
public:
    Zone **zones;
    int totalZones;
    AllocationEngine *engine;
    RollbackManager *rbManager;
    HistoryManager *history;
    int globalTime;

    ParkingSystem(int numZones, int slotsPerZone);
    ~ParkingSystem();

    bool parkVehicle(ParkingRequest *req);
    bool removeVehicle(int zoneID, int slotNum);

    void undoLastAction();
    void showStatus();
    void exportToHTML();
};

#endif