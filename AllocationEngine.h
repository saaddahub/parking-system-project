#ifndef ALLOCATIONENGINE_H
#define ALLOCATIONENGINE_H
#include "zone.h"
#include "vehicle.h"
#include "parkingslot.h"

class AllocationEngine
{
public:
    ParkingSlot *assignSlot(Vehicle *v, Zone **zones, int numZones);
};
#endif