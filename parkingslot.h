#ifndef PARKINGSLOT_H
#define PARKINGSLOT_H
#include <string>
using namespace std;

class ParkingSlot
{
public:
    int slotNum;
    int zoneNum;
    bool isOccupied;
    string vehId; // Stores the ID of the car parked here

    ParkingSlot(int sNum, int zNum);
    void occupy(string vId);
    void free();
};
#endif