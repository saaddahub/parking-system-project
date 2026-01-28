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

RollbackManager::~RollbackManager()
{
    while(top != nullptr) {
        RollbackNode* temp = top;
        top = top->next;
        // Don't delete the request/vehicle here because they might be in History or active use.
        // The undo stack just holds references to actions.
        // Actually, if we undo a PARK action, we might have to delete the request if it's not used anywhere else?
        // But for shutdown, we just want to clear the nodes.
        delete temp;
    }
}