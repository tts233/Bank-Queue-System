// 禁用某些CRT安全警告（用于兼容旧函数）
#define _CRT_SECURE_NO_WARNINGS

// 引入头文件
#include "QueueManager.h"     // 队列管理器类声明
#include "LogManager.h"       // 日志管理器类声明
#include <fstream>            // 文件读写
#include <vector>             // 向量容器
#include <iostream>           // 标准输入输出
#include <algorithm>          // STL算法
#include <filesystem>         // 文件系统操作
#include <direct.h>           // Windows下创建目录
#include <io.h>               // 文件访问检查
#include <sys/stat.h>         // 文件权限设置
#include <mutex>              // 互斥锁
#include <unordered_map>      // 哈希表

// 使用标准命名空间
using namespace std;

// 根据操作系统定义清屏命令和创建目录方式
#ifdef _WIN32
#include <io.h>
#define access _access
#define mkdir(dir) _mkdir(dir)
#define CLEAR "cls"           // Windows清屏命令
#else
#include <unistd.h>
#define CLEAR "clear"         // Linux清屏命令
#endif

// 必须先定义静态成员变量（类外定义）
mutex QueueManager::queueMutex;  // 互斥锁，保护队列数据访问
map<string, QueueManager::WindowInfo> QueueManager::windows;  // 所有窗口信息，key为窗口ID
unordered_map<string, pair<bool, string>> QueueManager::userQueueType;  // 用户排队状态（VIP标志, 分配的窗口ID）

// 获取当前可用窗口（不加锁版，仅供内部使用）
vector<string> QueueManager::getAvailableWindows_NO_LOCK(bool forVip) {
    vector<string> available;
    if (windows.empty()) return available;
    for (const auto& [windowId, info] : windows) {
        if (info.isActive && (!forVip || info.isVipWindow)) {
            available.push_back(windowId);
        }
    }
    return available;
}

// 注册窗口
bool QueueManager::registerWindow(const string& windowId,
    const string& staffId,
    bool isVipWindow) {
    lock_guard<mutex> lock(queueMutex);

    // 验证窗口ID格式
    if (windowId.empty() || windowId.length() > 20) {
        LogManager::log("注册失败：窗口ID长度无效");
        return false;
    }

    // 检查非法字符
    for (char c : windowId) {
        if (!isalnum(c) && c != '_') {
            LogManager::log("注册失败：窗口ID包含非法字符");
            return false;
        }
    }

    // 检查是否已存在
    if (windows.count(windowId)) {
        LogManager::log("注册失败：窗口ID已存在");
        return false;
    }

    // 验证员工ID
    if (staffId.empty()) {
        LogManager::log("注册失败：无效的员工ID");
        return false;
    }

    // 创建数据目录
    string dirPath = "data/" + string(isVipWindow ? "vip" : "normal");
    if (!filesystem::exists(dirPath)) {
        try {
            filesystem::create_directories(dirPath);
        }
        catch (...) {
            LogManager::log("注册失败：无法创建数据目录");
            return false;
        }
    }

    // 注册窗口
    windows[windowId] = WindowInfo{ windowId, staffId, isVipWindow, true };

    // 创建初始队列文件
    string queueFile = getQueueFilePath(isVipWindow, windowId);
    ofstream fout(queueFile);
    if (!fout) {
        windows.erase(windowId); // 回滚
        LogManager::log("注册失败：无法创建队列文件");
        return false;
    }

    LogManager::log("成功注册窗口: " + windowId);
    return true;
}
// 激活或关闭窗口
bool QueueManager::activateWindow(const string& windowId, bool activate) {
    lock_guard<mutex> lock(queueMutex);

    auto it = windows.find(windowId);
    if (it == windows.end()) {
        LogManager::log("[错误] 窗口不存在: " + windowId);
        return false;
    }

    it->second.isActive = activate;
    LogManager::log("[状态] 窗口 " + windowId +
        " 设置为: " + (activate ? "活跃" : "关闭"));
    return true;
}

// 创建 data 目录（根据平台区分）
static void createDataDir() {
#ifdef _WIN32
    _mkdir("data");
#else
    mkdir("data", 0777);
#endif
}

