#ifndef PARKINGSLOT_H
#define PARKINGSLOT_H
#include <string>
#include "parkingrequest.h"
using namespace std;

class ParkingSlot
{
public:
    int slotNum;
    int zoneNum;
    bool isOccupied;
    string vehId;

    ParkingRequest *currentReq;

    ParkingSlot(int sNum, int zNum);

    // FIXED: Must have 2 arguments for the new system
    void occupy(string vId, ParkingRequest *req);

    void free();
};
#endif