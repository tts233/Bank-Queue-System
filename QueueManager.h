#pragma once
#include <string>
#include <mutex>
#include <unordered_map>
#include <map>
#include <filesystem>
#include <vector>

using namespace std;
#define ENABLE_DEBUG_OUTPUT 0  // 设置为1启用调试输出，0禁用
#if ENABLE_DEBUG_OUTPUT
cout << "[DEBUG] 队列文件路径: " << fullPath << endl;
#endif
class QueueManager {
public:
    // 窗口信息结构体
    struct WindowInfo 
    {
        string windowId;      // 窗口唯一标识（如"WIN_001"）
        string assignedStaff; // 关联的员工ID（如"EMP_1001"）
        bool isVipWindow;     // 是否为VIP窗口
        bool isActive;        // 窗口是否激活

        // 明确构造函数参数
        WindowInfo(string wId = "",
            string staff = "",
            bool vip = false,
            bool active = true)
            : windowId(move(wId)),
            assignedStaff(move(staff)),
            isVipWindow(vip),
            isActive(active) {
        }
    };

    // 客户取号
    static bool takeNumber(const string& cardId, bool isVip); // 根据客户卡号和是否VIP进行取号

    // 取消排队
    static bool cancelNumber(const string& cardId); // 客户根据卡号取消排队

    // 查看当前排队信息
    static void viewQueue(const string& cardId); // 查看指定卡号的排队状态

    // 注册新窗口
    static bool registerWindow(const string& windowId, // 窗口ID
        const string& staffId,  // 员工ID
        bool isVipWindow);      // 是否为VIP窗口

    // 激活或关闭窗口
    static bool activateWindow(const string& windowId, bool activate); // 启用/禁用指定窗口

    // 叫号服务
    static void callNextNumber(const string& windowId); // 指定窗口叫下一个号

    // 查看当前服务状态
    static void viewCurrentService(const string& windowId); // 查看当前窗口服务的卡号

    // 清空所有或指定类型队列
    static void clearQueue(bool isVip); // 清空普通或VIP队列

    // 查看所有队列状态
    static void viewAllQueues(); // 打印所有队列信息（调试/管理员功能）

    // 检查用户是否已在队列中
    static bool isUserInQueue(const string& cardId); // 判断某卡号是否正在排队

    // 获取空闲窗口（不加锁，供内部调用）
    static vector<string> getAvailableWindows_NO_LOCK(bool forVip); // 获取当前空闲的普通或VIP窗口（内部）

    // 获取空闲窗口（线程安全）
    static vector<string> getAvailableWindows(bool forVip); // 获取当前空闲窗口，线程安全版本

    // 打印调试状态
    static void debugPrintStatus(); // 打印内部状态信息，调试用
    //重置所有队列
    static void resetAllQueues();
    //解决内存与文件状态不一致的问题
    static void synchronizeQueues();
private:
    static mutex queueMutex; // 队列操作互斥锁，确保线程安全

    static map<string, WindowInfo> windows; // 所有窗口信息，键为窗口ID

    static unordered_map<string, pair<bool, string>> userQueueType;
    // 用户排队信息映射，键为卡号，值为<是否VIP, 当前所在窗口ID或空字符串>

    // 获取队列文件路径
    static string getQueueFilePath(bool isVip, // 是否为VIP队列
        const string& windowId = "");        // 可选窗口ID，用于指定某窗口的临时队列

    // 获取某队列文件中的排队人数
    static size_t getQueueCountForWindow(const string& filename); // 读取文件内容计算队列人数

    // 将队列保存到文件
    static bool saveQueueToFile(bool isVip, // 是否为VIP队列
        const vector<string>& queue,       // 队列内容
        const string& windowId = {});      // 可选窗口ID用于分文件保存
};
