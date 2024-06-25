#include "user_auth.h"

bool isValidUsername(const std::string& username) {
    // Username must not be empty and can only contain alphanumeric characters and underscores
    return (username.length() > 0) &&
           (username.find_first_not_of(VALID_USERNAME_CHARACTER) == std::string::npos);
}

void addUser(const User& user) {
    File users = LittleFS.open("/users.csv", "a");
    Serial.print("Validate username");
    if (!isValidUsername(user.username)) {
        Serial.println(
            "[UserAuth::addUser] Username is not supported. Please only use the following "
            "characters: "
            "\"" VALID_USERNAME_CHARACTER "\"");
        return;
    }
    if (users) {
        users.print(user.username.c_str());
        users.print(",");
        users.print(user.passwordHash.c_str());  // Convert String to const char*
        users.print(",");
        users.print(user.role.c_str());
        users.println();
        users.close();
    } else {
        logFatalln("[UserAuth::addUser] Failed to open file for writing");
    }
}

void removeUser(const std::string& username) {
    // Open a temporary file
    File tempFile = LittleFS.open("/temp.csv", "w");
    File users = LittleFS.open("/users.csv", "r");

    while (users.available()) {
        std::string line = users.readStringUntil('\n').c_str();
        if (line.find((username + ",").c_str()) !=
            0) {  // if the line does not start with the username
            tempFile.println(line.c_str());
        }
    }

    users.close();
    tempFile.close();

    LittleFS.remove("/users.csv");
    LittleFS.rename("/temp.csv", "/users.csv");
}

User searchUser(const std::string& username) {
    // CSV format: username,passwordHash,role
    // Second possible CSV format: username,passwordHash
    // Both formats are supported, no role means UserRole::User

    User user;
    user.username = "";      // Default empty username
    user.passwordHash = "";  // Default empty password hash
    user.role = UserRole::User;

    std::string line;

    const char* filename = "/users.csv";

    logVerboseln(("[UserAuth::searchUser] Searching for user: " + username).c_str());
    File users = LittleFS.open(filename, "r");  // Programm seems to crash at this point
    logVerboseln(("[UserAuth::searchUser] File opened: " + String(filename)).c_str());

    size_t commaIndex1;

    while (users.available()) {
        line = users.readStringUntil('\n').c_str();
        commaIndex1 = line.find(',');
        // logVerboseln(("[UserAuth::searchUser] Parse line: " + line).c_str());
        if (commaIndex1 == std::string::npos) {
            continue;
        }
        user.username = line.substr(0, commaIndex1);
        if (user.username == username) {
            size_t commaIndex2 = line.find(',', commaIndex1 + 1);
            if (commaIndex2 == std::string::npos) {
                logVerboseln(("[UserAuth::searchUser] Parse old format of: " + line).c_str());
                user.passwordHash = line.substr(commaIndex1 + 1, line.length() - commaIndex1 - 1);
                size_t endIndex = user.passwordHash.find('\r');
                user.passwordHash = user.passwordHash.substr(0, endIndex);
                user.role = UserRole::User;
            } else {
                logVerboseln(("[UserAuth::searchUser] Parse new format of: " + line).c_str());
                user.passwordHash = line.substr(commaIndex1 + 1, commaIndex2 - commaIndex1 - 1);
                user.role = line.substr(commaIndex2 + 1, line.length() - commaIndex2 - 1);
                size_t endIndex = user.role.find('\r');
                user.role = user.role.substr(0, endIndex);
            }
            break;
        }

        // Serial log
        /*
        logVerboseln(("Username: " + user.username).c_str());
        logVerboseln(("Password Hash: " + user.passwordHash).c_str());
        logVerboseln(("Role: " + user.role).c_str());
        */
    }

    logVerboseln(("Username: " + user.username).c_str());
    logVerboseln(("Password Hash: " + user.passwordHash).c_str());
    logVerboseln(("Role: " + user.role).c_str());

    users.close();
    // Serial log
    return user;
}

std::string hashPassword(const std::string& password) {
    SHA3_256 sha3;
    sha3.update(password.c_str(), password.length());
    byte hash[32];
    sha3.finalize(hash, sizeof(hash));

    std::string hashedPassword = "";
    for (int i = 0; i < sizeof(hash); i++) {  // Convert byte array to string
        if (hash[i] < 0x10) {
            hashedPassword += "0";
        }
        hashedPassword += std::to_string(hash[i]);
    }

    return hashedPassword;
}

bool checkPasswordHash(const User& user,
                       const std::string& passwordHash) {  // Replace String with std::string
    logVerboseln(
        ("[UserAuth::checkPasswordHash] Checking password hash for user " + user.username).c_str());
    logVerboseln(
        ("[UserAuth::checkPasswordHash]  User password hash: \"" + user.passwordHash + "\"")
            .c_str());
    logVerboseln(
        ("[UserAuth::checkPasswordHash] Input password hash: \"" + passwordHash + "\"").c_str());
    int check = user.passwordHash.compare(passwordHash);
    return (check == 0);
}

bool checkAccess(const std::string& username, const std::string& password) {
    User user = searchUser(username);
    if (user.username == "" || user.passwordHash == "") {
        logAuthln(("[UserAuth::checkAcces] User " + username + " not found").c_str());
        return false;
    }
    if (checkPasswordHash(user, hashPassword(password))) {
        logAuthln(("[UserAuth::checkAcces] User " + username + " authenticated").c_str());
        return true;
    }
    logAuthln(("[UserAuth::checkAcces] User " + username + " typed wrong password").c_str());
    return false;
}

void setupUserAuth() {
    if (!LittleFS.begin(true)) {
        logFatalln("[UserAuth::setupUserAuth] Failed to mount file system");
        return;
    }
    // LittleFS.remove("/users.csv"); // test, remove later
    if (!LittleFS.exists("/users.csv")) {
        File users = LittleFS.open("/users.csv", "w");
        users.close();
    }

    // addUser(User{"admin", hashPassword("admin")}); // test, remove later
}

bool isAdmin(const User& user) {
    logVerboseln(("[UserAuth::isAdmin] User role: \"" + user.role + "\"").c_str());
    logVerboseln(("[UserAuth::isAdmin] Admin role: \"" + UserRole::Admin + "\"").c_str());
    if (user.role.compare(UserRole::Admin) == 0) {
        logVerbose("[UserAuth::isAdmin] User is an admin");
        return true;
    }
    return user.role.compare(UserRole::Admin) == 0;
}

bool checkAdminAccess(const std::string& username, const std::string& password) {
    logAuthln("[UserAuth::checkAdminAccess] begin authentication");
    User user = searchUser(username);

    if (user.username == "" || user.passwordHash == "") {
        logAuthln(("[UserAuth::checkAdminAccess] User " + username + " not found").c_str());
        return false;
    }
    logAuthln(("[UserAuth::checkAdminAccess] User " + username + " found").c_str());
    if (!checkPasswordHash(user, hashPassword(password))) {
        logAuthln(
            ("[UserAuth::checkAdminAccess] User " + username + " typed wrong password").c_str());
        return false;
    }
    logAuthln(("[UserAuth::checkAdminAccess] User " + username + " authenticated").c_str());
    if (!isAdmin(user)) {
        logAuthln(("[UserAuth::checkAdminAccess] User " + username + " is not an admin").c_str());
        return false;
    }
    logAuthln(("[UserAuth::checkAdminAccess] User " + username + " is an admin").c_str());
    return true;
}