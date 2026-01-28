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
    
    // Sites
    string* siteNames;
    int numSites;
    int zonesPerSite;
    
    // Core Components
    Zone **zones;
    AllocationEngine *engine;
    RollbackManager *rbManager;
    HistoryManager *history;

    // Constructor & Destructor
    ParkingSystem(int numZones, int slotsPerZone);
    ~ParkingSystem();

    // Core Operations
    // Core Operations
    bool parkVehicle(ParkingRequest *req);
    // Overload for Site-Specific Parking if needed, or handle in vehicle?
    // User input: Site Index, Zone Index.
    // Vehicle PrefZoneID is likely just 1-3. We need to store SiteID in Vehicle too?
    // Or just Map (Site, Zone) -> GlobalZoneID before creating Vehicle.
    // Let's stick to Global ID internally, but UI presents Site + Zone.
    
    bool removeVehicle(int zID, int sID);
    void undoLastAction();
    void rollback(int k); // Batch Rollback
    void showStatus();

    // UI Generator (The missing link!)
    void exportToHTML(); 
};

#endif