#include "parkingsystem.h"
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;

ParkingSystem::ParkingSystem(int numSites, int zonesPerSite)
{
    this->numSites = numSites; // e.g. 4
    this->zonesPerSite = zonesPerSite; // e.g. 3
    this->totalZones = numSites * zonesPerSite; // 12
    
    this->siteNames = new string[numSites];
    // Default Names
    if(numSites >= 4) {
        siteNames[0] = "AIRPORT TERMINAL";
        siteNames[1] = "CENTRAL PARK";
        siteNames[2] = "LAHORE MALL";
        siteNames[3] = "DATA DARBAR";
    }

    this->globalTime = 0;
    this->engine = new AllocationEngine();
    this->rbManager = new RollbackManager();
    this->history = new HistoryManager(totalZones); 
    this->zones = new Zone *[totalZones];

    // 1. Create Zones (FLAT ARRAY 0..11)
    for (int i = 0; i < totalZones; i++)
    {
        // Zone ID is strictly 1..3 per site or 1..12 total?
        // User said "Areas will then be divided into 3 zones".
        // Let's keep Global ID 1..12 for internal logic, but display relative in UI.
        this->zones[i] = new Zone(i + 1, 2);
        this->zones[i]->addArea(1, 2); // 2 Slots per area
        this->zones[i]->addArea(2, 2); // 2 Slots per area
        
        for (int a = 0; a < this->zones[i]->CurCount; a++)
        {
            for (int s = 0; s < 2; s++) // hardcode 2 slots for now
            {
                this->zones[i]->areas[a]->addSlot(s + 1);
            }
        }
    }

    // 2. Setup Adjacency (Linear Graph: 1 <-> 2 <-> 3 ...)
    // Only link zones WITHIN the same site? Or facilitate cross-site?
    // User implied specific areas. Usually parking doesn't overflow from Airport to Mall (miles away).
    // So distinct graphs.
    for (int s = 0; s < numSites; s++) {
        int start = s * zonesPerSite;
        int end = start + zonesPerSite;
        for (int i = start; i < end; i++) {
            if (i > start) zones[i]->addNeighbor(zones[i - 1]);
            if (i < end - 1) zones[i]->addNeighbor(zones[i + 1]);
        }
    }

    exportToHTML();
}

ParkingSystem::~ParkingSystem() {
    delete history;
    delete rbManager;
    delete engine;
    for(int i=0; i<totalZones; i++) {
        delete zones[i]; 
    }
    delete[] zones;
    delete[] siteNames;
}

