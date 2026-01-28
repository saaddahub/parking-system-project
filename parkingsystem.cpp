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
    // Initialize History with NumZones
    this->history = new HistoryManager(numZones); 
    this->zones = new Zone *[totalZones];

    // 1. Create Zones
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

    // 2. Setup Adjacency (Linear Graph: 1 <-> 2 <-> 3 ...)
    for (int i = 0; i < totalZones; i++)
    {
        if (i > 0) zones[i]->addNeighbor(zones[i - 1]); // Previous
        if (i < totalZones - 1) zones[i]->addNeighbor(zones[i + 1]); // Next
    }

    exportToHTML();
}

ParkingSystem::~ParkingSystem() {}

bool ParkingSystem::parkVehicle(ParkingRequest *req)
{
    // New Request State
    req->setState(REQUESTED);

    ParkingSlot *slot = engine->assignSlot(req->vehicle, zones, totalZones);

    if (slot)
    {
        if (slot->zoneNum != req->vehicle->preferredZoneID)
        {
            req->penaltyCost = 50.0;
        }

        // Strict State: REQUESTED -> ALLOCATED -> OCCUPIED
        req->setState(ALLOCATED);
        req->setState(OCCUPIED);
        
        req->startTime = this->globalTime;

        slot->occupy(req->vehicle->vehId, req);
        rbManager->pushOperation(PARK_ACTION, req, slot);
        
        // Analytics: Record Usage of the ACTUAL zone where it parked
        history->recordUsage(slot->zoneNum); 

        this->globalTime++;
        exportToHTML();
        return true;
    }

    req->setState(CANCELLED);
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
                            // STATE: OCCUPIED -> RELEASED
                            finishedReq->setState(RELEASED);
                            history->addRecord(finishedReq);
                            history->recordCompletion(); // Track Completion
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
        // Undo Park: Free slot, Cancel Request
        action->slot->free();
        
        bool success = action->request->setState(CANCELLED); 
        if(!success) {
             cout << "FORCE CHANGING STATE DUE TO ROLLBACK" << endl;
             action->request->status = CANCELLED;
        }
        
        // Track Cancellation
        history->recordCancellation();
    }
    exportToHTML();
}

void ParkingSystem::rollback(int k) {
    cout << "ROLLING BACK " << k << " STEPS..." << endl;
    for(int i=0; i<k; i++) {
        if(rbManager->isEmpty()) break;
        undoLastAction();
    }
}

void ParkingSystem::showStatus() {}

