#include "parkingsystem.h"
#include <iostream>
#include <fstream>
using namespace std;

ParkingSystem::ParkingSystem(int numZones, int slotsPerZone) {
    this->totalZones = numZones;
    this->engine = new AllocationEngine();
    this->rbManager = new RollbackManager();
    this->zones = new Zone*[totalZones];
    
    for (int i = 0; i < totalZones; i++) {
        this->zones[i] = new Zone(i + 1, 2); 
        this->zones[i]->addArea(1, slotsPerZone);
        this->zones[i]->addArea(2, slotsPerZone);
        
        for(int a=0; a < this->zones[i]->CurCount; a++) {
            for(int s=0; s < slotsPerZone; s++) {
                this->zones[i]->areas[a]->addSlot(s+1);
            }
        }
    }
    exportToHTML();
}

ParkingSystem::~ParkingSystem() {
    for(int i=0; i<totalZones; i++) delete zones[i];
    delete[] zones;
    delete engine;
    delete rbManager;
}

bool ParkingSystem::parkVehicle(ParkingRequest* req) {
    req->updateStatus(REQUESTED);
    ParkingSlot* slot = engine->assignSlot(req->vehicle, zones, totalZones);

    if(slot) {
        req->updateStatus(ALLOCATED);
        req->updateStatus(OCCUPIED);
        
        // SAVE HISTORY
        rbManager->pushOperation(PARK_ACTION, req, slot);

        cout << "Success: " << req->vehicle->vehId << " parked in Zone " << slot->zoneNum << endl;
        exportToHTML();
        return true;
    }
    req->updateStatus(CANCELLED);
    cout << "Failed to park." << endl;
    cout<<"Parking lot full, we are sorry :("<<endl;
    return false;
}

bool ParkingSystem::removeVehicle(int zID, int sID) {
    for(int i=0; i<totalZones; i++) {
        if(zones[i]->zoneID == zID) {
            for(int a=0; a<zones[i]->CurCount; a++) {
                for(int s=0; s<zones[i]->areas[a]->currentCount; s++) {
                    if(zones[i]->areas[a]->slots[s]->slotNum == sID) {
                        if(zones[i]->areas[a]->slots[s]->isOccupied) {
                            zones[i]->areas[a]->slots[s]->free();
                            cout << "Vehicle Removed from the zone." << endl;
                            exportToHTML();
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

void ParkingSystem::undoLastAction() {
    if (rbManager->isEmpty()) {
        cout << "Nothing to undo! check again" << endl;
        return;
    }

    HistoryNode* action = rbManager->popOperation();
    
    if (action->type == PARK_ACTION) {
        cout << "UNDO: Removing " << action->slot->vehId << " from history." << endl;
        action->slot->free(); 
        action->request->updateStatus(CANCELLED); 
    }
    
    exportToHTML();
    delete action; 
}

void ParkingSystem::showStatus() { }

void ParkingSystem::exportToHTML() {
    ofstream file("dashboard.html");
    file << "<html><head><meta http-equiv='refresh' content='2'>";
    file << "<style>body{background:#0d1117;color:white;font-family:monospace;} .zone{border:1px solid #333;margin:10px;padding:10px;} .occ{color:red;} .free{color:green;}</style>";
    file << "</head><body><h1>PARKING OS</h1>";
    for(int i=0; i<totalZones; i++) {
        file << "<div class='zone'><h3>Zone " << zones[i]->zoneID << "</h3>";
        for(int a=0; a<zones[i]->CurCount; a++) {
            file << "Area " << zones[i]->areas[a]->areaID << ": ";
            for(int s=0; s<zones[i]->areas[a]->currentCount; s++) {
                if(zones[i]->areas[a]->slots[s]->isOccupied)
                    file << "<span class='occ'>[" << zones[i]->areas[a]->slots[s]->vehId << "] </span>";
                else
                    file << "<span class='free'>[FREE] </span>";
            }
            file << "<br>";
        }
        file << "</div>";
    }
    file << "</body></html>";
    file.close();
}