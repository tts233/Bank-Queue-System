#include "QueueManager.h"
#include "LogManager.h"
#include <fstream>
#include <vector>
#include <iostream>
#include <algorithm>

static string getFilename(bool isVip) {
    return isVip ? "data/queue_vip.txt" : "data/queue_normal.txt";
}

void QueueManager::takeNumber(const string& cardId, bool isVip) {
    ofstream fout(getFilename(isVip), ios::app);
    fout << cardId << endl;
    LogManager::log("用户[" + cardId + "] 取号 (" + (isVip ? "VIP" : "普通") + ")");
    cout << "您的排队号码已生成。\n";
}

void QueueManager::viewQueue(const string& cardId) {
    for (bool vip : {false, true}) {
        ifstream fin(getFilename(vip));
        vector<string> queue;
        string id;
        while (getline(fin, id)) queue.push_back(id);
        cout << (vip ? "VIP" : "普通") << "排队人数: " << queue.size() << endl;
    }
}

void QueueManager::cancelNumber(const string& cardId) {
    for (bool vip : {false, true}) {
        ifstream fin(getFilename(vip));
        vector<string> queue;
        string id;
        while (getline(fin, id)) if (id != cardId) queue.push_back(id);
        fin.close();
        ofstream fout(getFilename(vip));
        for (auto& q : queue) fout << q << endl;
    }
    LogManager::log("用户[" + cardId + "] 取消排队");
    cout << "已取消排队。\n";
}

void QueueManager::callNextNumber(const string& staffId) {
    for (bool vip : {true, false}) {
        ifstream fin(getFilename(vip));
        vector<string> queue;
        string id;
        while (getline(fin, id)) queue.push_back(id);
        fin.close();
        if (!queue.empty()) {
            string called = queue.front();
            queue.erase(queue.begin());
            ofstream fout(getFilename(vip));
            for (auto& q : queue) fout << q << endl;
            LogManager::log("窗口[" + staffId + "] 叫号：" + called);
            cout << "请用户 " << called << " 到窗口处理业务。\n";
            return;
        }
    }
    cout << "当前无排队用户。\n";
}

void QueueManager::viewCurrentService(const string& staffId) {
    cout << "窗口[" << staffId << "] 当前服务用户功能暂未实现细化，可扩展。\n";
}

void QueueManager::clearQueue(bool isVip) {
    ofstream fout(getFilename(isVip));
    LogManager::log("管理员清空了 " + string(isVip ? "VIP" : "普通") + " 队列");
    cout << "队列已清空。\n";
}

void QueueManager::viewAllQueues() {
    for (bool vip : {false, true}) {
        ifstream fin(getFilename(vip));
        cout << (vip ? "[VIP]" : "[普通]") << " 队列用户：\n";
        string id;
        while (getline(fin, id)) cout << "  " << id << endl;
    }
}
