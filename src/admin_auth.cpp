#include "admin_auth.h"

void addAdmin(const Admin& admin) {
    File admins = LittleFS.open("/admins.csv", "a");
    if (admins) {
        admins.print(admin.username.c_str());
        admins.print(",");
        admins.println(admin.passwordHash.c_str()); // Convert String to const char*
        admins.close();
    } else {
        logErrorln("[AdminAuth] Failed to open file for writing");
    }
}

void removeAdmin(const std::string& username) {
    // Open a temporary file
    File tempFile = LittleFS.open("/temp.csv", "w");
    File admins = LittleFS.open("/admins.csv", "r");

    while (admins.available()) {
        std::string line = admins.readStringUntil('\n').c_str();
        if (line.find((username + ",").c_str()) != 0) { // if the line does not start with the username
            tempFile.println(line.c_str());
        }
    }

    admins.close();
    tempFile.close();

    LittleFS.remove("/admins.csv");
    LittleFS.rename("/temp.csv", "/admins.csv");
}

Admin searchAdmin(const std::string& username) {
    Admin admin;
    admin.username = "";  // Default empty username
    admin.passwordHash = "";  // Default empty password hash

    File admins = LittleFS.open("/admins.csv", "r");

    while (admins.available()) {
        std::string line = admins.readStringUntil('\n').c_str();
        if (line.find((username + ",").c_str()) == 0) { // if the line starts with the username
            size_t commaIndex = line.find(',');
            admin.username = line.substr(0, commaIndex);
            admin.passwordHash = line.substr(commaIndex + 1,line.length()-commaIndex-2); // remove whitespace
            logVerboseln(("Found admin: " + admin.username).c_str());
            logVerboseln(("Password hash: " + admin.passwordHash).c_str());
            break;
        }
    }

    admins.close();
    return admin;
}

bool checkAdminPasswordHash(const std::string& username, const std::string& passwordHash) { // Replace String with std::string
    Admin admin = searchAdmin(username);
    logVerboseln(("[AdminAuth] Checking password hash for admin " + username).c_str());
    logVerboseln(("[AdminAuth] Admin password hash: " + admin.passwordHash).c_str());
    logVerboseln(("[AdminAuth] Input password hash: " + passwordHash).c_str());
    int check = admin.passwordHash.compare(passwordHash);
    return (check==0);
}

bool checkAdminAccess(const std::string& username, const std::string& password) {
    Admin admin = searchAdmin(username);
    if (admin.username == "" || admin.passwordHash == "") {
        logErrorln(("[AdminAuth] Admin " + username + " not found").c_str());
        return false;
    }
    if (checkAdminPasswordHash(username, hashPassword(password))) {
        logErrorln(("[AdminAuth] Admin " + username + " authenticated").c_str());
        return true;
    }
    logErrorln(("[AdminAuth] Admin " + username + " typed wrong password: " + password).c_str());
    return false;
}

void setupAdminAuth() {
    if (!LittleFS.begin(true)) {
        logErrorln("Failed to mount file system");
        return;
    }
    //LittleFS.remove("/admins.csv"); // test, remove later
    if (!LittleFS.exists("/admins.csv")) {
        File admins = LittleFS.open("/admins.csv", "w");
        admins.close();
    }
    
    //addAdmin(Admin{"admin", hashPassword("admin")}); // test, remove later
}
