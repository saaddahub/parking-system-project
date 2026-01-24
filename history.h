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
    HistoryRecord *head;
    int count;

    HistoryManager()
    {
        head = nullptr;
        count = 0;
    }
    void add(ParkingRequest *req)
    {
        HistoryRecord *newRecord = new HistoryRecord();
        newRecord->data = req;
        newRecord->next = head;
        head = newRecord;
        count++;
    }

    double getAvgDuration()
    {
        if (count == 0)
            return 0.0;
        double totalDuration = 0;
        HistoryRecord *current = head;
        while (current != nullptr)
        {
            totalDuration += (current->data->endTime - current->data->startTime);
            current = current->next;
        }
        return totalDuration / count;
    }
    double getTotalRevenue()
    {
        double total = 0;
        HistoryRecord *current = head;
        while (current != nullptr)
        {
            // Base Fee 10 + Penalty
            total += (10.0 + current->data->penaltyCost);
            current = current->next;
        }
        return total;
    }
};

#endif