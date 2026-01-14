#include "AllocationEngine.h"
#include <iostream>
using namespace std;

ParkingSlot *AllocationEngine::assignSlot(vehicle *v, zone **zones, int numZones)
{
    for (int i = 0; i < numZones; i++)
    {
        if (zones[i]->ZoneID == v->ZoneToGoId)
        {

            if (!zones[i]->isFull())
            {

                for (int j = 0; j < zones[i]->CurCount; j++)
                {
                    if (!zones[i]->slots[j]->isOccupied)
                    {

                        zones[i]->slots[j]->occupy(v->vehId);
                        return zones[i]->slots[j]; // Return the winning slot
                    }
                }
            }
            break;
        }

        for (int i = 0; i < numZones; i++)
        {

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
        return nullptr;
    }