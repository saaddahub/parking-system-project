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
    void occupy(string vId);
    void free();
};
#endif