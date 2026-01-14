#include "zone.h"
#include <iostream>
using namespace std;

zone::zone(int id, int cap)
{
    this->ZoneID = id;
    this->capacity = cap;
    this->CurCount = 0;
    this->slots = new ParkingSlot *[capacity];

    // Initialize them to empty (nullptr)
    for (int i = 0; i < capacity; i++)
    {
        this->slots[i] = nullptr;
    }
}

// Destructor (Cleaning up memory)
zone::~zone()
{
    for (int i = 0; i < CurCount; i++)
    {
        delete slots[i];
    }
    delete[] slots;
}

void zone::SlotAddition(int slotID)
{
    if (CurCount < capacity)
    {
        slots[CurCount] = new ParkingSlot(slotID, this->ZoneID);
        CurCount++;
    }
}

// Check if full
bool zone::isFull()
{
    if (CurCount == 0)
        return true; // Safety check

    for (int i = 0; i < CurCount; i++)
    {
        // If we find a slot that is NOT occupied, the zone is NOT full
        if (slots[i]->isOccupied == false)
        {
            return false;
        }
    }
    return true; // All slots were occupied
}