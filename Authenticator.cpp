#include "Authenticator.h"
#include <fstream>

bool Authenticator::authenticateUser(const string& id, const string& pwd) {
    ifstream fin("data/staff_info.txt");
    string uid, upwd;
    while (fin >> uid >> upwd) {
        if (uid == id && upwd == pwd) return true;
    }
    return false;
}

void Authenticator::registerUser(const string& id, const string& pwd) {
    ofstream fout("data/staff_info.txt", ios::app);
    fout << id << " " << pwd << endl;
}
