#include "allocationengine.h"
#include "zone.h"
#include <iostream>
using namespace std;

ParkingSlot *AllocationEngine::assignSlot(Vehicle *v, Zone **zones, int numZones)
{
    Zone* preferred = nullptr;
    // 1. Find Preferred Zone Object
    for(int i=0; i<numZones; i++) {
        if(zones[i]->zoneID == v->preferredZoneID) {
            preferred = zones[i];
            break;
        }
    }

    if(!preferred) return nullptr;

    // 2. Try Preferred Zone
    if (!preferred->isFull())
    {
        for (int a = 0; a < preferred->CurCount; a++)
        {
            for (int s = 0; s < preferred->areas[a]->currentCount; s++)
            {
                if (!preferred->areas[a]->slots[s]->isOccupied)
                {
                    return preferred->areas[a]->slots[s];
                }
            }
        }
    }

    // 3. Cross-Zone Allocation (Using Adjacency)
    cout << "Preferred zone full... Searching NEIGHBORS." << endl;
    for(int i=0; i<preferred->neighborCount; i++) {
        Zone* neighbor = preferred->neighbors[i];
        if(!neighbor->isFull()) {
             for (int a = 0; a < neighbor->CurCount; a++)
            {
                for (int s = 0; s < neighbor->areas[a]->currentCount; s++)
                {
                    if (!neighbor->areas[a]->slots[s]->isOccupied)
                    {
                        cout << "Re-routed to Neighbor Zone " << neighbor->zoneID << endl;
                        return neighbor->areas[a]->slots[s];
                    }
                }
            }
        }
    }
    
    return nullptr;
}