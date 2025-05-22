#include "LogManager.h"
#include <fstream>
#include <ctime>

void LogManager::log(const string& msg) {
    ofstream fout("data/logs.txt", ios::app);
    time_t now = time(nullptr);
    char timeStr[26];
    ctime_s(timeStr, sizeof(timeStr), &now);  // 使用安全版本
    fout << "[" << timeStr << "] " << msg << endl;
}
