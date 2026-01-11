#ifndef parkingslot_h
#define parkingslot_h
#include <string>
using namespace std;

class ParkingSlot
{
public:
    int slotNum;
    int zoneNum;
    bool isOccupied;
    string vehId;

    ParkingSlot(int sid, int zid);

    void occupy(string vehId);
    void free();
};
#endif