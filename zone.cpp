#include "zone.h"
#include <iostream>
using namespace std;

zone::zone(int id, int cap)
{
    this->ZoneID = id;
    this->capacity = cap;
    this->CurCount = 0;
    this->areas = new ParkingArea *[capacity];

    for (int i = 0; i < capacity; i++)
    {
        this->areas[i] = nullptr;
    }
}

// Destructor (Cleaning up memory)
zone::~zone()
{
    for (int i = 0; i < CurCount; i++)
    {
        delete areas[i];
    }
    delete[] areas;
}

void zone::SlotAddition(int slotID, int CapacityPerArea)
{
    if (CurCount < capacity)
    {
        areas[CurCount] = new ParkingArea(areaID, this->ZoneID, CapacityPerArea);
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
        for (int j = 0; j < areas[i]->CurCount; j++)
        {
            if (!areas[i]->slots[j]->isOccupied)
            {
                return false;
            }
        }
    }
    return true;
}