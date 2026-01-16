#ifndef ROLLBACKMANAGER_H
#define ROLLBACKMANAGER_H

#include "parkingrequest.h"
#include "parkingslot.h"

// Types of actions we can undo
enum ActionType
{
    PARK_ACTION,
    REMOVE_ACTION
};

// A Stack Node (Linked List)
struct HistoryNode
{
    ActionType type;
    ParkingRequest *request;
    ParkingSlot *slot;
    HistoryNode *next;
};

class RollbackManager
{
public:
    HistoryNode *top;

    RollbackManager();
    void pushOperation(ActionType type, ParkingRequest *req, ParkingSlot *slot);
    HistoryNode *popOperation();
    bool isEmpty();
};

#endif