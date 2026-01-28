#ifndef history_h
#define history_h
#include "parkingrequest.h"

struct HistoryRecord
{
    ParkingRequest *data;
    HistoryRecord *next;
};

class HistoryManager
{
private:
    HistoryRecord *head;

public:
    int count;
    int* zoneCounts;
    int maxZones;
    int cancelledCount;
    int completedCount;

    HistoryManager(int numZones)
    {
        head = nullptr;
        count = 0;
        maxZones = numZones;
        zoneCounts = new int[maxZones + 1]; // 1-based indexing for convenience
        for(int i=0; i<=maxZones; i++) zoneCounts[i] = 0;
        cancelledCount = 0;
        completedCount = 0;
    }

    void addRecord(ParkingRequest *req)
    {
        HistoryRecord *newRecord = new HistoryRecord();
        newRecord->data = req;
        newRecord->next = head;
        head = newRecord;
        count++;
    }

    void recordUsage(int zoneID) {
        if(zoneID > 0 && zoneID <= maxZones) {
            zoneCounts[zoneID]++;
        }
    }

    void recordCompletion() { completedCount++; }
    void recordCancellation() { cancelledCount++; }

    int getPeakZone() {
        int maxZ = 0;
        int maxVal = -1;
        for(int i=1; i<=maxZones; i++) {
            if(zoneCounts[i] > maxVal) {
                maxVal = zoneCounts[i];
                maxZ = i;
            }
        }
        return maxZ;
    }

    double getAverageDuration()
    {
        if (completedCount == 0) return 0.0;
        double totalDuration = 0;
        HistoryRecord *current = head;
        while (current != nullptr)
        {
            if(current->data->status == RELEASED) // only count finished valid trips
                 totalDuration += (current->data->endTime - current->data->startTime);
            current = current->next;
        }
        return totalDuration / completedCount; // Average of completed trips
    }

    double getTotalRevenue()
    {
        double total = 0;
        HistoryRecord *current = head;
        while (current != nullptr)
        {
            // Only count revenue for Released (Completed) trips
            if(current->data->status == RELEASED)
                total += (10.0 + current->data->penaltyCost);
            current = current->next;
        }
        return total;
    }
    ~HistoryManager() {
        HistoryRecord* current = head;
        while(current != nullptr) {
            HistoryRecord* next = current->next;
            // The actual data (ParkingRequest) is shared.
            // If History owns it, delete it. 
            // Requests are created in main/parkingsystem.
            // When a request is completed, it goes to History.
            // When cancelled (undo), it might be deleted or kept in a cancelled list?
            // Currently, cancelled ones are not added to History list (just counted).
            // So History only has 'RELEASED' requests? No, addRecord is called in removeVehicle.
            // So yes, we should delete them here if we are shutting down.
            if(current->data) {
                if(current->data->vehicle) delete current->data->vehicle;
                delete current->data;
            }
            delete current;
            current = next;
        }
        delete[] zoneCounts;
    }
};

#endif