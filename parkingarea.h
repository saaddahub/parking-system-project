#ifndef PARKINGAREA_H
#define PARKINGAREA_H
#include "parkingslot.h"

class ParkingArea
{
public:
    int areaID;
    int zoneID;
    int capacity;
    int currentCount;
    ParkingSlot **slots;

    ParkingArea(int aID, int zID, int cap);
    ~ParkingArea();
    void addSlot(int slotNum);
};
#endif