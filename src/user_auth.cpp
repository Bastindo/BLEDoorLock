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
        logFatalln(
            "[UserAuth] Username is not supported. Please only use the following characters: "
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
        logFatalln("[UserAuth] Failed to open file for writing");
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

    File users = LittleFS.open("/users.csv", "r");

    std::string line;
    size_t commaIndex1;

    while (users.available()) {
        commaIndex1 = line.find(',');
        line = users.readStringUntil('\n').c_str();
        if (commaIndex1 == std::string::npos) {
            continue;
        }
        user.username = line.substr(0, commaIndex1 - 1);
        if (user.username == username) {
            size_t commaIndex2 = line.find(',', commaIndex1 + 1);
            if (commaIndex2 == std::string::npos) {
                Serial.println("Parse old format of: ");
                Serial.println(line.c_str());
                user.passwordHash = line.substr(commaIndex1, line.length() - commaIndex1 - 6);
                user.role = UserRole::User;
            } else {
                Serial.println("Parse new format");
                Serial.println(line.c_str());
                user.passwordHash = line.substr(commaIndex1, commaIndex2 - commaIndex1);
                user.role = line.substr(commaIndex2 + 1, line.length() - commaIndex2 - 1);
            }
            break;
        }

        // Serial log
        Serial.print("Username: ");
        Serial.println(user.username.c_str());
        Serial.print("Password Hash: ");
        Serial.println(user.passwordHash.c_str());
        Serial.print("Role: ");
        Serial.println(user.role.c_str());
        // if (line.find((username + ",").c_str()) == 0) {  // if the line starts with the username
        //     size_t commaIndex = line.find(',');
        //     user.username = line.substr(0, commaIndex);
        //     user.passwordHash =
        //         line.substr(commaIndex + 1, line.length() - commaIndex - 2);  // remove
        //         whitespace
        //     break;
        // }
    }

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
    logVerboseln(("[UserAuth] Checking password hash for user " + user.username).c_str());
    logVerboseln(("[UserAuth]  User password hash: \"" + user.passwordHash + "\"").c_str());
    logVerboseln(("[UserAuth] Input password hash: \"" + passwordHash + "\"").c_str());
    int check = user.passwordHash.compare(passwordHash);
    return (check == 0);
}

bool checkAccess(const std::string& username, const std::string& password) {
    User user = searchUser(username);
    if (user.username == "" || user.passwordHash == "") {
        logAuthln(("[UserAuth] User " + username + " not found").c_str());
        return false;
    }
    Serial.println("Test OwO1");
    if (checkPasswordHash(user, hashPassword(password))) {
        logAuthln(("[UserAuth] User " + username + " authenticated").c_str());
        return true;
    }
    Serial.println("Test OwO2");
    logAuthln(("[UserAuth] User " + username + " typed wrong password").c_str());
    Serial.println("Test OwO3");
    return false;
}

void setupUserAuth() {
    if (!LittleFS.begin(true)) {
        logFatalln("[UserAuth] Failed to mount file system");
        return;
    }
    // LittleFS.remove("/users.csv"); // test, remove later
    if (!LittleFS.exists("/users.csv")) {
        File users = LittleFS.open("/users.csv", "w");
        users.close();
    }

    // addUser(User{"admin", hashPassword("admin")}); // test, remove later
}

bool checkAdminAccess(const std::string& username, const std::string& password) {
    User user = searchUser(username);
    if (user.username == "" || user.passwordHash == "") {
        logAuthln(("[UserAuth] Admin " + username + " not found").c_str());
        return false;
    }
    if (checkPasswordHash(user, hashPassword(password)) && user.role == UserRole::Admin) {
        logAuthln(("[UserAuth] Admin " + username + " authenticated").c_str());
        return true;
    }
    logAuthln(("[UserAuth] Admin " + username + " typed wrong password: " + password).c_str());
    return false;
}
