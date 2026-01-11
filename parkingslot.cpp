#include <parkingslot.h>
#include <iostream>
using namespace std;

ParkingSlot::ParkingSlot(int sid, int zid)
{
    this->slotNum = sid;
    this->zoneNum = zid;
    this->isOccupied = false;
    this->vehId = "";
}
