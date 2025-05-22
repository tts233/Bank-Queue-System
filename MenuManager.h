#pragma once
#include <iostream>

#ifdef MENUMANAGER_EXPORTS
#define MENU_API __declspec(dllexport)   // 导出符号（生成DLL时使用）
#else
#define MENU_API __declspec(dllimport)   // 导入符号（使用DLL时使用）
#endif

using namespace std;

class MENU_API MenuManager
{
public:
    static string generateWindowId(const std::string& staffId, bool isVip);//区分同一员工打开的多个窗口
    static void showIdentitySelection();  // 显示身份选择菜单（用户/员工/管理员）
    static void showUserMenu();            // 显示用户菜单界面
    static void showStaffMenu();           // 显示员工菜单界面
    static void showMainMenu();            // 显示主菜单界面
    static void userInterface();           // 用户界面逻辑入口
    static void staffInterface();          // 员工界面逻辑入口
    static void adminInterface();          // 管理员界面逻辑入口
};
