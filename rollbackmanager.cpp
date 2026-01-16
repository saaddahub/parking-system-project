#include "rollbackmanager.h"
#include <iostream>

RollbackManager::RollbackManager()
{
    this->top = nullptr;
}

void RollbackManager::pushOperation(ActionType type, ParkingRequest *req, ParkingSlot *slot)
{
    HistoryNode *newNode = new HistoryNode;
    newNode->type = type;
    newNode->request = req;
    newNode->slot = slot;
    newNode->next = top;
    top = newNode;
}

HistoryNode *RollbackManager::popOperation()
{
    if (isEmpty())
        return nullptr;
    HistoryNode *temp = top;
    top = top->next;
    return temp;
}

bool RollbackManager::isEmpty()
{
    return top == nullptr;
}
