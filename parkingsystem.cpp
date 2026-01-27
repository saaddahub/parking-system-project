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
}

ParkingSystem::~ParkingSystem()
{
    for (int i = 0; i < totalZones; ++i)
    {
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
}

void ParkingSystem::showStatus()
{
    // Console debug optional
}

// --- THE CYBERPUNK SERVER UI ---
string ParkingSystem::getHTML()
{
    stringstream ss;
    ss << "<!DOCTYPE html><html><head>";
    ss << "<title>NEON PARKING SERVER</title>";

    // --- MODERN GLASS CSS ---
    ss << "<style>";
    ss << "@import url('https://fonts.googleapis.com/css2?family=Rajdhani:wght@500;700&display=swap');";
    ss << "body { background: #0b0c15; color: #e0e0e0; font-family: 'Rajdhani', sans-serif; text-align: center; margin: 0; padding: 20px; }";
    ss << "h1 { color: #00ff9d; text-transform: uppercase; letter-spacing: 5px; text-shadow: 0 0 20px rgba(0, 255, 157, 0.6); margin-bottom: 40px; }";

    // Control Panel
    ss << ".control-panel { background: rgba(255, 255, 255, 0.03); backdrop-filter: blur(10px); border: 1px solid rgba(255, 255, 255, 0.1); border-radius: 16px; padding: 25px; max-width: 700px; margin: 0 auto 50px auto; box-shadow: 0 10px 30px rgba(0,0,0,0.5); }";
    ss << "input { background: rgba(0,0,0,0.3); border: 1px solid #333; color: white; padding: 12px; margin: 5px; border-radius: 8px; font-family: inherit; font-size: 1.1em; outline: none; transition: 0.3s; }";
    ss << "input:focus { border-color: #00ff9d; box-shadow: 0 0 10px rgba(0, 255, 157, 0.2); }";

    // Buttons
    ss << "button { background: linear-gradient(45deg, #00ff9d, #00cc7a); color: #0b0c15; border: none; padding: 12px 30px; font-weight: 800; cursor: pointer; margin: 5px; border-radius: 8px; text-transform: uppercase; letter-spacing: 1px; transition: 0.3s; }";
    ss << "button:hover { transform: translateY(-2px); box-shadow: 0 5px 15px rgba(0, 255, 157, 0.4); }";
    ss << ".btn-red { background: linear-gradient(45deg, #ff3e3e, #d60000); color: white; }";
    ss << ".btn-red:hover { box-shadow: 0 5px 15px rgba(255, 62, 62, 0.4); }";
    ss << ".btn-undo { background: linear-gradient(45deg, #f0ad4e, #ec971f); color: white; }";

    // Zones Grid
    ss << ".zones-container { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 30px; padding: 0 50px; }";
    ss << ".zone-card { background: #13141f; border-top: 4px solid #333; border-radius: 12px; padding: 20px; box-shadow: 0 10px 20px rgba(0,0,0,0.3); transition: 0.3s; }";
    ss << ".zone-card:hover { transform: translateY(-5px); border-color: #00ff9d; }";
    ss << ".zone-title { font-size: 1.8em; color: #fff; margin-bottom: 20px; font-weight: 700; border-bottom: 1px solid #2a2b3d; padding-bottom: 10px; }";

    // Slots
    ss << ".slot { display: flex; justify-content: space-between; align-items: center; padding: 12px; margin: 8px 0; border-radius: 8px; background: #1a1b26; border: 1px solid #2a2b3d; font-weight: 600; }";
    ss << ".free { border-left: 4px solid #00ff9d; color: #a0a0a0; }";
    ss << ".occ { border-left: 4px solid #ff3e3e; background: rgba(255, 62, 62, 0.05); color: #fff; }";
    ss << ".penalty { background: #ff3e3e; color: white; font-size: 0.7em; padding: 2px 6px; border-radius: 4px; margin-left: 10px; }";

    // Stats
    ss << ".stats { display: flex; justify-content: center; gap: 40px; margin-top: 60px; padding: 30px; background: #0f101a; border-top: 1px solid #2a2b3d; }";
    ss << ".stat-item { text-align: center; }";
    ss << ".stat-val { font-size: 3em; font-weight: 800; color: #fff; line-height: 1; }";
    ss << ".stat-label { font-size: 0.9em; color: #888; letter-spacing: 2px; margin-top: 10px; }";
    ss << "</style>";

    // --- JAVASCRIPT (Wired to Server Routes) ---
    ss << "<script>";

    ss << "function park() {";
    ss << "  let v = document.getElementById('vId').value;";
    ss << "  let z = document.getElementById('zId').value;";
    ss << "  if(!v || !z) { alert('Enter Details'); return; }";
    ss << "  window.location.href = '/park?v=' + v + '&z=' + z;";
    ss << "}";

    ss << "function remove() {";
    ss << "  let z = document.getElementById('remZ').value;";
    ss << "  let s = document.getElementById('remS').value;";
    ss << "  if(!z || !s) { alert('Enter Details'); return; }";
    ss << "  window.location.href = '/remove?z=' + z + '&s=' + s;";
    ss << "}";

    ss << "function undo() {";
    ss << "  window.location.href = '/undo';";
    ss << "}";

    ss << "</script></head><body>";

    ss << "<h1>SYSTEM ONLINE [" << globalTime << "]</h1>";

    // CONTROLS
    ss << "<div class='control-panel'>";
    ss << "<div><input type='text' id='vId' placeholder='Vehicle ID'> <input type='number' id='zId' placeholder='Zone (1-3)' style='width: 80px;'> <button onclick='park()'>PARK</button></div>";
    ss << "<div style='margin-top:10px;'><input type='number' id='remZ' placeholder='Zone' style='width: 70px;'> <input type='number' id='remS' placeholder='Slot' style='width: 70px;'> <button class='btn-red' onclick='remove()'>REMOVE</button></div>";
    ss << "<div style='margin-top:10px;'><button class='btn-undo' onclick='undo()'>UNDO LAST ACTION</button></div>";
    ss << "</div>";

    // ZONES
    ss << "<div class='zones-container'>";
    for (int i = 0; i < totalZones; i++)
    {
        ss << "<div class='zone-card'>";
        ss << "<div class='zone-title'>ZONE " << zones[i]->zoneID << "</div>";
        for (int a = 0; a < zones[i]->CurCount; a++)
        {
            for (int s = 0; s < zones[i]->areas[a]->currentCount; s++)
            {
                ParkingSlot *slot = zones[i]->areas[a]->slots[s];
                if (slot->isOccupied)
                {
                    ss << "<div class='slot occ'><span>" << slot->vehId << "</span>";
                    if (slot->currentReq && slot->currentReq->penaltyCost > 0)
                        ss << "<span class='penalty'>PENALTY</span>";
                    ss << "</div>";
                }
                else
                {
                    ss << "<div class='slot free'>SLOT " << slot->slotNum << "</div>";
                }
            }
        }
        ss << "</div>";
    }
    ss << "</div>";

    // STATS
    ss << "<div class='stats'>";
    ss << "<div class='stat-item'><div class='stat-val'>" << history->count << "</div><div class='stat-label'>TOTAL TRIPS</div></div>";
    ss << "<div class='stat-item'><div class='stat-val'>" << fixed << setprecision(1) << history->getAverageDuration() << "</div><div class='stat-label'>AVG DURATION</div></div>";
    ss << "<div class='stat-item'><div class='stat-val'>$" << (int)history->getTotalRevenue() << "</div><div class='stat-label'>REVENUE</div></div>";
    ss << "</div>";

    ss << "</body></html>";
    return ss.str();
}