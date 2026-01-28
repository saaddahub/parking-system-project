#ifndef ROLLBACKMANAGER_H
#define ROLLBACKMANAGER_H

#include "parkingrequest.h"
#include "parkingslot.h"

// Types of actions we can undo - using constants for simplicity
const int PARK_ACTION = 0;
const int REMOVE_ACTION = 1;

// Renamed to RollbackNode to avoid conflict with HistoryNode in History.h
struct RollbackNode
{
    int type; // PARK_ACTION or REMOVE_ACTION
    ParkingRequest *request;
    ParkingSlot *slot;
    RollbackNode *next;
};

class RollbackManager
{
public:
    RollbackNode *top;

    RollbackManager();
    void pushOperation(int type, ParkingRequest *req, ParkingSlot *slot);
    RollbackNode *popOperation();
    bool isEmpty();
    ~RollbackManager();
};

#endif