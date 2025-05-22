#include "MenuManager.h"
#include "Authenticator.h"    // 用户认证模块
#include "QueueManager.h"     // 排队管理模块
#include "LogManager.h"       // 日志管理模块
#include <fstream>

// 根据操作系统选择清屏命令
#ifdef _WIN32
#define CLEAR "cls"           // Windows系统清屏命令
#else
#define CLEAR "clear"         // Linux/macOS清屏命令
#endif

// 显示身份选择菜单
void MenuManager::showIdentitySelection()
{
    int choice; // 用户输入的菜单选项
    while (true) {
        system(CLEAR); // 清屏
        cout << "====== 银行排队系统 ======\n";
        cout << "1. 用户\n";
        cout << "2. 工作人员\n";
        cout << "3. 退出系统\n";
        cout << "请选择您的身份：";
        cin >> choice; // 获取用户选择

        switch (choice) {
        case 1: showUserMenu(); break;   // 用户菜单
        case 2: showStaffMenu(); break;  // 工作人员菜单
        case 3: return;                  // 退出
        default:
            cout << "无效选择！请重新输入\n";
            cin.clear();          // 清除错误状态
            cin.ignore(100, '\n'); // 丢弃错误输入
            system("pause");
        }
    }
}

// 用户菜单
void MenuManager::showUserMenu()
{
    int choice; // 用户菜单选项
    while (true) {
        system(CLEAR);
        cout << "====== 用户菜单 ======\n";
        cout << "1. 用户端\n";
        cout << "2. 返回上级\n";
        cout << "请选择：";
        cin >> choice;

        switch (choice) {
        case 1: userInterface(); break; // 进入用户端
        case 2: return;                 // 返回
        default:
            cout << "无效选择！请重新输入\n";
            cin.clear();          // 清除错误状态
            cin.ignore(100, '\n'); // 丢弃错误输入
            system("pause");
        }
    }
}

// 工作人员菜单（含窗口和管理员）
void MenuManager::showStaffMenu() {
    int choice; // 工作人员菜单选项
    while (true) {
        system(CLEAR);
        cout << "====== 工作人员菜单 ======\n";
        cout << "1. 窗口工作人员\n";
        cout << "2. 管理员端\n";
        cout << "3. 注册员工账号\n";
        cout << "4. 返回上级\n";
        cout << "请选择：";
        cin >> choice;

        switch (choice) {
        case 1: staffInterface(); break; // 窗口工作人员界面
        case 2: adminInterface(); break; // 管理员界面
        case 3: {
            string id, pwd; // 新员工工号与密码
            cout << "输入新工号："; cin >> id;
            cout << "输入密码："; cin >> pwd;
            Authenticator::registerUser(id, pwd); // 调用注册函数
            break;
        }
        case 4: return;
        default:
            cout << "无效选择！请重新输入\n";
            cin.clear();          // 清除错误状态
            cin.ignore(100, '\n'); // 丢弃错误输入
            system("pause");
        }
    }
}

// 主菜单入口（可选入口）
void MenuManager::showMainMenu() {
    std::remove("data/queue_vip.txt");    // 清空VIP队列文件
    std::remove("data/queue_normal.txt"); // 清空普通队列文件
    int choice; // 主菜单选项
    while (true) {
        system(CLEAR);
        cout << "====== 银行排队系统 ======\n";
        cout << "1. 用户端\n2. 窗口工作人员\n3. 管理员端\n4. 注册员工账号\n5. 退出\n请选择：";
        cin >> choice;
        switch (choice) {
        case 1: userInterface(); break;
        case 2: staffInterface(); break;
        case 3: adminInterface(); break;
        case 4: {
            string id, pwd; // 新员工信息
            cout << "输入新工号："; cin >> id;
            cout << "输入密码："; cin >> pwd;
            Authenticator::registerUser(id, pwd);
            break;
        }
        case 5: return;
        default:             cout << "无效选择！请重新输入\n";
            cin.clear();          // 清除错误状态
            cin.ignore(100, '\n'); // 丢弃错误输入
            system("pause");
        }
        system("pause");
    }
}

// 用户端功能逻辑
void MenuManager::userInterface() {
    int choice; // 用户功能选项
    string cardId; // 用户银行卡号
    bool isValid = false; // 验证卡号格式

    // 输入并验证银行卡号
    do {
        system(CLEAR);
        cout << "====== 用户端 ======\n请输入六位银行卡号：";
        cin >> cardId;
        isValid = (cardId.length() == 6);
        if (isValid) {
            for (char c : cardId) {
                if (!isdigit(c)) {
                    isValid = false;
                    break;
                }
            }
        }
        if (!isValid) {
            cout << "输入错误：必须是6位数字！请重新输入\n";
            cin.clear();             // 清除输入错误状态
            cin.ignore(100, '\n');   // 丢弃错误输入
            system("pause");
        }
    } while (!isValid);

    bool hasTakenNumber = QueueManager::isUserInQueue(cardId); // 检查用户是否已排队

    // 用户操作菜单
    while (true) {
        system(CLEAR);
        cout << "====== 用户端 ======\n";
        cout << "1. 普通用户取号\n";
        cout << "2. VIP用户取号\n";
        cout << "3. 查看等待人数\n";
        cout << "4. 取消排队\n";
        cout << "5. 返回\n";
        cout << "选择：";
        cin >> choice;
        if (choice == 5) break;
        switch (choice) {
        case 1:if (QueueManager::takeNumber(cardId, false)) {
            cout << "取号成功！\n";
        }
              else {
            cout << "取号失败，请重试！\n";
        }
              break; // 普通号
        case 2: QueueManager::takeNumber(cardId, true);
            break;  // VIP号
        case 3: QueueManager::viewQueue(cardId);
            break;
        case 4:
            if (QueueManager::cancelNumber(cardId)) {
                hasTakenNumber = false;
                cout << "已成功取消排队！\n";
            } else {
                cout << "取消失败：未找到您的排队记录！\n";
            }
            break;
        }
        system("pause");
    }
}

