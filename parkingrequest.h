#ifndef PARKINGREQUEST_H
#define PARKINGREQUEST_H

#include "vehicle.h"
#include <ctime>
#include <string>
using namespace std;

// The PDF specifically asks for these states
enum RequestStatus
{
    REQUESTED,
    ALLOCATED,
    OCCUPIED,
    CANCELLED,
    RELEASED
};

class ParkingRequest
{
public:
    int requestID;
    Vehicle *vehicle;
    RequestStatus status;
    time_t requestTime; // PDF asks for "Request time" [cite: 41]

    ParkingRequest(Vehicle *v);

    void updateStatus(RequestStatus newStatus);
    string getStatusString();
};

#endif