// 获取队列文件路径（可以是全局或窗口专属）
string QueueManager::getQueueFilePath(bool isVip, const string& windowId) {
    // 使用现有的TXT文件而不是创建新的.queue文件
    if (windowId.empty()) {
        // 全局队列文件
        return isVip ? "data/queue_vip.txt" : "data/queue_normal.txt";
    }

    // 窗口专属队列（如果需要）
    const string typeDir = isVip ? "vip/" : "normal/";
    string path = "data/" + typeDir;
    if (!filesystem::exists(path)) {
        filesystem::create_directories(path);
    }
    return path + windowId + ".txt";  // 使用.txt扩展名保持一致性
}
// 用户取号
bool QueueManager::takeNumber(const string& cardId, bool isVip) {
    // 1. 验证卡号格式
    if (cardId.length() != 6 || !all_of(cardId.begin(), cardId.end(), ::isdigit)) {
        cerr << "错误：卡号必须为6位数字\n";
        return false;
    }

    // 2. 获取互斥锁保护共享数据
    unique_lock<mutex> lock(queueMutex);

    try {
        // 3. 系统状态检查
        if (windows.empty()) {
            throw runtime_error("系统未初始化：没有注册任何服务窗口");
        }

        // 4. 检查用户是否已在队列中
        if (userQueueType.find(cardId) != userQueueType.end()) {
            throw runtime_error("该用户已在队列中");
        }

        // 5. 获取可用窗口（严格区分VIP和普通窗口）
        vector<string> suitableWindows;
        for (const auto& [windowId, info] : windows) {
            if (info.isActive && (!isVip || info.isVipWindow)) {
                suitableWindows.push_back(windowId);
            }
        }

        if (suitableWindows.empty()) {
            throw runtime_error(isVip ? "没有可用的VIP窗口" : "没有可用的服务窗口");
        }

        // 6. 选择队列最短的窗口
        string selectedWindow = *min_element(
            suitableWindows.begin(), suitableWindows.end(),
            [isVip](const string& a, const string& b) {
                return getQueueCountForWindow(getQueueFilePath(isVip, a)) <
                    getQueueCountForWindow(getQueueFilePath(isVip, b));
            });
        // 7. 确保队列目录存在
        const string globalQueueFile = getQueueFilePath(isVip, "");
        ofstream globalFout(globalQueueFile, ios::app);
        if (!globalFout) {
            throw runtime_error("无法写入全局队列文件: " + globalQueueFile);
        }
        globalFout << cardId << "\n";
        globalFout.close();
        // 8. 写入窗口专属队列文件
        userQueueType[cardId] = { isVip, selectedWindow };
        // 9. 记录日志
        LogManager::log(string("[取号成功] 用户") + cardId +
            " -> " + (isVip ? "VIP" : "普通") +
            "窗口" + selectedWindow);

        return true;
    }
    catch (const exception& e) {
        // 错误处理
        cerr << "系统错误: " << e.what() << "\n";
        LogManager::log(string("[取号失败] 用户") + cardId + ": " + e.what());
        return false;
    }
}
// 用户查看排队位置
void QueueManager::viewQueue(const string& cardId) {
    lock_guard<mutex> lock(queueMutex);

    try {
        auto it = userQueueType.find(cardId);
        if (it == userQueueType.end()) {
            cout << "您当前没有排队记录\n";
            return;
        }

        bool isVip = it->second.first;
        string queueFile = getQueueFilePath(isVip, "");

        vector<string> queue;
        ifstream fin(queueFile);
        string line;
        while (getline(fin, line)) {
            if (!line.empty()) queue.push_back(line);
        }

        auto pos = find(queue.begin(), queue.end(), cardId);
        if (pos != queue.end()) {
            size_t position = distance(queue.begin(), pos);
            cout << "您当前在" << (isVip ? "VIP" : "普通")
                << "队列中，前面还有" << position << "人等待\n";
        }
        else {
            cout << "您的排队信息有误，请重新取号\n";
            userQueueType.erase(it);
        }
    }
    catch (...) {
        cout << "系统错误，无法查询排队信息\n";
    }
}// 用户取消排队
bool QueueManager::cancelNumber(const string& cardId) {
    lock_guard<mutex> lock(queueMutex);

    auto it = userQueueType.find(cardId);
    if (it == userQueueType.end()) return false;

    bool isVip = it->second.first;
    string windowId = it->second.second;
    string filename = getQueueFilePath(isVip, windowId);

    vector<string> remainingUsers;
    bool found = false;

    ifstream fin(filename);
    if (fin) {
        string line;
        while (getline(fin, line)) {
            if (line != cardId) {
                remainingUsers.push_back(line);
            }
            else {
                found = true;
            }
        }
        fin.close();
    }

    if (found) {
        ofstream fout(filename);
        for (const auto& user : remainingUsers) {
            fout << user << "\n";
        }
        userQueueType.erase(it);
        return true;
    }
    return false;
}

// 窗口叫下一个号
void QueueManager::callNextNumber(const string& windowId) {
    lock_guard<mutex> lock(queueMutex);

    try {
        auto windowIt = windows.find(windowId);
        if (windowIt == windows.end() || !windowIt->second.isActive) {
            throw runtime_error("无效或未激活的窗口");
        }

        bool isVip = windowIt->second.isVipWindow;
        string queueFile = getQueueFilePath(isVip, "");

        // 从全局队列文件读取
        vector<string> queue;
        ifstream fin(queueFile);
        string line;
        while (getline(fin, line)) {
            if (!line.empty()) queue.push_back(line);
        }

        if (!queue.empty()) {
            string called = queue.front();
            queue.erase(queue.begin());

            // 更新全局队列文件
            ofstream fout(queueFile);
            for (const auto& id : queue) {
                fout << id << "\n";
            }

            // 更新内存
            userQueueType.erase(called);

            cout << "============================\n";
            cout << "请卡号为 " << called << " 的用户到 "
                << windowId << " 窗口办理业务！\n";
            cout << "============================\n";
        }
        else {
            cout << "当前无排队用户\n";
        }
    }
    catch (const exception& e) {
        cerr << "[错误] " << e.what() << "\n";
    }
}// 查看窗口服务信息
void QueueManager::viewCurrentService(const string& windowId) {
    lock_guard<mutex> lock(queueMutex);
    auto it = windows.find(windowId);

    if (it != windows.end()) {
        cout << "窗口[" << windowId << "] 员工:" << it->second.assignedStaff;
            
    }
}

