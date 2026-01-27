#ifndef PARKINGSYSTEM_H
#define PARKINGSYSTEM_H

#include "zone.h"
#include "AllocationEngine.h"
#include "rollbackmanager.h"
#include "history.h"
#include "parkingrequest.h"
#include <string>

using namespace std;

class ParkingSystem
{
public:
    int totalZones;
    int globalTime;
    
    // Core Components
    Zone **zones;
    AllocationEngine *engine;
    RollbackManager *rbManager;
    HistoryManager *history;

    // Constructor & Destructor
    ParkingSystem(int numZones, int slotsPerZone);
    ~ParkingSystem();

    // Core Operations
    bool parkVehicle(ParkingRequest *req);
    bool removeVehicle(int zID, int sID);
    void undoLastAction();
    void showStatus();

    // UI Generator (The missing link!)
    void exportToHTML(); 
};

#endif