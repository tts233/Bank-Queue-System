#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING // 屏蔽 experimental::filesystem 的弃用警告
#include <experimental/filesystem> // 使用旧版本文件系统库（兼容旧编译器）
namespace fs = std::experimental::filesystem; // 命名空间别名（未实际使用）
#include "MenuManager.h"       // 菜单管理模块（包含主界面交互逻辑）
#include "QueueManager.h"      // 队列管理模块（包含窗口注册/排队等核心逻辑）
#include <iostream>            // 输入输出流
#include <filesystem>          // 标准 C++17 文件系统库
#include <fstream>             // 添加此头文件以支持文件流操作 <--- 这是需要添加的行

int main() {
    // 初始化数据目录 - 不再删除旧文件
    try {
        // 确保数据目录存在（不删除旧数据）
        filesystem::create_directories("data/normal");
        filesystem::create_directories("data/vip");

        // 安全初始化队列文件
        auto initQueueFile = [](const string& filename) {
            ofstream file(filename, ios::app);  // 以追加模式打开
            if (!file.is_open()) {
                throw runtime_error("无法初始化队列文件: " + filename);
            }
            file.close();  // 显式关闭文件
            };

        initQueueFile("data/queue_normal.txt");
        initQueueFile("data/queue_vip.txt");

        // 重置队列状态（不清除文件内容）
        QueueManager::resetAllQueues();
    }
    catch (const exception& e) {
        cerr << "[初始化失败] " << e.what() << "\n";
        return 1;
    }
    // 2. 注册并激活服务窗口（分别是普通窗口和VIP窗口）
    if (!QueueManager::registerWindow("WIN_NORMAL_1", "staff_1", false)) {
        // 注册普通窗口，ID = WIN_NORMAL_1，员工 = staff_1，类型 = 非VIP
        std::cerr << "[错误] 普通窗口注册失败\n";
        return 1;
    }
    if (!QueueManager::registerWindow("WIN_VIP_1", "staff_vip", true)) {
        // 注册 VIP 窗口，ID = WIN_VIP_1，员工 = staff_vip，类型 = VIP
        std::cerr << "[错误] VIP窗口注册失败\n";
        return 1;
    }
    if (!QueueManager::activateWindow("WIN_NORMAL_1", true)) {
        // 激活普通窗口
        std::cerr << "[错误] 普通窗口激活失败\n";
        return 1;
    }
    if (!QueueManager::activateWindow("WIN_VIP_1", true)) {
        // 激活 VIP 窗口
        std::cerr << "[错误] VIP窗口激活失败\n";
        return 1;
    }

    // 3. 启动主界面交互（身份选择菜单）
    try {
        MenuManager::showIdentitySelection(); // 显示身份选择界面（用户 / 员工 / 管理员）
    }
    catch (const std::system_error& e) { // 捕获死锁或线程相关错误
        std::cerr << "[死锁错误] " << e.what() << "\n";
        return 1;
    }
    catch (const std::exception& e) { // 捕获其他运行时异常
        std::cerr << "[运行时错误] " << e.what() << "\n";
        return 1;
    }
    catch (...) { // 捕获所有未知异常
        std::cerr << "[未知错误] 程序异常终止\n";
        return 1;
    }

    return 0; // 程序正常结束
}
