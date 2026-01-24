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

ParkingSystem::~ParkingSystem() {}

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

void ParkingSystem::showStatus() {}

// --- THE CYBERPUNK UI GENERATOR ---
void ParkingSystem::exportToHTML()
{
    ofstream file("dashboard.html");

    file << "<html><head><meta http-equiv='refresh' content='3'>";
    file << "<title>NEON PARKING OS</title>";
    file << "<style>";
    file << "body { background-color: #050510; color: #00ffcc; font-family: 'Courier New', monospace; text-align: center; margin: 0; padding: 20px; }";
    file << "h1 { text-shadow: 0 0 10px #00ffcc; margin-bottom: 30px; letter-spacing: 5px; }";

    // CONTROL PANEL
    file << ".control-panel { background: rgba(0, 255, 204, 0.05); border: 2px solid #00ffcc; border-radius: 10px; padding: 20px; max-width: 600px; margin: 0 auto 40px auto; box-shadow: 0 0 15px rgba(0,255,204,0.2); }";
    file << "input { background: #111; border: 1px solid #00ffcc; color: white; padding: 10px; margin: 5px; border-radius: 5px; font-family: inherit; }";
    file << "button { background: #00ffcc; color: black; border: none; padding: 10px 20px; font-weight: bold; cursor: pointer; margin: 5px; border-radius: 5px; transition: 0.3s; }";
    file << "button:hover { background: white; box-shadow: 0 0 15px white; }";
    file << ".btn-red { background: #ff3366; color: white; } .btn-red:hover { background: #ff6699; box-shadow: 0 0 15px #ff3366; }";

    // ZONES
    file << ".zones-container { display: flex; flex-wrap: wrap; justify-content: center; gap: 20px; }";
    file << ".zone-card { background: #0a0a1a; border: 1px solid #333; border-radius: 10px; padding: 15px; width: 280px; position: relative; }";
    file << ".zone-title { font-size: 1.5em; border-bottom: 1px solid #333; padding-bottom: 10px; margin-bottom: 10px; color: #fff; }";

    // SLOTS
    file << ".slot { display: flex; justify-content: space-between; padding: 8px; margin: 5px 0; border-radius: 5px; font-weight: bold; }";
    file << ".free { background: rgba(0, 255, 0, 0.1); border: 1px solid #00ff00; color: #00ff00; }";
    file << ".occ { background: rgba(255, 50, 100, 0.2); border: 1px solid #ff3366; color: #ff3366; }";
    file << ".penalty { color: yellow; font-size: 0.8em; float: right; }";

    // ANALYTICS
    file << ".stats { margin-top: 40px; padding: 20px; border-top: 1px solid #333; display: flex; justify-content: center; gap: 50px; }";
    file << ".stat-item { text-align: center; } .stat-val { font-size: 2em; color: white; }";

    file << "</style>";

    // JAVASCRIPT FOR BUTTONS
    file << "<script>";
    file << "function sendCmd(type) {";
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
    file << "<h3>OPERATIONS</h3>";
    file << "<div><input type='text' id='vId' placeholder='Vehicle ID'> <input type='number' id='zId' placeholder='Zone (1-3)' style='width: 80px;'> <button onclick=\"sendCmd('PARK')\">PARK</button></div>";
    file << "<div style='margin-top:10px;'><input type='number' id='remZ' placeholder='Zone' style='width: 70px;'> <input type='number' id='remS' placeholder='Slot' style='width: 70px;'> <button class='btn-red' onclick=\"sendCmd('REMOVE')\">REMOVE</button></div>";
    file << "<div style='margin-top:10px;'><button style='background: #ffcc00;' onclick=\"sendCmd('UNDO')\">UNDO LAST ACTION</button></div>";
    file << "<p style='font-size: 0.8em; color: #888;'>*Browser will download command.txt. Save it to project folder to execute.</p>";
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
                    // Check penalty visually
                    if (slot->currentReq && slot->currentReq->penaltyCost > 0)
                        file << "<span class='penalty'>[!] $50</span>";
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
    file << "<div class='stat-item'><div class='stat-val'>" << history->count << "</div><div>TOTAL TRIPS</div></div>";
    file << "<div class='stat-item'><div class='stat-val'>" << fixed << setprecision(1) << history->getAverageDuration() << "</div><div>AVG DURATION</div></div>";
    file << "<div class='stat-item'><div class='stat-val'>$" << (int)history->getTotalRevenue() << "</div><div>REVENUE</div></div>";
    file << "</div>";

    file << "</body></html>";
    file.close();
}