#include "Authenticator.h"
#include <fstream>       // 用于文件输入输出
#include <functional>    // 提供 std::hash 函数模板

// 对密码进行哈希处理，返回哈希值字符串
string Authenticator::hashPassword(const string& pwd) {
    hash<string> hasher;           // 创建用于字符串的哈希函数对象
    return to_string(hasher(pwd)); // 返回密码哈希后的字符串
}

// 验证用户身份是否合法
bool Authenticator::authenticateUser(const string& id, const string& pwd) {
    ifstream fin("data/staff_info.txt"); // 打开用户信息文件以读取模式
    string uid, storedHash;              // uid 表示文件中的用户 ID，storedHash 表示已存储的哈希密码
    string inputHash = hashPassword(pwd); // 对输入密码进行哈希处理，得到 inputHash

    // 遍历文件中的每一行，查找是否有匹配的用户 ID 和密码哈希
    while (fin >> uid >> storedHash) {
        if (uid == id && storedHash == inputHash) {
            return true; // 找到匹配项，认证成功
        }
    }
    return false; // 未找到匹配项，认证失败
}

// 注册新用户，将用户 ID 和密码哈希写入文件
void Authenticator::registerUser(const string& id, const string& pwd) {
    ofstream fout("data/staff_info.txt", ios::app); // 以追加模式打开文件以写入
    fout << id << " " << hashPassword(pwd) << endl; // 写入新用户信息
}
