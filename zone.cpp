#include "zone.h"

Zone::Zone(int id, int cap)
{
    this->zoneID = id;
    this->capacity = cap;
    this->CurCount = 0;
    this->areas = new ParkingArea *[capacity];

    // Initialize Neighbors (Dynamic Array)
    this->neighborCapacity = 5; // Default max 5 neighbors
    this->neighborCount = 0;
    this->neighbors = new Zone *[neighborCapacity];
}

Zone::~Zone()
{
    for (int i = 0; i < CurCount; i++)
    {
        delete areas[i];
    }
    delete[] areas;
    delete[] neighbors; // Clean up adjacency
}

void Zone::addArea(int areaID, int capacityPerArea)
{
    if (CurCount < capacity)
    {
        areas[CurCount] = new ParkingArea(areaID, this->zoneID, capacityPerArea);
        CurCount++;
    }
}

void Zone::addNeighbor(Zone *neighbor)
{
    if (neighborCount < neighborCapacity)
    {
        neighbors[neighborCount] = neighbor;
        neighborCount++;
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