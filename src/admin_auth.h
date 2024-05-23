#ifndef ADMIN_AUTH_H
#define ADMIN_AUTH_H

#include <string>
#include <FS.h>
#include <LittleFS.h>
#include <Crypto.h>
#include <SHA3.h>

#include "log.h"

struct Admin {
    std::string username;
    std::string passwordHash;
};

void addAdmin(const Admin& admin);
void removeAdmin(const std::string& username);
Admin searchAdmin(const std::string& username);
std::string hashPassword(const std::string& password);
bool checkAdminPasswordHash(const std::string& username, const std::string& passwordHash);
bool checkAdminAccess(const std::string& username, const std::string& password);
void setupAdminAuth();

#endif // ADMIN_AUTH_H