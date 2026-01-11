#include "parkingslot.h"
#include <iostream>
using namespace std;

ParkingSlot::ParkingSlot(int sid, int zid)
{
    this->SlotID = sid;
    this->zoneID = zid;
    this->isOccupied = false;
    this->OccByVehicleID = "";
}

void ParkingSlot::occupy(string vehId)
{
    this->isOccupied = true;
    this->OccByVehicleID = vehId;
}
void ParkingSlot::free()
{
    this->isOccupied = false;
    this->OccByVehicleID = "";
}