#ifndef USER_AUTH_H
#define USER_AUTH_H
#include <string>
#include <FS.h>
#include <LittleFS.h>
#include <Crypto.h>
#include <SHA3.h>

#include "log.h"

struct User {
    std::string username;
    std::string passwordHash;
};

void addUser(const User& user);
void removeUser(const std::string& username);
User searchUser(const std::string& username);
std::string hashPassword(const std::string& password);
bool checkPasswordHash(const std::string& username, const std::string& passwordHash);
bool checkAccess(const std::string& username, const std::string& password);
void setupUserAuth();

#endif // USER_AUTH_H