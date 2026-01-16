#ifndef zone_h
#define zone_h

#include "parkingarea.h"

class zone
{
public:
    int ZoneID;
    int capacity;
    int CurCount;
    ParkingArea **areas;

    zone(int zid, int cap);
    ~zone(); // destructor

    void SlotAddition(int slotid, int CapacityPerArea);
    bool isFull();
};

#endif