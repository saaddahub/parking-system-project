#include "rollbackmanager.h"
#include <iostream>
using namespace std;

RollbackManager::RollbackManager()
{
    top = nullptr;
}

void RollbackManager::pushOperation(int type, ParkingRequest *req, ParkingSlot *slot)
{
    RollbackNode *node = new RollbackNode();
    node->type = type;
    node->request = req;
    node->slot = slot;
    node->next = top;
    top = node;
}

RollbackNode *RollbackManager::popOperation()
{
    if (top == nullptr)
        return nullptr;
    RollbackNode *node = top;
    top = top->next;
    return node;
}

bool RollbackManager::isEmpty()
{
    return top == nullptr;
}