#include "parkingsystem.h"
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;

ParkingSystem::ParkingSystem(int numZones, int slotsPerZone)
{
    this->totalZones = numZones;
    this->globalTime = 0;
    this->engine = new AllocationEngine();
    this->rbManager = new RollbackManager();
    this->history = new HistoryManager();
    this->zones = new Zone *[totalZones];

    for (int i = 0; i < totalZones; i++)
    {
        this->zones[i] = new Zone(i + 1, 2);
        this->zones[i]->addArea(1, slotsPerZone);
        this->zones[i]->addArea(2, slotsPerZone);

        for (int a = 0; a < this->zones[i]->CurCount; a++)
        {
            for (int s = 0; s < slotsPerZone; s++)
            {
                this->zones[i]->areas[a]->addSlot(s + 1);
            }
        }
    }
    exportToHTML();
}

ParkingSystem::~ParkingSystem()
{
    // Basic cleanup
}

bool ParkingSystem::parkVehicle(ParkingRequest *req)
{
    req->updateStatus(0); // REQUESTED
    ParkingSlot *slot = engine->assignSlot(req->vehicle, zones, totalZones);

    if (slot)
    {
        if (slot->zoneNum != req->vehicle->preferredZoneID)
        {
            req->penaltyCost = 50.0;
        }

        req->updateStatus(1); // ALLOCATED
        req->updateStatus(2); // OCCUPIED
        req->startTime = this->globalTime;

        slot->occupy(req->vehicle->vehId, req);
        rbManager->pushOperation(PARK_ACTION, req, slot);

        this->globalTime++;
        exportToHTML();
        return true;
    }

    req->updateStatus(4); // CANCELLED
    return false;
}

bool ParkingSystem::removeVehicle(int zID, int sID)
{
    for (int i = 0; i < totalZones; i++)
    {
        if (zones[i]->zoneID == zID)
        {
            for (int a = 0; a < zones[i]->CurCount; a++)
            {
                for (int s = 0; s < zones[i]->areas[a]->currentCount; s++)
                {
                    ParkingSlot *slot = zones[i]->areas[a]->slots[s];
                    if (slot->slotNum == sID && slot->isOccupied)
                    {
                        ParkingRequest *finishedReq = slot->currentReq;
                        if (finishedReq != nullptr)
                        {
                            finishedReq->endTime = this->globalTime;
                            finishedReq->updateStatus(3); // COMPLETED
                            history->addRecord(finishedReq);
                        }
                        slot->free();
                        this->globalTime++;
                        exportToHTML();
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void ParkingSystem::undoLastAction()
{
    if (rbManager->isEmpty())
        return;

    RollbackNode *action = rbManager->popOperation();
    if (action->type == PARK_ACTION)
    {
        action->slot->free();
        action->request->updateStatus(4);
    }
    exportToHTML();
}

void ParkingSystem::showStatus() {}

void ParkingSystem::exportToHTML()
{
    ofstream file("dashboard.html");
    file << "<html><head><meta http-equiv='refresh' content='2'>";
    file << "<style>body{background: #0f0c29; color: white; font-family: sans-serif; text-align: center;}";
    file << ".zone-card{background: rgba(255,255,255,0.05); border-radius: 15px; padding: 20px; display: inline-block; margin: 10px; min-width: 250px;}";
    file << ".stats{background: rgba(0,0,0,0.3); padding: 20px; margin-top: 20px; border-radius: 15px;}";
    file << "</style></head><body><h1>Parking OS Dashboard</h1>";

    for (int i = 0; i < totalZones; i++)
    {
        file << "<div class='zone-card'><h3>Zone " << zones[i]->zoneID << "</h3>";
        for (int a = 0; a < zones[i]->CurCount; a++)
        {
            for (int s = 0; s < zones[i]->areas[a]->currentCount; s++)
            {
                if (zones[i]->areas[a]->slots[s]->isOccupied)
                    file << "[ " << zones[i]->areas[a]->slots[s]->vehId << " ] ";
                else
                    file << "[ FREE ] ";
            }
            file << "<br>";
        }
        file << "</div>";
    }

    file << "<div class='stats'><h2>Live Analytics</h2>";
    file << "Total Trips: " << history->count << " | ";
    file << "Avg Duration: " << fixed << setprecision(1) << history->getAverageDuration() << " ticks | ";
    file << "Revenue: $" << history->getTotalRevenue();
    file << "</div></body></html>";
    file.close();
}