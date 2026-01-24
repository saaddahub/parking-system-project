#ifndef parkingrequest_h
#define parkingrequest_h
#include "vehicle.h"
#include <string>

class ParkingRequest
{
public:
    Vehicle *vehicle;

    int status;
    int startTime;
    int endTime;
    double penaltyCost;

    ParkingRequest(Vehicle *v);
    void updateStatus(int newStatus);
};
#endif