// 窗口工作人员界面
void MenuManager::staffInterface() {
    string id, pwd;
    cout << "工号：";
    if (!(cin >> id)) {
        cerr << "无效的工号输入\n";
        cin.clear();
        cin.ignore(100, '\n');
        return;
    }

    cout << "密码：";
    if (!(cin >> pwd)) {
        cerr << "无效的密码输入\n";
        cin.clear();
        cin.ignore(100, '\n');
        return;
    }

    if (!Authenticator::authenticateUser(id, pwd)) {
        cerr << "认证失败！请检查工号和密码\n";
        system("pause");
        return;
    }

    int typeChoice;
    cout << "选择窗口类型：\n1. 普通窗口\n2. VIP窗口\n选择：";
    if (!(cin >> typeChoice) || (typeChoice != 1 && typeChoice != 2)) {
        cerr << "无效的选择\n";
        cin.clear();
        cin.ignore(100, '\n');
        return;
    }
    bool isVip = (typeChoice == 2);

    // 生成窗口ID
    string windowId = generateWindowId(id, isVip);
    cout << "系统分配的窗口ID: " << windowId << endl;

    // 注册窗口
    if (!QueueManager::registerWindow(windowId, id, isVip)) {
        cerr << "窗口注册失败！可能原因：\n";
        cerr << "1. 窗口ID冲突\n";
        cerr << "2. 数据目录不可写\n";
        cerr << "3. 无效的窗口ID格式\n";
        system("pause");
        return;
    }

    // 激活窗口
    if (!QueueManager::activateWindow(windowId, true)) {
        cerr << "窗口激活失败！\n";
        system("pause");
        return;
    }

    // 进入窗口操作菜单
    int choice;
    while (true) {
        system(CLEAR);
        cout << "=== 窗口 " << windowId << " ===\n";
        cout << "1. 叫号\n2. 查看状态\n3. 关闭窗口\n选择：";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(100, '\n');
            continue;
        }

        if (choice == 3) {
            QueueManager::activateWindow(windowId, false);
            break;
        }

        switch (choice) {
        case 1: QueueManager::callNextNumber(windowId); break;
        case 2: QueueManager::viewCurrentService(windowId); break;
        default:            cout << "无效选择！请重新输入\n";
            cin.clear();          // 清除错误状态
            cin.ignore(100, '\n'); // 丢弃错误输入
            system("pause");
        }
        system("pause");
    }
}
// 管理员界面
void MenuManager::adminInterface() 
{
    string id, pwd; // 管理员工号与密码
    cout << "管理员工号："; cin >> id;
    cout << "密码："; cin >> pwd;

    if (Authenticator::authenticateUser(id, pwd)) {
        int choice; // 管理员操作选项
        while (true) {
            system(CLEAR);
            cout << "====== 管理员端 ======\n"
                 << "1. 查看所有排队信息\n"
                 << "2. 清空普通队列\n"
                 << "3. 清空VIP队列\n"
                 << "4. 注册VIP窗口\n"
                 << "5. 返回\n选择：";
            cin >> choice;
            switch (choice) 
            {
            case 1: QueueManager::viewAllQueues(); break;
            case 2: QueueManager::clearQueue(false); break; // 清空普通
            case 3: QueueManager::clearQueue(true); break;  // 清空VIP
            case 4: 
            {
                string staffId, windowId; // 注册用变量
                cout << "输入员工工号：";
                cin >> staffId;
                windowId = "WIN_" + staffId + "_" + std::to_string(rand() % 1000); // 随机生成唯一窗口ID
                if (QueueManager::registerWindow(windowId, staffId, true)) {
                    cout << "VIP窗口 " << windowId << " 注册成功！\n";
                } else {
                    cout << "失败：窗口ID冲突或无效输入\n";
                }
                break;
            
            }
            case 5:
                QueueManager::resetAllQueues();
                cout << "所有队列已重置！\n";
                break;
            }
            system("pause");
        }
    } else {
        cout << "认证失败！\n";
    }
}
std::string MenuManager::generateWindowId(const std::string& staffId, bool isVip) {
    // 1. 清洗员工ID，移除非字母数字字符
    std::string cleanStaffId;
    for (char c : staffId) {
        if (isalnum(c)) {  // 只保留字母和数字
            cleanStaffId += toupper(c);  // 统一转为大写
        }
    }

    // 如果清洗后为空，使用默认值
    if (cleanStaffId.empty()) {
        cleanStaffId = "STAFF";
    }

    // 2. 添加全局计数器确保唯一性
    static std::atomic<int> counter(0);  // 线程安全的计数器
    int currentCount = ++counter;

    // 3. 组合最终窗口ID
    return "WIN_" + cleanStaffId + "_" +
        (isVip ? "VIP" : "NOR") + "_" +
        std::to_string(currentCount);
}