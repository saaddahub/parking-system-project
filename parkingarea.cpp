#include "parkingarea.h"

ParkingArea::ParkingArea(int aID, int zID, int cap)
{
    this->areaID = aID;
    this->zoneID = zID;
    this->capacity = cap;
    this->currentCount = 0;

    this->slots = new ParkingSlot *[capacity];
    for (int i = 0; i < capacity; i++)
        this->slots[i] = nullptr;
}

ParkingArea::~ParkingArea()
{
    for (int i = 0; i < currentCount; i++)
        delete slots[i];
    delete[] slots;
}

void ParkingArea::addSlot(int slotNum)
{
    if (currentCount < capacity)
    {
        slots[currentCount] = new ParkingSlot(slotNum, this->zoneID);
        currentCount++;
    }
}