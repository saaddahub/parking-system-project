#include "parkingslot.h"

ParkingSlot::ParkingSlot(int sNum, int zNum)
{
    this->slotNum = sNum;
    this->zoneNum = zNum;
    this->isOccupied = false;
    this->vehId = "";
}

void ParkingSlot::occupy(string vId)
{
    this->isOccupied = true;
    this->vehId = vId;
}

void ParkingSlot::free()
{
    this->isOccupied = false;
    this->vehId = "";
}