#ifndef parkingslot_h
#define parkingslot_h
#include <string>
using namespace std;

class ParkingSlot
{
public:
    int SlotID;
    int zoneID;
    bool isOccupied;
    string OccByVehicleID;

    ParkingSlot(int sid, int zid);

    void occupy(string vehId);
    void free();
};
#endif