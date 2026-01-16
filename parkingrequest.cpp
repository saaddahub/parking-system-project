#include "parkingrequest.h"

// A global counter to give every request a unique ID (1001, 1002, etc.)
int globalRequestID = 1000;

ParkingRequest::ParkingRequest(Vehicle *v)
{
    this->vehicle = v;
    this->requestID = ++globalRequestID;
    this->status = REQUESTED;    // Initial state is always REQUESTED [cite: 46]
    this->requestTime = time(0); // Stores the exact second the request was made
}

void ParkingRequest::updateStatus(RequestStatus newStatus)
{
    // In a real strict system, we would check if the transition is valid here.
    // For example, you can't go from REQUESTED to RELEASED directly.
    this->status = newStatus;
}

string ParkingRequest::getStatusString()
{
    switch (status)
    {
    case REQUESTED:
        return "REQUESTED";
    case ALLOCATED:
        return "ALLOCATED";
    case OCCUPIED:
        return "OCCUPIED";
    case CANCELLED:
        return "CANCELLED";
    case RELEASED:
        return "RELEASED";
    default:
        return "UNKNOWN";
    }
}