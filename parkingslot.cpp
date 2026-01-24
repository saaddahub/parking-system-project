#include "parkingslot.h"

ParkingSlot::ParkingSlot(int sNum, int zNum)
{
    this->slotNum = sNum;
    this->zoneNum = zNum;
    this->isOccupied = false;
    this->vehId = "";
    this->currentReq = nullptr;
}

void ParkingSlot::occupy(string vId, ParkingRequest *req)
{
    this->isOccupied = true;
    this->vehId = vId;
    this->currentReq = req;
}

void ParkingSlot::free()
{
    this->isOccupied = false;
    this->vehId = "";
    this->currentReq = nullptr;
}