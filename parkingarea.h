#ifndef PARKINGAREA_H
#define PARKINGAREA_H

#include "parkingslot.h"

class ParkingArea
{
public:
    int areaID;
    int zoneID;   // The Zone this area belongs to
    int capacity; // How many slots this area has
    int CurCount;

    // The PDF requires the Area to hold the slots, not the Zone
    ParkingSlot **slots;

    ParkingArea(int aID, int zID, int cap);
    ~ParkingArea();

    void addSlot(int slotID);
};

#endif