// 清空整个队列
void QueueManager::clearQueue(bool isVip) {
    lock_guard<mutex> lock(queueMutex);

    ofstream fout(getQueueFilePath(isVip, ""));

    for (auto it = userQueueType.begin(); it != userQueueType.end(); ) {
        if (it->second.first == isVip)
            it = userQueueType.erase(it);
        else
            ++it;
    }

    LogManager::log("管理员清空了 " + string(isVip ? "VIP" : "普通") + " 队列");
    cout << "队列已清空。\n";
}

// 查看所有全局队列用户
void QueueManager::viewAllQueues() {
    lock_guard<mutex> lock(queueMutex);
    for (bool vip : {false, true}) {
        ifstream fin(getQueueFilePath(vip, ""));
        string id;
        while (getline(fin, id)) cout << "  " << id << endl;
    }
}

// 保存队列到文件
bool QueueManager::saveQueueToFile(bool isVip, const vector<string>& queue, const string& windowId) {
    try {
        string filename = getQueueFilePath(isVip) + "_" + windowId;
        ofstream fout(filename);
        if (!fout) return false;
        for (const auto& user : queue) {
            fout << user << "\n";
        }
        return true;
    }
    catch (...) {
        return false;
    }
}

// 查询用户是否在队列中
bool QueueManager::isUserInQueue(const string& cardId) {
    lock_guard<mutex> lock(queueMutex);
    return userQueueType.find(cardId) != userQueueType.end();
}

// 获取窗口队列用户数量
size_t QueueManager::getQueueCountForWindow(const string& filename) {
    size_t count = 0;
    ifstream fin(filename);
    if (fin) {
        string line;
        while (getline(fin, line)) {
            if (!line.empty()) count++;
        }
    }
    return count;
}

// 获取当前可用窗口（加锁版，供外部使用）
vector<string> QueueManager::getAvailableWindows(bool forVip) {
    lock_guard<mutex> lock(queueMutex);
    vector<string> available;

    for (const auto& [windowId, info] : windows) {
        if (info.isActive && (!forVip || info.isVipWindow)) {
            available.push_back(windowId);
        }
    }

    cout << "可用" << (forVip ? "VIP" : "普通") << "窗口: ";
    for (const auto& id : available) cout << id << " ";
    cout << "\n";

    return available;
}

// 打印当前系统状态（调试用）
void QueueManager::debugPrintStatus() {
#if ENABLE_DEBUG_OUTPUT
    auto checkFile = [](const string& path) {
        ifstream f(path);
        return f.good() ? "有效" : "无效";
        };

    cout << "\n=== 线程安全状态 ===\n";
    cout << "内存数据:\n"
        << "  注册窗口数: " << windows.size() << "\n"
        << "  排队用户数: " << userQueueType.size() << "\n";
    cout << "文件状态:\n"
        << "  普通队列: " << checkFile(getQueueFilePath(false)) << "\n"
        << "  VIP队列: " << checkFile(getQueueFilePath(true)) << "\n";
#endif
}

void QueueManager::resetAllQueues() {
    lock_guard<mutex> lock(queueMutex);

    // 清空内存数据
    userQueueType.clear();

    // 清空全局队列文件
    ofstream("data/queue_normal.txt", ios::trunc).close();
    ofstream("data/queue_vip.txt", ios::trunc).close();

    // 清空所有窗口队列文件
    for (const auto& [windowId, _] : windows) {
        string file = getQueueFilePath(false, windowId);
        ofstream(file, ios::trunc).close();
        file = getQueueFilePath(true, windowId);
        ofstream(file, ios::trunc).close();
    }
}
void QueueManager::synchronizeQueues() {
    lock_guard<mutex> lock(queueMutex);

    // 1. 清空所有现有队列文件
    for (const auto& [windowId, _] : windows) {
        // 清空普通窗口队列
        string normalFile = getQueueFilePath(false, windowId);
        ofstream(normalFile, ios::trunc).close();

        // 清空VIP窗口队列
        string vipFile = getQueueFilePath(true, windowId);
        ofstream(vipFile, ios::trunc).close();
    }

    // 清空全局队列文件
    ofstream(getQueueFilePath(false, ""), ios::trunc).close();
    ofstream(getQueueFilePath(true, ""), ios::trunc).close();

    // 2. 根据内存数据重建所有队列文件
    for (const auto& [user, info] : userQueueType) {
        bool isVip = info.first;
        string windowId = info.second;
        string queueFile = getQueueFilePath(isVip, windowId);

        // 追加用户到对应队列文件
        ofstream fout(queueFile, ios::app);
        fout << user << "\n";
    }

    LogManager::log("[系统] 已完成队列同步，共同步 " +
        to_string(userQueueType.size()) + " 个用户");
}