// --- NEXT-GEN UI GENERATOR ---
void ParkingSystem::exportToHTML()
{
    ofstream file("dashboard.html");

    file << "<html><head>";
    file << "<title>Lahore PARKING OS</title>";
    
    // --- ANIMATED CSS ---
    file << "<style>";
    file << "@import url('https://fonts.googleapis.com/css2?family=Rajdhani:wght@500;700&display=swap');";
    file << "body { background: #0b0c15; color: #e0e0e0; font-family: 'Rajdhani', sans-serif; text-align: center; margin: 0; padding: 20px; overflow-x: hidden; }";
    
    // Header Animation
    file << "h1 { color: #00ff9d; text-transform: uppercase; letter-spacing: 5px; margin-bottom: 40px; animation: glow 2s ease-in-out infinite alternate; }";
    file << "@keyframes glow { from { text-shadow: 0 0 10px #00ff9d; } to { text-shadow: 0 0 20px #00ff9d, 0 0 30px #00ff9d; } }";

    // Control Panel (Glass)
    file << ".control-panel { background: rgba(255, 255, 255, 0.03); backdrop-filter: blur(10px); border: 1px solid rgba(255, 255, 255, 0.1); border-radius: 16px; padding: 25px; max-width: 700px; margin: 0 auto 50px auto; box-shadow: 0 10px 30px rgba(0,0,0,0.5); transition: 0.3s; }";
    file << ".control-panel:hover { box-shadow: 0 0 30px rgba(0, 255, 157, 0.1); border-color: rgba(0, 255, 157, 0.3); }";

    // Inputs & Buttons
    file << "input { background: rgba(0,0,0,0.3); border: 1px solid #333; color: white; padding: 12px; margin: 5px; border-radius: 8px; font-family: inherit; font-size: 1.1em; outline: none; transition: 0.3s; }";
    file << "input:focus { border-color: #00ff9d; box-shadow: 0 0 15px rgba(0, 255, 157, 0.2); transform: scale(1.05); }";
    
    file << "button { background: linear-gradient(45deg, #00ff9d, #00cc7a); color: #0b0c15; border: none; padding: 12px 30px; font-weight: 800; cursor: pointer; margin: 5px; border-radius: 8px; text-transform: uppercase; letter-spacing: 1px; transition: 0.3s; position: relative; overflow: hidden; }";
    file << "button:hover { transform: translateY(-3px); box-shadow: 0 0 20px rgba(0, 255, 157, 0.6); }";
    file << ".btn-red { background: linear-gradient(45deg, #ff3e3e, #d60000); color: white; }";
    file << ".btn-red:hover { box-shadow: 0 0 20px rgba(255, 62, 62, 0.6); }";
    file << ".btn-undo { background: linear-gradient(45deg, #f0ad4e, #ec971f); color: white; }";

    // Grid Layout
    file << ".zones-container { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 30px; padding: 0 50px; }";
    
    // Zone Cards (Slide In Animation)
    file << ".zone-card { background: #13141f; border-top: 4px solid #333; border-radius: 12px; padding: 20px; box-shadow: 0 10px 20px rgba(0,0,0,0.3); transition: 0.4s; animation: slideUp 0.8s ease-out; }";
    file << "@keyframes slideUp { from { opacity: 0; transform: translateY(50px); } to { opacity: 1; transform: translateY(0); } }";
    file << ".zone-card:hover { transform: translateY(-10px) scale(1.02); border-color: #00ff9d; box-shadow: 0 0 30px rgba(0, 255, 157, 0.2); }";
    
    file << ".zone-title { font-size: 1.8em; color: #fff; margin-bottom: 20px; font-weight: 700; border-bottom: 1px solid #2a2b3d; padding-bottom: 10px; }";
    
    // Slots (Pulse Animation for Occupied)
    file << ".slot { display: flex; justify-content: space-between; align-items: center; padding: 12px; margin: 8px 0; border-radius: 8px; background: #1a1b26; border: 1px solid #2a2b3d; font-weight: 600; transition: 0.3s; }";
    
    file << ".free { border-left: 4px solid #00ff9d; color: #a0a0a0; }";
    file << ".free:hover { background: rgba(0, 255, 157, 0.1); cursor: pointer; }";

    file << ".occ { border-left: 4px solid #ff3e3e; background: rgba(255, 62, 62, 0.1); color: #fff; animation: pulseRed 2s infinite; }";
    file << "@keyframes pulseRed { 0% { box-shadow: 0 0 0 rgba(255, 62, 62, 0); } 50% { box-shadow: 0 0 15px rgba(255, 62, 62, 0.3); } 100% { box-shadow: 0 0 0 rgba(255, 62, 62, 0); } }";

    file << ".penalty { background: #ff3e3e; color: white; font-size: 0.7em; padding: 2px 6px; border-radius: 4px; margin-left: 10px; animation: flash 1s infinite; }";
    file << "@keyframes flash { 0%, 100% { opacity: 1; } 50% { opacity: 0.5; } }";

    // Stats
    file << ".stats { display: flex; justify-content: center; gap: 40px; margin-top: 60px; padding: 30px; background: #0f101a; border-top: 1px solid #2a2b3d; }";
    file << ".stat-item { text-align: center; transition: 0.3s; }";
    file << ".stat-item:hover { transform: scale(1.1); }";
    file << ".stat-val { font-size: 3em; font-weight: 800; color: #fff; line-height: 1; }";
    file << ".stat-label { font-size: 0.9em; color: #888; letter-spacing: 2px; margin-top: 10px; }";
    file << "</style>";

    // --- JAVASCRIPT ---
    file << "<script>";
    file << "function sendCmd(type) {";
    file << "  if(type === 'TEST') {";
    file << "    const blob = new Blob(['TEST'], { type: 'text/plain' });";
    file << "    const a = document.createElement('a'); a.href = URL.createObjectURL(blob);";
    file << "    a.download = 'command.txt'; document.body.appendChild(a); a.click(); document.body.removeChild(a);";
    file << "    return;";
    file << "  }";
    file << "  let content = '';";
    file << "  if(type === 'PARK') {";
    file << "    let v = document.getElementById('vId').value; let z = document.getElementById('zId').value;";
    file << "    if(!v || !z) { alert('Enter Details'); return; }";
    file << "    content = 'PARK ' + v + ' ' + z;";
    file << "  } else if(type === 'REMOVE') {";
    file << "    let z = document.getElementById('remZ').value; let s = document.getElementById('remS').value;";
    file << "    if(!z || !s) { alert('Enter Details'); return; }";
    file << "    content = 'REMOVE ' + z + ' ' + s;";
    file << "  } else if(type === 'UNDO') { content = 'UNDO'; }";
    file << "  const blob = new Blob([content], { type: 'text/plain' });";
    file << "  const a = document.createElement('a'); a.href = URL.createObjectURL(blob);";
    file << "  a.download = 'command.txt'; document.body.appendChild(a); a.click(); document.body.removeChild(a);";
    file << "}";
    file << "</script>";

    file << "</head><body>";
    file << "<h1>SYSTEM ONLINE [" << globalTime << "]</h1>";

    // INPUT FORM
    file << "<div class='control-panel'>";
    file << "<div style='margin-top:10px;'><button style='width: 100%; background: linear-gradient(90deg, #444, #666); color: white; padding: 12px; border: 1px solid #777; font-weight: bold; cursor: pointer;' onclick=\"sendCmd('TEST')\">RUN DIAGNOSTICS (TEST SUITE)</button></div>";
    file << "<div><input type='text' id='vId' placeholder='Vehicle ID'> <input type='number' id='zId' placeholder='Zone (1-3)' style='width: 80px;'> <button onclick=\"sendCmd('PARK')\">PARK</button></div>";
    file << "<div style='margin-top:10px;'><input type='number' id='remZ' placeholder='Zone' style='width: 70px;'> <input type='number' id='remS' placeholder='Slot' style='width: 70px;'> <button class='btn-red' onclick=\"sendCmd('REMOVE')\">REMOVE</button></div>";
    file << "<div style='margin-top:10px;'><button class='btn-undo' onclick=\"sendCmd('UNDO')\">UNDO LAST ACTION</button></div>";
    file << "<p style='font-size: 0.8em; color: #888; margin-top:15px;'>*Browser will download command.txt. Save to project folder.</p>";
    file << "</div>";

    // ZONES DISPLAY
    file << "<div class='zones-container'>";
    for (int i = 0; i < totalZones; i++)
    {
        file << "<div class='zone-card'>";
        file << "<div class='zone-title'>ZONE " << zones[i]->zoneID << "</div>";
        for (int a = 0; a < zones[i]->CurCount; a++)
        {
            for (int s = 0; s < zones[i]->areas[a]->currentCount; s++)
            {
                ParkingSlot *slot = zones[i]->areas[a]->slots[s];
                if (slot->isOccupied)
                {
                    file << "<div class='slot occ'><span>" << slot->vehId << "</span>";
                    if (slot->currentReq && slot->currentReq->penaltyCost > 0)
                        file << "<span class='penalty'>PENALTY</span>";
                    file << "</div>";
                }
                else
                {
                    file << "<div class='slot free'>SLOT " << slot->slotNum << "</div>";
                }
            }
        }
        file << "</div>";
    }
    file << "</div>";

    // ANALYTICS
    file << "<div class='stats'>";
    file << "<div class='stat-item'><div class='stat-val'>" << history->completedCount << "</div><div class='stat-label'>COMPLETED</div></div>";
    file << "<div class='stat-item'><div class='stat-val'>" << history->cancelledCount << "</div><div class='stat-label'>CANCELLED</div></div>";
    file << "<div class='stat-item'><div class='stat-val'>ZONE " << history->getPeakZone() << "</div><div class='stat-label'>PEAK ZONE</div></div>";
    file << "<div class='stat-item'><div class='stat-val'>$" << (int)history->getTotalRevenue() << "</div><div class='stat-label'>REVENUE</div></div>";
    file << "</div>";

    file << "</body></html>";
    file.close();
}