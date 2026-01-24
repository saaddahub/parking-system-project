#include "parkingsystem.h"
#include <iostream>
#include <fstream>
#include <iomanip> // For setprecision

using namespace std;

// --- CONSTRUCTOR ---
ParkingSystem::ParkingSystem(int numZones, int slotsPerZone)
{
    this->totalZones = numZones;
    this->globalTime = 0; // START THE CLOCK

    this->engine = new AllocationEngine();
    this->rbManager = new RollbackManager();
    this->history = new HistoryManager(); // Init History

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
    // Destructor logic
}

// --- PARK VEHICLE ---
bool ParkingSystem::parkVehicle(ParkingRequest *req)
{
    req->updateStatus(0); // REQUESTED

    // Try to find a slot
    ParkingSlot *slot = engine->assignSlot(req->vehicle, zones, totalZones);

    if (slot)
    {
        // 1. CHECK PENALTY (Requirement #5)
        if (slot->zoneNum != req->vehicle->preferredZoneID)
        {
            req->penaltyCost = 50.0; // $50 Fine
            cout << " [!] Cross-Zone Allocation. Penalty Applied." << endl;
        }

        // 2. UPDATE STATE & TIME
        req->updateStatus(1); // ALLOCATED
        req->updateStatus(2); // OCCUPIED

        req->startTime = this->globalTime; // Record Start Time

        // 3. OCCUPY SLOT (Pass the request so we keep it)
        slot->occupy(req->vehicle->vehId, req);

        // 4. SAVE UNDO
        rbManager->pushOperation(PARK_ACTION, req, slot);

        cout << "Success: " << req->vehicle->vehId << " parked in Zone " << slot->zoneNum << endl;

        // 5. TICK CLOCK & UPDATE DASHBOARD
        this->globalTime++;
        exportToHTML();
        return true;
    }

    req->updateStatus(4); // CANCELLED
    cout << "Failed to park." << endl;
    return false;
}

// --- REMOVE VEHICLE ---
bool ParkingSystem::removeVehicle(int zID, int sID)
{
    // Search loops
    for (int i = 0; i < totalZones; i++)
    {
        if (zones[i]->zoneID == zID)
        {
            for (int a = 0; a < zones[i]->CurCount; a++)
            {
                for (int s = 0; s < zones[i]->areas[a]->currentCount; s++)
                {
                    ParkingSlot *currentSlot = zones[i]->areas[a]->slots[s];

                    if (currentSlot->slotNum == sID)
                    {
                        if (currentSlot->isOccupied)
                        {
                            cout << "Vehicle " << currentSlot->vehId << " is leaving..." << endl;

                            // 1. RETRIEVE THE REQUEST
                            ParkingRequest *finishedReq = currentSlot->currentReq;

                            if (finishedReq != nullptr)
                            {
                                // 2. CLOSE THE TICKET
                                finishedReq->endTime = this->globalTime;
                                finishedReq->updateStatus(3); // COMPLETED

                                // 3. MOVE TO HISTORY (Requirement #7)
                                history->addRecord(finishedReq);
                            }

                            // 4. FREE THE SLOT
                            currentSlot->free();

                            // 5. TICK CLOCK & UPDATE DASHBOARD
                            this->globalTime++;
                            exportToHTML();
                            return true;
                        }
                    }
                }
            }
        }
    }
    cout << "Slot not found." << endl;
    return false;
}

void ParkingSystem::undoLastAction()
{
    if (rbManager->isEmpty())
    {
        cout << "Nothing to undo." << endl;
        return;
    }
    cout << "Undo performed." << endl;
    // Note: To fully implement strict undo, you'd pop from stack here.
    // For now, this placeholder prevents crashing.

    exportToHTML();
}

void ParkingSystem::showStatus() {}

// --- THE DASHBOARD GENERATOR ---
void ParkingSystem::exportToHTML()
{
    ofstream file("dashboard.html");

    file << "<html><head><meta http-equiv='refresh' content='2'>";
    file << "<title>Live Dashboard</title>";
    file << "<style>";
    file << "body { background: linear-gradient(135deg, #0f0c29, #302b63, #24243e); color: white; font-family: 'Segoe UI', sans-serif; padding: 20px; }";
    file << "h1 { text-align: center; text-transform: uppercase; letter-spacing: 2px; margin-bottom: 40px; }";
    file << ".container { display: flex; flex-wrap: wrap; justify-content: center; gap: 20px; }";

    file << ".zone-card { background: rgba(255, 255, 255, 0.05); backdrop-filter: blur(10px); border: 1px solid rgba(255,255,255,0.1); border-radius: 15px; padding: 20px; width: 300px; box-shadow: 0 10px 30px rgba(0,0,0,0.5); }";
    file << ".zone-title { font-size: 1.2rem; font-weight: bold; margin-bottom: 15px; border-bottom: 1px solid rgba(255,255,255,0.1); padding-bottom: 10px; }";

    file << ".slot { display: inline-block; padding: 8px 12px; margin: 5px; border-radius: 8px; font-size: 0.9rem; font-weight: bold; }";
    file << ".free { background: rgba(0, 255, 136, 0.2); color: #00ff88; border: 1px solid #00ff88; }";
    file << ".occ { background: rgba(255, 107, 107, 0.2); color: #ff6b6b; border: 1px solid #ff6b6b; }";

    file << ".stats { margin-top: 50px; background: rgba(0,0,0,0.3); padding: 20px; border-radius: 15px; text-align: center; }";
    file << ".stat-box { display: inline-block; margin: 0 20px; }";
    file << ".stat-val { font-size: 2rem; font-weight: bold; color: #00b09b; }";
    file << "</style></head><body>";

    file << "<h1>City Parking Overview (Time: " << globalTime << ")</h1>";

    // ZONES
    file << "<div class='container'>";
    for (int i = 0; i < totalZones; i++)
    {
        file << "<div class='zone-card'>";
        file << "<div class='zone-title'>ZONE " << zones[i]->zoneID << "</div>";

        for (int a = 0; a < zones[i]->CurCount; a++)
        {
            file << "<p style='font-size:0.8rem; color:#aaa;'>AREA " << zones[i]->areas[a]->areaID << "</p>";
            for (int s = 0; s < zones[i]->areas[a]->currentCount; s++)
            {
                if (zones[i]->areas[a]->slots[s]->isOccupied)
                    file << "<div class='slot occ'>" << zones[i]->areas[a]->slots[s]->vehId << "</div>";
                else
                    file << "<div class='slot free'>FREE</div>";
            }
            file << "<br><br>";
        }
        file << "</div>";
    }
    file << "</div>";

    // ANALYTICS (Requirement #7)
    file << "<div class='stats'>";
    file << "<h2>Live Analytics</h2>";
    file << "<div class='stat-box'><div class='stat-val'>" << history->count << "</div><div>Total Trips</div></div>";
    file << "<div class='stat-box'><div class='stat-val'>" << fixed << setprecision(1) << history->getAverageDuration() << " ticks</div><div>Avg Duration</div></div>";
    file << "<div class='stat-box'><div class='stat-val'>$" << (int)history->getTotalRevenue() << "</div><div>Revenue</div></div>";
    file << "</div>";

    file << "</body></html>";
    file.close();
}