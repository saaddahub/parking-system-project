#include "parkingsystem.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>

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
    // exportToHTML();
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
        // exportToHTML(); // Logic decoupled from UI
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
                        // exportToHTML();
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
    // exportToHTML();
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

// --- DASHBOARD UI GENERATOR (SERVER RESPONSE) ---
string ParkingSystem::getHTML()
{
    stringstream ss;
    ss << "<!DOCTYPE html><html><head>";
    ss << "<title>NEON PARKING MONITOR</title>";
    
    // Auto-refresh mechanism? No, actions trigger reload.
    // Maybe a periodic refresh for read-only views? 
    // Let's add a small script to refresh every 5s just in case, but actions are instant.
    
    ss << "<style>";
    ss << "@import url('https://fonts.googleapis.com/css2?family=Orbitron:wght@400;700&family=Rajdhani:wght@500;700&display=swap');";
    ss << "body { background-color: #050505; color: #fff; font-family: 'Rajdhani', sans-serif; margin: 0; padding: 20px; overflow-x: hidden; text-align: center; }";
    ss << "::-webkit-scrollbar { width: 8px; } ::-webkit-scrollbar-thumb { background: #333; border-radius: 4px; }";
    
    ss << ".container { max-width: 1200px; margin: 0 auto; }";
    ss << "header { margin-bottom: 40px; border-bottom: 2px solid #222; padding-bottom: 20px; }";
    ss << "h1 { font-family: 'Orbitron', sans-serif; font-size: 3em; margin: 0; background: linear-gradient(90deg, #00f260, #0575E6); -webkit-background-clip: text; -webkit-text-fill-color: transparent; text-shadow: 0 0 30px rgba(0, 242, 96, 0.3); }";

    // Controls
    ss << ".control-panel { background: #111; padding: 20px; border-radius: 12px; margin-bottom: 40px; border: 1px solid #333; display: flex; justify-content: center; gap: 20px; flex-wrap: wrap; }";
    ss << "form { display: flex; align-items: center; gap: 10px; }";
    ss << "input { background: #222; border: 1px solid #444; color: white; padding: 10px; border-radius: 6px; width: 100px; }";
    ss << "button { padding: 10px 20px; border: none; border-radius: 6px; font-weight: bold; cursor: pointer; transition: 0.2s; font-family: 'Orbitron'; }";
    ss << ".btn-green { background: #00f260; color: #000; } .btn-green:hover { box-shadow: 0 0 15px #00f260; }";
    ss << ".btn-red { background: #ff0055; color: #fff; } .btn-red:hover { box-shadow: 0 0 15px #ff0055; }";
    ss << ".btn-gold { background: #ffcc00; color: #000; } .btn-gold:hover { box-shadow: 0 0 15px #ffcc00; }";

    // KPI Cards
    ss << ".kpi-grid { display: flex; justify-content: center; gap: 20px; margin-bottom: 50px; }";
    ss << ".kpi-card { background: #111; border: 1px solid #333; padding: 20px 40px; border-radius: 12px; min-width: 150px; box-shadow: 0 0 20px rgba(0,0,0,0.5); }";
    ss << ".kpi-val { font-size: 2.5em; font-weight: bold; color: #fff; font-family: 'Orbitron'; }";
    ss << ".kpi-label { color: #888; font-size: 0.9em; text-transform: uppercase; letter-spacing: 1px; }";

    // Zone Grid
    ss << ".zone-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(350px, 1fr)); gap: 30px; text-align: left; }";
    ss << ".zone-card { background: #0f1012; border-radius: 16px; border: 1px solid #1f2026; padding: 25px; position: relative; overflow: hidden; transition: transform 0.3s; }";
    ss << ".zone-card::before { content: ''; position: absolute; top: 0; left: 0; width: 100%; height: 4px; background: linear-gradient(90deg, #00f260, #0575E6); }";
    ss << ".zone-title { font-family: 'Orbitron'; font-size: 1.5em; color: #e0e0e0; margin-bottom: 20px; }";
    
    // Slots
    ss << ".slot-list { display: grid; gap: 10px; }";
    ss << ".slot { display: flex; justify-content: space-between; align-items: center; padding: 15px; border-radius: 8px; background: #18191f; border-left: 5px solid #333; font-weight: 700; font-size: 1.1em; }";
    ss << ".slot.free { border-left-color: #00f260; color: #aaa; }";
    ss << ".slot.occupied { border-left-color: #ff0055; background: #1f1015; color: #fff; }";
    ss << ".status { font-size: 0.8em; padding: 4px 10px; border-radius: 4px; }";
    ss << ".occupied .status { background: rgba(255,0,85,0.2); color: #ff0055; }";
    ss << ".free .status { background: rgba(0,242,96,0.1); color: #00f260; }";

    ss << "</style></head><body>";

    ss << "<div class='container'>";
    ss << "<header><h1>NEON PARKING</h1></header>";

    // CONTROLS FORM (HTTP GET REQUESTS)
    ss << "<div class='control-panel'>";
    
    // Park Form
    ss << "<form action='/park' method='GET'>";
    ss << "<input type='text' name='v' placeholder='Vehicle ID' required>";
    ss << "<input type='number' name='z' placeholder='Zone' required style='width:60px'>";
    ss << "<button type='submit' class='btn-green'>PARK</button>";
    ss << "</form>";

    // Remove Form
    ss << "<form action='/remove' method='GET'>";
    ss << "<input type='number' name='z' placeholder='Zone' required style='width:60px'>";
    ss << "<input type='number' name='s' placeholder='Slot' required style='width:60px'>";
    ss << "<button type='submit' class='btn-red'>REMOVE</button>";
    ss << "</form>";

    // Undo Form
    ss << "<form action='/undo' method='GET'>";
    ss << "<button type='submit' class='btn-gold'>UNDO</button>";
    ss << "</form>";

    ss << "</div>";

    // KPI Section
    ss << "<div class='kpi-grid'>";
    ss << "<div class='kpi-card'><div class='kpi-val'>" << history->count << "</div><div class='kpi-label'>Total Parked</div></div>";
    ss << "<div class='kpi-card'><div class='kpi-val'>" << fixed << setprecision(1) << history->getAverageDuration() << "s</div><div class='kpi-label'>Avg Duration</div></div>";
    ss << "<div class='kpi-card'><div class='kpi-val'>$" << (int)history->getTotalRevenue() << "</div><div class='kpi-label'>Revenue</div></div>";
    ss << "</div>";

    // Zones Section
    ss << "<div class='zone-grid'>";
    for (int i = 0; i < totalZones; i++)
    {
        ss << "<div class='zone-card'>";
        ss << "<div class='zone-title'>ZONE " << zones[i]->zoneID << "</div>";
        ss << "<div class='slot-list'>";
        
        for (int a = 0; a < zones[i]->CurCount; a++)
        {
            for (int s = 0; s < zones[i]->areas[a]->currentCount; s++)
            {
                ParkingSlot *slot = zones[i]->areas[a]->slots[s];
                
                if (slot->isOccupied) {
                    ss << "<div class='slot occupied'>";
                    ss << "<span>" << slot->vehId << "</span>";
                    ss << "<div>";
                    if(slot->currentReq && slot->currentReq->penaltyCost > 0) ss << "<span style='color:gold;margin-right:5px'>⚠</span> ";
                    ss << "<span class='status'>OCCUPIED</span>";
                    ss << "</div></div>";
                } else {
                    ss << "<div class='slot free'>";
                    ss << "<span>Slot " << slot->slotNum << "</span>";
                    ss << "<span class='status'>OPEN</span>";
                    ss << "</div>";
                }
            }
        }
        ss << "</div></div>";
    }
    ss << "</div>";
    
    ss << "<footer style='margin-top:50px;color:#444'>System Time: " << globalTime << "</footer>";
    ss << "</div></body></html>";
    
    return ss.str();
}
