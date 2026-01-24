#include "allocationengine.h"
#include <iostream>
using namespace std;

ParkingSlot *AllocationEngine::assignSlot(Vehicle *v, Zone **zones, int numZones)
{
    // 1. Check Preferred Zone
    for (int i = 0; i < numZones; i++)
    {
        if (zones[i]->zoneID == v->preferredZoneID)
        {
            if (!zones[i]->isFull())
            {
                for (int a = 0; a < zones[i]->CurCount; a++)
                {
                    for (int s = 0; s < zones[i]->areas[a]->currentCount; s++)
                    {
                        if (!zones[i]->areas[a]->slots[s]->isOccupied)
                        {
                            // REMOVED: occupy() call. ParkingSystem will handle it.
                            return zones[i]->areas[a]->slots[s];
                        }
                    }
                }
            }
            break;
        }
    }

    // 2. Cross-Zone Allocation
    cout << "Preferred zone full... Searching elsewhere." << endl;
    for (int i = 0; i < numZones; i++)
    {
        if (zones[i]->zoneID == v->preferredZoneID)
            continue;
        if (!zones[i]->isFull())
        {
            for (int a = 0; a < zones[i]->CurCount; a++)
            {
                for (int s = 0; s < zones[i]->areas[a]->currentCount; s++)
                {
                    if (!zones[i]->areas[a]->slots[s]->isOccupied)
                    {
                        // REMOVED: occupy() call.
                        cout << "Re-routed to Zone " << zones[i]->zoneID << endl;
                        return zones[i]->areas[a]->slots[s];
                    }
                }
            }
        }
    }
    return nullptr;
}