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
    // Fix Memory Leaks
    for (int i = 0; i < totalZones; ++i) {
        // Assuming Zone destructor handles its areas/slots or we need to look at Zone
        delete zones[i]; 
    }
    delete[] zones;
    delete engine;
    delete rbManager;
    delete history;
}

bool ParkingSystem::parkVehicle(ParkingRequest *req)
{
    req->updateStatus(0);
    ParkingSlot *slot = engine->assignSlot(req->vehicle, zones, totalZones);

    if (slot)
    {
        if (slot->zoneNum != req->vehicle->preferredZoneID)
        {
            req->penaltyCost = 50.0;
        }

        req->updateStatus(1);
        req->updateStatus(2);
        req->startTime = this->globalTime;

        slot->occupy(req->vehicle->vehId, req);
        rbManager->pushOperation(PARK_ACTION, req, slot);

        this->globalTime++;
        exportToHTML();
        return true;
    }

    req->updateStatus(4);
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
                            finishedReq->updateStatus(3);
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

void ParkingSystem::showStatus() {
    cout << "\n--- GRID STATUS ---" << endl;
    for(int i=0; i < totalZones; i++) {
        cout << "ZONE " << zones[i]->zoneID << ":" << endl;
        for (int a = 0; a < zones[i]->CurCount; a++) {
             for (int s = 0; s < zones[i]->areas[a]->currentCount; s++) {
                 ParkingSlot* slot = zones[i]->areas[a]->slots[s];
                 cout << "  [ S" << slot->slotNum << ": ";
                 if(slot->isOccupied) {
                     cout << slot->vehId;
                     if(slot->currentReq && slot->currentReq->penaltyCost > 0) cout << "(!)"; 
                 } else {
                     cout << "FREE";
                 }
                 cout << " ]";
             }
             cout << endl;
        }
    }
    cout << "-------------------\n" << endl;
}

// --- DASHBOARD UI GENERATOR (READ ONLY) ---
// Premium Design, No JS Interactions needed for user
void ParkingSystem::exportToHTML()
{
    ofstream file("dashboard.html");

    file << "<!DOCTYPE html><html><head>";
    file << "<title>NEON PARKING MONITOR</title>";
    
    // Auto-refresh can be annoying if manually handling, but user asked for "browser should show me".
    // Since we are doing "manual" reload, I will NOT add meta refresh.
    
    file << "<style>";
    file << "@import url('https://fonts.googleapis.com/css2?family=Orbitron:wght@400;700&family=Rajdhani:wght@500;700&display=swap');";
    file << "body { background-color: #050505; color: #fff; font-family: 'Rajdhani', sans-serif; margin: 0; padding: 20px; overflow-x: hidden; }";
    file << "::-webkit-scrollbar { width: 8px; } ::-webkit-scrollbar-thumb { background: #333; border-radius: 4px; }";
    
    file << ".container { max-width: 1200px; margin: 0 auto; }";
    file << "header { text-align: center; margin-bottom: 40px; border-bottom: 2px solid #222; padding-bottom: 20px; }";
    file << "h1 { font-family: 'Orbitron', sans-serif; font-size: 3em; margin: 0; background: linear-gradient(90deg, #00f260, #0575E6); -webkit-background-clip: text; -webkit-text-fill-color: transparent; text-shadow: 0 0 30px rgba(0, 242, 96, 0.3); }";
    file << ".subtitle { color: #666; font-size: 1.2em; letter-spacing: 2px; margin-top: 10px; }";

    // KPI Cards
    file << ".kpi-grid { display: flex; justify-content: center; gap: 20px; margin-bottom: 50px; }";
    file << ".kpi-card { background: #111; border: 1px solid #333; padding: 20px 40px; border-radius: 12px; text-align: center; min-width: 150px; box-shadow: 0 0 20px rgba(0,0,0,0.5); }";
    file << ".kpi-val { font-size: 2.5em; font-weight: bold; color: #fff; font-family: 'Orbitron'; }";
    file << ".kpi-label { color: #888; font-size: 0.9em; text-transform: uppercase; letter-spacing: 1px; }";

    // Zone Grid
    file << ".zone-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(350px, 1fr)); gap: 30px; }";
    file << ".zone-card { background: #0f1012; border-radius: 16px; border: 1px solid #1f2026; padding: 25px; position: relative; overflow: hidden; transition: transform 0.3s; }";
    file << ".zone-card:hover { transform: translateY(-5px); border-color: #444; }";
    file << ".zone-card::before { content: ''; position: absolute; top: 0; left: 0; width: 100%; height: 4px; background: linear-gradient(90deg, #00f260, #0575E6); }";
    
    file << ".zone-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 20px; }";
    file << ".zone-title { font-family: 'Orbitron'; font-size: 1.5em; color: #e0e0e0; }";
    
    // Slots
    file << ".slot-list { display: grid; gap: 10px; }";
    file << ".slot { display: flex; justify-content: space-between; align-items: center; padding: 15px; border-radius: 8px; background: #18191f; border-left: 5px solid #333; font-weight: 700; font-size: 1.1em; transition: 0.2s; }";
    
    // States
    file << ".slot.free { border-left-color: #00f260; color: #aaa; }";
    file << ".slot.free .status { color: #00f260; background: rgba(0, 242, 96, 0.1); padding: 4px 10px; border-radius: 4px; font-size: 0.8em; }";
    
    file << ".slot.occupied { border-left-color: #ff0055; background: #1f1015; color: #fff; box-shadow: 0 0 15px rgba(255, 0, 85, 0.1); }";
    file << ".slot.occupied .status { color: #ff0055; background: rgba(255, 0, 85, 0.1); padding: 4px 10px; border-radius: 4px; font-size: 0.8em; }";
    
    file << ".veh-id { letter-spacing: 1px; }";
    file << ".penalty-tag { background: #ffcc00; color: #000; font-size: 0.7em; padding: 2px 6px; border-radius: 4px; margin-left: 10px; font-weight: bold; }";

    // Footer
    file << ".footer { margin-top: 50px; text-align: center; color: #444; font-size: 0.8em; }";

    file << "</style></head><body>";

    file << "<div class='container'>";
    file << "<header><h1>NEON PARKING</h1><div class='subtitle'>REAL-TIME MONITORING SYSTEM</div></header>";

    // KPI Section
    file << "<div class='kpi-grid'>";
    file << "<div class='kpi-card'><div class='kpi-val'>" << history->count << "</div><div class='kpi-label'>Total Parked</div></div>";
    file << "<div class='kpi-card'><div class='kpi-val'>" << fixed << setprecision(1) << history->getAverageDuration() << "s</div><div class='kpi-label'>Avg Duration</div></div>";
    file << "<div class='kpi-card'><div class='kpi-val'>$" << (int)history->getTotalRevenue() << "</div><div class='kpi-label'>Revenue</div></div>";
    file << "</div>";

    // Zones Section
    file << "<div class='zone-grid'>";
    for (int i = 0; i < totalZones; i++)
    {
        file << "<div class='zone-card'>";
        file << "<div class='zone-header'><div class='zone-title'>ZONE " << zones[i]->zoneID << "</div></div>";
        file << "<div class='slot-list'>";
        
        for (int a = 0; a < zones[i]->CurCount; a++)
        {
            for (int s = 0; s < zones[i]->areas[a]->currentCount; s++)
            {
                ParkingSlot *slot = zones[i]->areas[a]->slots[s];
                
                if (slot->isOccupied) {
                    file << "<div class='slot occupied'>";
                    file << "<span class='veh-id'>" << slot->vehId << "</span>";
                    file << "<div>";
                    if(slot->currentReq && slot->currentReq->penaltyCost > 0) file << "<span class='penalty-tag'>PENALTY</span> ";
                    file << "<span class='status'>OCCUPIED</span>";
                    file << "</div></div>";
                } else {
                    file << "<div class='slot free'>";
                    file << "<span>Slot " << slot->slotNum << "</span>";
                    file << "<span class='status'>OPEN</span>";
                    file << "</div>";
                }
            }
        }
        file << "</div></div>"; // End Zone Card
    }
    file << "</div>"; // End Zone Grid

    file << "<div class='footer'>SYSTEM TIME: " << globalTime << "TICKS | REFRESH BROWSER TO UPDATE</div>";
    file << "</div>";
    
    file << "</body></html>";
    file.close();
}
