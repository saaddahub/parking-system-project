#include "allocationengine.h"
#include <iostream>
using namespace std;

ParkingSlot *AllocationEngine::assignSlot(vehicle *v, zone **zones, int numZones)
{

    // --- STRATEGY 1: Check Preferred Zone ---
    for (int i = 0; i < numZones; i++)
    {
        if (zones[i]->ZoneID == v->ZoneToGoId)
        {

            if (!zones[i]->isFull())
            {
                for (int j = 0; j < zones[i]->CurCount; j++)
                {
                    // Using your specific variable names
                    if (!zones[i]->slots[j]->isOccupied)
                    {
                        zones[i]->slots[j]->occupy(v->vehId);
                        return zones[i]->slots[j];
                    }
                }
            }
            break;
        }
    }

    // --- STRATEGY 2: Cross-Zone Allocation ---
    cout << "Preferred zone full... Attempting Cross-Zone allocation." << endl;

    for (int i = 0; i < numZones; i++)
    {
        if (zones[i]->ZoneID == v->ZoneToGoId)
            continue;

        if (!zones[i]->isFull())
        {
            for (int j = 0; j < zones[i]->CurCount; j++)
            {
                if (!zones[i]->slots[j]->isOccupied)
                {
                    zones[i]->slots[j]->occupy(v->vehId);
                    cout << "Re-routed to Zone " << zones[i]->ZoneID << endl;
                    return zones[i]->slots[j];
                }
            }
        }
    }

    return nullptr;
}