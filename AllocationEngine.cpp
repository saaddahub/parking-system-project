#include "AllocationEngine.h"
#include <iostream>
using namespace std;

ParkingSlot *AllocationEngine::assignSlot(vehicle *v, zone **zones, int numZones)
{

    // --- STRATEGY 1: Check Preferred Zone ---
    // We loop through zones to find the one matching the ID
    for (int i = 0; i < numZones; i++)
    {
        if (zones[i]->ZoneID == v->ZoneToGoId)
        {

            // We found the preferred zone. Is it full?
            if (!zones[i]->isFull())
            {
                // Not full! Find the first empty slot inside it.
                for (int j = 0; j < zones[i]->CurCount; j++)
                {
                    if (!zones[i]->slots[j]->isOccupied)
                    {
                        // Found it! Park the car here.
                        zones[i]->slots[j]->occupy(v->vehId);
                        return zones[i]->slots[j]; // Return the winning slot
                    }
                }
            }
            break; // Stop looking for preferred zone if we found it but it was full
        }
    }

    // --- STRATEGY 2: Cross-Zone Allocation (Backup Plan) ---
    // If we are here, the preferred zone was full. Look ANYWHERE else.
    for (int i = 0; i < numZones; i++)
    {
        // Skip the preferred one (we already checked it)
        if (zones[i]->ZoneID == v->ZoneToGoId)
            continue;

        if (!zones[i]->isFull())
        {
            // Found a backup zone! Find a slot.
            for (int j = 0; j < zones[i]->CurCount; j++)
            {
                if (!zones[i]->slots[j]->isOccupied)
                {
                    zones[i]->slots[j]->occupy(v->vehId);
                    cout << "Preferred zone full. Re-routed to Zone " << zones[i]->ZoneID << endl;
                    return zones[i]->slots[j];
                }
            }
        }
    }

    // --- STRATEGY 3: Failure ---
    return nullptr; // No space anywhere in the city
}