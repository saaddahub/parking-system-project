#ifndef vehicle_h
#define vehicle_h
#include <string>
    using namespace std;

class vehicle
{
public:
    string vehId;
    int ZoneToGoId;

    vehicle(string id, int ztgid);
};

#endif