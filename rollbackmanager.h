#ifndef ROLLBACKMANAGER_H
#define ROLLBACKMANAGER_H

#include "parkingrequest.h"
#include "parkingslot.h"

// CONSTANTS (No Enum)
const int PARK_ACTION = 0;
const int REMOVE_ACTION = 1;

// Renamed to 'RollbackNode' to avoid conflict with History.h
struct RollbackNode
{
    int type; // 0 or 1
    ParkingRequest *request;
    ParkingSlot *slot;
    RollbackNode *next;
};

class RollbackManager
{
public:
    RollbackNode *top;

    RollbackManager();
    // Changed 'ActionType' to 'int'
    void pushOperation(int type, ParkingRequest *req, ParkingSlot *slot);
    RollbackNode *popOperation();
    bool isEmpty();
};

#endif