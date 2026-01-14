#include "parkingsystem.h"
#include <iostream>
using namespace std;

ParkingSystem::ParkingSystem(int numZones, int slotsPerZone)
{
    this->totalZones = numZones;
    this->engine = new AllocationEngine();

    this->zones = new zone *[totalZones];

    for (int i = 0; i < totalZones; i++)
    {
        this->zones[i] = new zone(i + 1, slotsPerZone);

        for (int s = 0; s < slotsPerZone; s++)
        {
            this->zones[i]->SlotAddition(s + 1);
        }
    }
}

ParkingSystem::~ParkingSystem()
{
    for (int i = 0; i < totalZones; i++)
    {
        delete zones[i];
    }
    delete[] zones;
    delete engine;
}

bool ParkingSystem::parkVehicle(vehicle *v)
{
    ParkingSlot *assignedSlot = engine->assignSlot(v, zones, totalZones);

    if (assignedSlot != nullptr)
    {
        cout << "Success: Vehicle " << v->vehId << " parked in Zone "
             << assignedSlot->zoneID << ", Slot " << assignedSlot->SlotID << endl;
        return true;
    }
    else
    {
        cout << "Failure: No parking space available for " << v->vehId << endl;
        return false;
    }
}

bool ParkingSystem::removeVehicle(int zID, int sID)
{
    for (int i = 0; i < totalZones; i++)
    {
        if (zones[i]->ZoneID == zID)
        {

            for (int j = 0; j < zones[i]->CurCount; j++)
            {
                if (zones[i]->slots[j]->SlotID == sID)
                {

                    if (zones[i]->slots[j]->isOccupied)
                    {
                        cout << "Vehicle " << zones[i]->slots[j]->OccByVehicleID << " left the parking." << endl;
                        zones[i]->slots[j]->free();
                        return true;
                    }
                    else
                    {
                        cout << "Error: Slot is already empty." << endl;
                        return false;
                    }
                }
            }
        }
    }
    cout << "Error: Zone or Slot not found." << endl;
    return false;
}

void ParkingSystem::showStatus()
{
    cout << "\n--- CURRENT PARKING STATUS ---" << endl;
    for (int i = 0; i < totalZones; i++)
    {
        cout << "Zone " << zones[i]->ZoneID << ": ";
        for (int j = 0; j < zones[i]->CurCount; j++)
        {
            if (zones[i]->slots[j]->isOccupied)
            {
                cout << "[ " << zones[i]->slots[j]->OccByVehicleID << " ] ";
            }
            else
            {
                cout << "[ Free ] ";
            }
        }
        cout << endl;
    }
    cout << "------------------------------\n"
         << endl;
}