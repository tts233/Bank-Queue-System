#include "LogManager.h"
#include <fstream>
#include <ctime>

void LogManager::log(const string& msg) {
    ofstream fout("data/logs.txt", ios::app);
    time_t now = time(nullptr);
   // fout << "[" << ctime(&now) << "] " << msg << endl;
}
