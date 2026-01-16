#include "parkingarea.h"

ParkingArea::ParkingArea(int aID, int zID, int cap)
{
    this->areaID = aID;
    this->zoneID = zID;
    this->capacity = cap;
    this->currentCount = 0;

    // Allocate memory for the slots
    this->slots = new ParkingSlot *[capacity];
    for (int i = 0; i < capacity; i++)
    {
        this->slots[i] = nullptr;
    }
}

ParkingArea::~ParkingArea()
{
    // Delete all slots inside this area
    for (int i = 0; i < currentCount; i++)
    {
        delete slots[i];
    }
    delete[] slots;
}

void ParkingArea::addSlot(int slotID)
{
    if (currentCount < capacity)
    {
        // Create a new slot. Note: We pass zoneID so the slot knows where it lives.
        slots[currentCount] = new ParkingSlot(slotID, this->zoneID);
        currentCount++;
    }
}