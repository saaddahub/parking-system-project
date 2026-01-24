#include "rollbackmanager.h"
#include <iostream>

RollbackManager::RollbackManager()
{
    this->top = nullptr;
}

void RollbackManager::pushOperation(int type, ParkingRequest *req, ParkingSlot *slot)
{
    RollbackNode *newNode = new RollbackNode;
    newNode->type = type;
    newNode->request = req;
    newNode->slot = slot;
    newNode->next = top;
    top = newNode;
}

RollbackNode *RollbackManager::popOperation()
{
    if (isEmpty())
        return nullptr;
    RollbackNode *temp = top;
    top = top->next;
    return temp;
}

bool RollbackManager::isEmpty()
{
    return top == nullptr;
}