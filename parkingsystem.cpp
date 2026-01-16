#include "parkingsystem.h"
#include <iostream>
#include <fstream>
using namespace std;

ParkingSystem::ParkingSystem(int numZones, int slotsPerZone)
{
    this->totalZones = numZones;
    this->engine = new AllocationEngine();
    this->zones = new Zone *[totalZones];

    for (int i = 0; i < totalZones; i++)
    {
        this->zones[i] = new Zone(i + 1, 2); // 2 Areas per Zone
        // Add 2 Areas
        this->zones[i]->addArea(1, slotsPerZone);
        this->zones[i]->addArea(2, slotsPerZone);

        // Add slots to areas
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
    for (int i = 0; i < totalZones; i++)
        delete zones[i];
    delete[] zones;
    delete engine;
}

bool ParkingSystem::parkVehicle(Vehicle *v)
{
    ParkingSlot *slot = engine->assignSlot(v, zones, totalZones);
    if (slot)
    {
        cout << "Success: " << v->vehId << " parked in Zone " << slot->zoneNum << endl;
        exportToHTML();
        return true;
    }
    cout << "Failed to park " << v->vehId << endl;
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
                    if (zones[i]->areas[a]->slots[s]->slotNum == sID)
                    {
                        if (zones[i]->areas[a]->slots[s]->isOccupied)
                        {
                            cout << zones[i]->areas[a]->slots[s]->vehId << " left." << endl;
                            zones[i]->areas[a]->slots[s]->free();
                            exportToHTML();
                            return true;
                        }
                        else
                        {
                            cout << "Slot empty." << endl;
                            return false;
                        }
                    }
                }
            }
        }
    }
    return false;
}

void ParkingSystem::showStatus()
{
    // Console status (optional)
}

void ParkingSystem::exportToHTML()
{
    ofstream file("dashboard.html");
    file << "<html><head><meta http-equiv='refresh' content='2'>";
    file << "<style>body{background:#0d1117;color:white;font-family:monospace;} .zone{border:1px solid #333;margin:10px;padding:10px;} .occ{color:red;} .free{color:green;}</style>";
    file << "</head><body><h1>PARKING OS</h1>";
    for (int i = 0; i < totalZones; i++)
    {
        file << "<div class='zone'><h3>Zone " << zones[i]->zoneID << "</h3>";
        for (int a = 0; a < zones[i]->CurCount; a++)
        {
            file << "Area " << zones[i]->areas[a]->areaID << ": ";
            for (int s = 0; s < zones[i]->areas[a]->currentCount; s++)
            {
                if (zones[i]->areas[a]->slots[s]->isOccupied)
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