#include "parkingrequest.h"
#include <iostream>
using namespace std;

const int REQUESTED = 0;
const int ALLOCATED = 1;
const int OCCUPIED = 2;
const int COMPLETED = 3;
const int CANCELLED = 4;

ParkingRequest::ParkingRequest(Vehicle *v)
{
    this->vehicle = v;
    this->status = REQUESTED;
    this->startTime = 0;
    this->endTime = 0;
    this->penaltyCost = 0.0;
}
void ParkingRequest::updateStatus(int newStatus)
}
{
    if (this->status == REQUESTED && newStatus == OCCUPIED)
    {
        cout << "ERROR, invalid state Transition" << endl;
        return;
    }
    this->status = newStatus;
}