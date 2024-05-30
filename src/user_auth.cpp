#include "user_auth.h"

void addUser(const User& user) {
    File users = LittleFS.open("/users.csv", "a");
    if (users) {
        users.print(user.username.c_str());
        users.print(",");
        users.println(user.passwordHash.c_str()); // Convert String to const char*
        users.close();
    } else {
        Serial.println("Failed to open file for writing");
    }
}

void removeUser(const std::string& username) {
    // Open a temporary file
    File tempFile = LittleFS.open("/temp.csv", "w");
    File users = LittleFS.open("/users.csv", "r");

    while (users.available()) {
        std::string line = users.readStringUntil('\n').c_str();
        if (line.find((username + ",").c_str()) != 0) { // if the line does not start with the username
            tempFile.println(line.c_str());
        }
    }

    users.close();
    tempFile.close();

    LittleFS.remove("/users.csv");
    LittleFS.rename("/temp.csv", "/users.csv");
}

User searchUser(const std::string& username) {
    User user;
    user.username = "";  // Default empty username
    user.passwordHash = "";  // Default empty password hash

    File users = LittleFS.open("/users.csv", "r");

    while (users.available()) {
        std::string line = users.readStringUntil('\n').c_str();
        Serial.println(line.c_str());
        if (line.find((username + ",").c_str()) == 0) { // if the line starts with the username
            size_t commaIndex = line.find(',');
            user.username = line.substr(0, commaIndex);
            user.passwordHash = line.substr(commaIndex + 1,line.length()-commaIndex-2); // remove whitespace
            /*Serial.print("Found user: ");
            Serial.print(user.username.c_str());
            Serial.println(", ");
            Serial.println(user.passwordHash.c_str());*/
            break;
        }
    }

    users.close();
    return user;
}

std::string hashPassword(const std::string& password) {
    SHA3_256 sha3;
    sha3.update(password.c_str(), password.length());
    byte hash[32];
    sha3.finalize(hash, sizeof(hash));

    std::string hashedPassword = "";
    for (int i = 0; i < sizeof(hash); i++) {    // Convert byte array to string
        if (hash[i] < 0x10) {
            hashedPassword += "0";
        }
        hashedPassword += std::to_string(hash[i]);
    }
    /*Serial.print("Password: ");
    Serial.println(password.c_str());
    Serial.print("Hashed password: ");
    Serial.println(hashedPassword.c_str());*/

    return hashedPassword;
}

bool checkPasswordHash(const std::string& username, const std::string& passwordHash) { // Replace String with std::string
    User user = searchUser(username);
    logVerboseln(("[UserAuth] Checking password hash for user " + username).c_str());
    logVerboseln(("[UserAuth] User password hash: " + user.passwordHash).c_str());
    logVerboseln(("[UserAuth] Input password hash: " + passwordHash).c_str());
    int check = user.passwordHash.compare(passwordHash);
    return (check==0);
}

bool checkAccess(const std::string& username, const std::string& password) {
    User user = searchUser(username);
    if (user.username == "" || user.passwordHash == "") {
        logErrorln(("[UserAuth] User " + username + " not found").c_str());
        return false;
    }
    if (checkPasswordHash(username, hashPassword(password))) {
        logErrorln(("[UserAuth] User " + username + " authenticated").c_str());
        return true;
    }
    logErrorln(("[UserAuth] User " + username + " typed wrong password: " + password).c_str());
    return false;
}

void setupUserAuth() {
    if (!LittleFS.begin(true)) {
        logErrorln("Failed to mount file system");
        return;
    }
    //LittleFS.remove("/users.csv"); // test, remove later
    if (!LittleFS.exists("/users.csv")) {
        File users = LittleFS.open("/users.csv", "w");
        users.close();
    }
    
    //addUser(User{"admin", hashPassword("admin")}); // test, remove later
}
