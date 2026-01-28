#include "parkingrequest.h"
#include <iostream>
using namespace std;

ParkingRequest::ParkingRequest(Vehicle *v)
{
    this->vehicle = v;
    this->status = REQUESTED;
    this->startTime = 0;
    this->endTime = 0;
    this->penaltyCost = 0.0;
}

bool ParkingRequest::setState(RequestState newState)
{
    // Idempotency check: If new state is same as current, return true (no-op)
    if (this->status == newState) return true;

    bool allowed = false;
    switch (this->status)
    {
    case REQUESTED:
        if (newState == ALLOCATED || newState == CANCELLED) allowed = true;
        break;
    case ALLOCATED:
        if (newState == OCCUPIED || newState == CANCELLED) allowed = true;
        break;
    case OCCUPIED:
        // Allow RELEASED (Normal exit) or CANCELLED (Rollback)
        if (newState == RELEASED || newState == CANCELLED) allowed = true;
        break;
    case RELEASED:
        // Terminal state
        break;
    case CANCELLED:
        // Terminal state
        break;
    }

    if (allowed)
    {
        this->status = newState;
        return true;
    }
    
    cout << "[ERROR] Invalid State Transition from " << this->status << " to " << newState << endl;
    return false;
}

string ParkingRequest::getStateString() {
    switch(status) {
        case REQUESTED: return "REQUESTED";
        case ALLOCATED: return "ALLOCATED";
        case OCCUPIED: return "OCCUPIED";
        case RELEASED: return "RELEASED";
        case CANCELLED: return "CANCELLED";
        default: return "UNKNOWN";
    }
}