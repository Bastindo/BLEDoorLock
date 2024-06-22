#ifndef USER_AUTH_H
#define USER_AUTH_H
#include <Crypto.h>
#include <FS.h>
#include <LittleFS.h>
#include <SHA3.h>

#include <string>

#include "log.h"

namespace UserRole {
const std::string User = "user";
const std::string Admin = "admin";
}  // namespace UserRole
struct User {
    std::string username;
    std::string passwordHash;
    std::string role;
};

void addUser(const User& user);
void removeUser(const std::string& username);
User searchUser(const std::string& username);
std::string hashPassword(const std::string& password);
bool checkPasswordHash(const std::string& username, const std::string& passwordHash);
bool checkAccess(const std::string& username, const std::string& password);
bool checkAdminAccess(const std::string& username, const std::string& password);
void setupUserAuth();

#endif  // USER_AUTH_H