#ifndef zone_h
#define zone_h

#include "parkingslot.h"

class zone
{
public:
    int ZoneID;
    ;
    ParkingSlot **slots; // made a dynamic array of pointers pointers
    int capacity;
    int CurCount;

    zone(int zid, int cap);
    ~zone(); // destructor

    void SlotAddition(int slotid);
    bool isFull();
};

#endif