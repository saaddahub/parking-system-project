#ifndef allocationenigne_h
#define allocationenigne_h

#include "zone.h"
#include "vehicle.h"
#include "parkingslot.h"

class AllocationEngine
{
public:
    ParkingSlot* assignSlot(vehicle *veh, zone **zones, int numZones);
};

#endif