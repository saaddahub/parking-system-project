#include "allocationengine.h"
#include <iostream>
using namespace std;

ParkingSlot *AllocationEngine::assignSlot(Vehicle *v, Zone **zones, int numZones)
{

    // --- STRATEGY 1: Check Preferred Zone ---
    for (int i = 0; i < numZones; i++)
    {
        if (zones[i]->zoneID == v->preferredZoneID)
        {

            if (!zones[i]->isFull())
            {
                // Loop through AREAS using your variable 'CurCount'
                for (int a = 0; a < zones[i]->CurCount; a++)
                {

                    // Loop through SLOTS in that area
                    for (int s = 0; s < zones[i]->areas[a]->currentCount; s++)
                    {

                        if (!zones[i]->areas[a]->slots[s]->isOccupied)
                        {
                            // Found space!
                            zones[i]->areas[a]->slots[s]->occupy(v->vehId);
                            return zones[i]->areas[a]->slots[s];
                        }
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
        if (zones[i]->zoneID == v->preferredZoneID)
            continue;

        if (!zones[i]->isFull())
        {
            // Check Areas (CurCount)
            for (int a = 0; a < zones[i]->CurCount; a++)
            {
                // Check Slots (currentCount)
                for (int s = 0; s < zones[i]->areas[a]->currentCount; s++)
                {

                    if (!zones[i]->areas[a]->slots[s]->isOccupied)
                    {
                        zones[i]->areas[a]->slots[s]->occupy(v->vehId);
                        cout << "Re-routed to Zone " << zones[i]->zoneID << endl;
                        return zones[i]->areas[a]->slots[s];
                    }
                }
            }
        }
    }

    return nullptr;
}