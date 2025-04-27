#include "MenuManager.h"
#include "Authenticator.h"
#include "QueueManager.h"
#include "LogManager.h"

#ifdef _WIN32
#define CLEAR "cls"
#else
#define CLEAR "clear"
#endif

void MenuManager::showMainMenu() {
    int choice;
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
            string id, pwd;
            cout << "输入新工号："; cin >> id;
            cout << "输入密码："; cin >> pwd;
            Authenticator::registerUser(id, pwd);
            break;
        }
        case 5: return;
        default: cout << "无效选择！\n";
        }
        system("pause");
    }
}

void MenuManager::userInterface() {
    int choice;
    string cardId;
    system(CLEAR);
    cout << "====== 用户端 ======\n请输入六位银行卡号：";
    cin >> cardId;
    while (true) {
        system(CLEAR);
        cout << "1. 普通用户取号\n2. VIP用户取号\n3. 查看等待人数\n4. 取消排队\n5. 返回\n选择：";
        cin >> choice;
        if (choice == 5) break;
        switch (choice) {
        case 1: QueueManager::takeNumber(cardId, false); break;
        case 2: QueueManager::takeNumber(cardId, true); break;
        case 3: QueueManager::viewQueue(cardId); break;
        case 4: QueueManager::cancelNumber(cardId); break;
        }
        system("pause");
    }
}

void MenuManager::staffInterface() {
    string id, pwd;
    cout << "工号："; cin >> id;
    cout << "密码："; cin >> pwd;
    if (Authenticator::authenticateUser(id, pwd)) {
        int choice;
        while (true) {
            system(CLEAR);
            cout << "====== 窗口工作人员端 ======\n1. 叫号\n2. 查看当前服务对象\n3. 返回\n选择：";
            cin >> choice;
            if (choice == 3) break;
            switch (choice) {
            case 1: QueueManager::callNextNumber(id); break;
            case 2: QueueManager::viewCurrentService(id); break;
            }
            system("pause");
        }
    }
    else {
        cout << "认证失败！\n";
    }
}

void MenuManager::adminInterface() {
    string id, pwd;
    cout << "管理员工号："; cin >> id;
    cout << "密码："; cin >> pwd;
    if (Authenticator::authenticateUser(id, pwd)) {
        int choice;
        while (true) {
            system(CLEAR);
            cout << "====== 管理员端 ======\n1. 查看所有排队信息\n2. 清空普通队列\n3. 清空VIP队列\n4. 返回\n选择：";
            cin >> choice;
            if (choice == 4) break;
            switch (choice) {
            case 1: QueueManager::viewAllQueues(); break;
            case 2: QueueManager::clearQueue(false); break;
            case 3: QueueManager::clearQueue(true); break;
            }
            system("pause");
        }
    }
    else {
        cout << "认证失败！\n";
    }
}
