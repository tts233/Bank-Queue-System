#pragma once             // 防止头文件被多重包含
#include <string>       // 引入string类
using namespace std;    // 使用标准命名空间，简化string写法

class Authenticator {
public:
    static bool authenticateUser(const string& id, const string& pwd);
    // 认证用户身份，参数为用户ID和密码，返回认证是否成功

    static void registerUser(const string& id, const string& pwd);
    // 注册新用户，参数为用户ID和密码，写入存储

private:
    static string hashPassword(const string& pwd);
    // 对密码进行哈希处理，返回哈希后的字符串（私有函数）
};
