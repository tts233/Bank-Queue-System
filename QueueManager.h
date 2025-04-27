
#pragma once
#include <string>
using namespace std;

class QueueManager {
public:
    static void takeNumber(const string& cardId, bool isVip);
    static void viewQueue(const string& cardId);
    static void cancelNumber(const string& cardId);
    static void callNextNumber(const string& staffId);
    static void viewCurrentService(const string& staffId);
    static void clearQueue(bool isVip);
    static void viewAllQueues();
};
