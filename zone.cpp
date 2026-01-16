#include "zone.h"

Zone::Zone(int id, int cap)
{
    this->zoneID = id;
    this->capacity = cap;
    this->CurCount = 0;

    this->areas = new ParkingArea *[capacity];
    for (int i = 0; i < capacity; i++)
        this->areas[i] = nullptr;
}

Zone::~Zone()
{
    for (int i = 0; i < CurCount; i++)
        delete areas[i];
    delete[] areas;
}

void Zone::addArea(int areaID, int capacityPerArea)
{
    if (CurCount < capacity)
    {
        areas[CurCount] = new ParkingArea(areaID, this->zoneID, capacityPerArea);
        CurCount++;
    }
}

bool Zone::isFull()
{
    if (CurCount == 0)
        return true;
    for (int i = 0; i < CurCount; i++)
    {
        for (int j = 0; j < areas[i]->currentCount; j++)
        {
            if (!areas[i]->slots[j]->isOccupied)
                return false;
        }
    }
    return true;
}