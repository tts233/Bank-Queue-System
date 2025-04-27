
#pragma once
#include <string>
using namespace std;

class Authenticator {
public:
    static bool authenticateUser(const string& id, const string& pwd);
    static void registerUser(const string& id, const string& pwd);
};