bool ParkingSystem::parkVehicle(ParkingRequest *req)
{
    // New Request State
    req->setState(REQUESTED);

    // Constraint Check: DUPLICATE CAR
    // Scan all slots? O(N). N is small.
    for(int i=0; i<totalZones; i++) {
        for(int a=0; a<zones[i]->CurCount; a++) {
             for(int s=0; s<zones[i]->areas[a]->currentCount; s++) {
                 if(zones[i]->areas[a]->slots[s]->isOccupied && 
                    zones[i]->areas[a]->slots[s]->vehId == req->vehicle->vehId) 
                 {
                     // Found duplicate
                     req->setState(CANCELLED);
                     if(req->vehicle) delete req->vehicle;
                     delete req;
                     return false; // REJECT
                 }
             }
        }
    }

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
    // MEMORY FIX: System refused it. Delete it.
    if(req->vehicle) delete req->vehicle;
    delete req;
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

        // MEMORY FIX: The request is now dead. It's not in a slot, and not in history (only count incremented).
        // We MUST delete it to avoid leak.
        if(action->request) {
            if(action->request->vehicle) delete action->request->vehicle;
            delete action->request; 
        }
        
        // The 'action' node itself is deleted by the caller (popOperation doesn't delete, caller does).
        // Wait, popOperation returns a pointer to a node that was detached from the list.
        // We are holding 'action'. We need to delete it.
        delete action;
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
    file << "<title>Lahore Parker</title>";
    
    // --- MODERN FLAT CSS ---
    file << "<style>";
    file << "@import url('https://fonts.googleapis.com/css2?family=Nunito:wght@400;700;800&display=swap');";
    
    // ANIMATED BACKGROUND
    file << "body { background: linear-gradient(-45deg, #f0f2f5, #e0eafc, #f0f2f5); background-size: 400% 400%; animation: bgFlow 10s ease infinite; color: #2d3436; font-family: 'Nunito', sans-serif; text-align: center; margin: 0; padding: 20px; overflow-x: hidden; }";
    file << "@keyframes bgFlow { 0% { background-position: 0% 50%; } 50% { background-position: 100% 50%; } 100% { background-position: 0% 50%; } }";
    
    // Header
    file << "h1 { color: #2d3436; font-weight: 800; font-size: 2.5em; margin-bottom: 10px; letter-spacing: -1px; position: relative; z-index: 10;}";
    file << "h1 span { color: #0984e3; }";

    // Dashboard Container
    file << ".dashboard { display: flex; flex-wrap: wrap; justify-content: center; gap: 40px; margin-top: 30px; position: relative; z-index: 10; }";

    // CAR ANIMATION ELEMENT
    file << "#anim-car { position: fixed; bottom: 20px; left: -200px; font-size: 100px; z-index: 5; opacity: 0; pointer-events: none; }";
    
    file << "@keyframes driveIn { 0% { left: -200px; opacity: 1; transform: scaleX(1); } 60% { left: 80%; opacity: 1; transform: scaleX(1); } 100% { left: 85%; opacity: 0; transform: scaleX(1); } }";
    
    file << "@keyframes driveOut { 0% { left: 85%; opacity: 1; transform: scaleX(-1); } 100% { left: -200px; opacity: 1; transform: scaleX(-1); } }";

    // Control Panel
    file << ".control-panel { background: #ffffff; border-radius: 20px; padding: 30px; min-width: 320px; max-width: 380px; box-shadow: 0 10px 25px rgba(0,0,0,0.05); height: fit-content; text-align: left; transition: transform 0.3s ease; }";
    file << ".control-panel:hover { transform: translateY(-5px); box-shadow: 0 15px 35px rgba(0,0,0,0.1); }";
    file << ".control-panel h3 { margin-top: 0; color: #636e72; font-size: 0.9em; text-transform: uppercase; letter-spacing: 1px; border-bottom: 2px solid #dfe6e9; padding-bottom: 10px; margin-bottom: 20px; }";

    // Inputs & Buttons
    file << "label { font-size: 0.85em; font-weight: 700; color: #b2bec3; display: block; margin-bottom: 5px; }";
    file << "select, input { width: 100%; background: #f1f2f6; border: 2px solid #dfe6e9; color: #2d3436; padding: 12px; margin-bottom: 15px; border-radius: 12px; font-family: inherit; font-size: 1em; outline: none; transition: 0.2s; box-sizing: border-box; }";
    file << "input:focus, select:focus { border-color: #0984e3; background: #fff; }";
    
    file << "button { width: 100%; background: #0984e3; color: white; border: none; padding: 14px; font-weight: 800; cursor: pointer; border-radius: 12px; font-size: 1em; transition: 0.2s; box-shadow: 0 4px 6px rgba(9, 132, 227, 0.2); }";
    file << "button:hover { background: #74b9ff; transform: translateY(-2px); box-shadow: 0 6px 12px rgba(9, 132, 227, 0.3); }";
    
    file << ".btn-red { background: #d63031; box-shadow: 0 4px 6px rgba(214, 48, 49, 0.2); }";
    file << ".btn-red:hover { background: #ff7675; box-shadow: 0 6px 12px rgba(214, 48, 49, 0.3); }";
    
    file << ".btn-undo { background: #fdcb6e; color: #2d3436; box-shadow: 0 4px 6px rgba(253, 203, 110, 0.2); margin-top: 10px; }";
    file << ".btn-undo:hover { background: #ffeaa7; }";

    // Site Containers
    file << ".sites-wrapper { flex: 1; display: grid; grid-template-columns: repeat(auto-fit, minmax(320px, 1fr)); gap: 25px; max-width: 1100px; }";
    file << ".site-group { background: #ffffff; border-radius: 16px; padding: 20px; box-shadow: 0 4px 15px rgba(0,0,0,0.03); border: 1px solid #dfe6e9; animation: popIn 0.5s ease-out; }";
    file << "@keyframes popIn { from { opacity: 0; transform: scale(0.95); } to { opacity: 1; transform: scale(1); } }";
    
    file << ".site-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 20px; padding-bottom: 10px; border-bottom: 2px solid #f1f2f6; }";
    file << ".site-name { font-weight: 800; font-size: 1.2em; color: #2d3436; }";
    file << ".site-badge { background: #cef4da; color: #00b894; padding: 4px 10px; border-radius: 20px; font-size: 0.7em; font-weight: 800; }";

    file << ".zone-row { display: flex; gap: 15px; }";
    file << ".zone-box { flex: 1; background: #f8f9fa; padding: 10px; border-radius: 12px; border: 1px solid #e9ecef; }";
    file << ".zone-name { font-size: 0.8em; color: #b2bec3; font-weight: 800; margin-bottom: 10px; text-transform: uppercase; }";

    file << ".slot { padding: 10px; margin: 6px 0; border-radius: 8px; font-size: 0.9em; font-weight: 800; transition: 0.2s; display: flex; justify-content: center; align-items: center; position: relative; }";
    file << ".free { background: #ffffff; border: 2px dashed #dfe6e9; color: #b2bec3; }";
    file << ".free:hover { border-color: #0984e3; color: #0984e3; cursor: pointer; }";
    file << ".occ { background: #fab1a0; color: #d63031; border: 2px solid #ff7675; animation: bounceIn 0.4s; }";
    file << "@keyframes bounceIn { 0% { transform: scale(0.3); opacity: 0; } 50% { transform: scale(1.05); } 100% { transform: scale(1); opacity: 1; } }";

    // Stats
    file << ".stats { background: #ffffff; width: 90%; max-width: 1000px; margin: 50px auto; padding: 30px; border-radius: 20px; display: flex; justify-content: space-around; box-shadow: 0 10px 30px rgba(0,0,0,0.05); }";
    file << ".stat-item { text-align: center; }";
    file << ".stat-val { font-size: 2.5em; font-weight: 900; color: #2d3436; }";
    file << ".stat-label { font-size: 0.8em; color: #b2bec3; font-weight: 700; letter-spacing: 1px; text-transform: uppercase; margin-top: 5px; }";
    file << "</style>";

    // --- JAVASCRIPT ---
    file << "<script>";
    file << "function anim(type, callback) {";
    file << "  let car = document.getElementById('anim-car');";
    file << "  // Reset animation";
    file << "  car.style.animation = 'none';";
    file << "  car.offsetHeight; /* trigger reflow */";
    file << "  if(type === 'IN') {";
    file << "      car.style.animation = 'driveIn 2s ease-out forwards';";
    file << "  } else if(type === 'OUT') {";
    file << "      car.style.animation = 'driveOut 2s ease-in forwards';";
    file << "  }";
    file << "  setTimeout(callback, 1500);"; // Wait for anim
    file << "}";

    file << "function sendCmd(type) {";
    file << "  let url = '';";
    file << "  let isAnim = false; let animType = '';";
    file << "  if(type === 'TEST') { url = '/api/test'; }";
    file << "  else if(type === 'PARK') {";
    file << "    let siteIdx = document.getElementById('siteIdx').value;";
    file << "    let zoneRel = document.getElementById('zoneRel').value;";
    file << "    let v = document.getElementById('vId').value;";
    file << "    if(!v) { alert('Whoops! Enter Vehicle ID please.'); return; }";
    file << "    let z = (parseInt(siteIdx) * " << zonesPerSite << ") + parseInt(zoneRel);";
    file << "    url = '/api/park?v=' + encodeURIComponent(v) + '&z=' + z;";
    file << "    isAnim = true; animType = 'IN';";
    file << "  } else if(type === 'REMOVE') {";
    file << "    let siteIdx = document.getElementById('remSite').value;";
    file << "    let zoneRel = document.getElementById('remZone').value;";
    file << "    let s = document.getElementById('remSlot').value;";
    file << "    let z = (parseInt(siteIdx) * " << zonesPerSite << ") + parseInt(zoneRel);";
    file << "    url = '/api/remove?z=' + z + '&s=' + s;";
    file << "    isAnim = true; animType = 'OUT';";
    file << "  } else if(type === 'UNDO') { url = '/api/undo'; }";
    
    file << "  let exec = () => fetch(url, { method: 'POST' }).then(r => {";
    file << "    if(r.ok) location.reload();";
    file << "    else return r.text().then(t => alert('Oh snap! ' + t));";
    file << "  }).catch(e => alert('Connection Lost :('));";
    
    file << "  if(isAnim) anim(animType, exec); else exec();";
    file << "}";
    file << "</script>";

    file << "</head><body>";
    file << "<div id='anim-car'>🚗</div>"; // The Car Emoji
    file << "<h1>Lahore <span>Parker</span></h1>";
    file << "<p style='color: #636e72; margin-top:-10px; margin-bottom: 30px;'>Smart Parking System • Ver " << globalTime << "</p>";
    
    file << "<div class='dashboard'>";
    // ... (Rest of HTML is same)
    
    // LEFT: CONTROL PANEL
    file << "<div class='control-panel'>";
    file << "<h3>Operator Controls</h3>";
    
    // Park Form
    file << "<div style='margin-bottom: 25px;'>";
    file << "<label>NEW ARRIVAL</label>";
    file << "<input type='text' id='vId' placeholder='Plate #'>";
    file << "<div style='display:flex; gap:10px;'>";
    file << "<select id='siteIdx'>";
    for(int i=0; i<numSites; i++) file << "<option value='" << i << "'>" << siteNames[i] << "</option>";
    file << "</select>";
    file << "<select id='zoneRel'>";
    for(int i=1; i<=zonesPerSite; i++) file << "<option value='" << i << "'>Zone " << i << "</option>";
    file << "</select>";
    file << "</div>";
    file << "<button onclick=\"sendCmd('PARK')\">Park Vehicle</button>";
    file << "</div>";

    // Remove Form
    file << "<div style='margin-bottom: 25px;'>";
    file << "<label>DEPARTURE</label>";
    file << "<div style='display:flex; gap:10px;'>";
    file << "<select id='remSite' style='flex:2;'>";
    for(int i=0; i<numSites; i++) file << "<option value='" << i << "'>" << siteNames[i] << "</option>";
    file << "</select>";
    file << "<select id='remZone' style='flex:1;'>";
    for(int i=1; i<=zonesPerSite; i++) file << "<option value='" << i << "'>Z" << i << "</option>";
    file << "</select>";
    file << "<input type='number' id='remSlot' placeholder='#' style='flex:1;'>";
    file << "</div>";
    file << "<button class='btn-red' onclick=\"sendCmd('REMOVE')\">Release Vehicle</button>";
    file << "</div>";

    // Undo
    file << "<button class='btn-undo' onclick=\"sendCmd('UNDO')\">Undo Mistake</button>";

    // Diagnostics
    file << "<button class='btn-diag' onclick=\"sendCmd('TEST')\">Run Diagnostics</button>";
    file << "</div>"; 

    // RIGHT: SITES DISPLAY
    file << "<div class='sites-wrapper'>";
    
    for (int s = 0; s < numSites; s++)
    {
        file << "<div class='site-group'>";
        file << "<div class='site-header'><span class='site-name'>" << siteNames[s] << "</span> <span class='site-badge'>OPEN</span></div>";
        file << "<div class='zone-row'>";
        
        // Loop Zones for this Site
        int startZone = s * zonesPerSite;
        int endZone = startZone + zonesPerSite;
        
        for (int i = startZone; i < endZone; i++) {
            file << "<div class='zone-box'>";
            file << "<div class='zone-name'>Z" << (i - startZone + 1) << "</div>";
            
            // Slots
            for (int a = 0; a < zones[i]->CurCount; a++) {
                for (int slotIdx = 0; slotIdx < zones[i]->areas[a]->currentCount; slotIdx++) {
                    ParkingSlot *slot = zones[i]->areas[a]->slots[slotIdx];
                    if (slot->isOccupied)
                        file << "<div class='slot occ'><span>" << slot->vehId << "</span></div>";
                    else
                        file << "<div class='slot free'><span>" << slot->slotNum << "</span></div>";
                }
            }
            file << "</div>"; // End Zone Box
        }
        file << "</div>"; // End Zone Row
        file << "</div>"; // End Site Group
    }
    
    file << "</div>"; // End Sites Wrapper
    file << "</div>"; // End Dashboard

    // FOOTER STATS
    file << "<div class='stats'>";
    file << "<div class='stat-item'><div class='stat-val'>" << history->completedCount << "</div><div class='stat-label'>Parked</div></div>";
    file << "<div class='stat-item'><div class='stat-val'>" << history->cancelledCount << "</div><div class='stat-label'>Canceled</div></div>";
    file << "<div class='stat-item'><div class='stat-val'>#" << history->getPeakZone() << "</div><div class='stat-label'>Busy Zone</div></div>";
    file << "<div class='stat-item'><div class='stat-val'>$" << (int)history->getTotalRevenue() << "</div><div class='stat-label'>Revenue</div></div>";
    file << "</div>";

    file << "</body></html>";
    file.close();
}