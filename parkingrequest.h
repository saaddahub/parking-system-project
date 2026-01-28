#ifndef parkingrequest_h
#define parkingrequest_h
#include "vehicle.h"
#include <string>

enum RequestState {
    REQUESTED,
    ALLOCATED,
    OCCUPIED,
    RELEASED,
    CANCELLED
};

class ParkingRequest
{
public:
    Vehicle *vehicle;
    RequestState status; // Use Enum
    
    int startTime;
    int endTime;
    double penaltyCost;

    ParkingRequest(Vehicle *v);
    bool setState(RequestState newState); // Warning: bool return for success/fail
    // Helper to print state
    std::string getStateString();
};
#endif
