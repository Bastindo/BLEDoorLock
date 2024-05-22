#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>

File users;

struct User {
    String username;
    String passwordHash;
};

void addUser(const User& user) {
    users = LittleFS.open("/users.csv", "a");
    if (users) {
        users.print(user.username);
        users.print(",");
        users.println(user.passwordHash);
        users.close();
    } else {
        Serial.println("Failed to open file for writing");
    }
}

void removeUser(const String& username) {
    // Open a temporary file
    File tempFile = LittleFS.open("/temp.csv", "w");
    users = LittleFS.open("/users.csv", "r");

    while (users.available()) {
        String line = users.readStringUntil('\n');
        if (!line.startsWith(username + ",")) {
            tempFile.println(line);
        }
    }

    users.close();
    tempFile.close();

    LittleFS.remove("/users.csv");
    LittleFS.rename("/temp.csv", "/users.csv");
}

User searchUser(const String& username) {
    User user;
    user.username = "";  // Default empty username
    user.passwordHash = "";  // Default empty password hash

    users = LittleFS.open("/users.csv", "r");

    while (users.available()) {
        String line = users.readStringUntil('\n');
        if (line.startsWith(username + ",")) {
            int commaIndex = line.indexOf(',');
            user.username = line.substring(0, commaIndex);
            user.passwordHash = line.substring(commaIndex + 1);
            break;
        }
    }

    users.close();
    return user;
}


bool checkPasswordHash(const String& username, const String& passwordHash) {
    User user = searchUser(username);
    return (user.username == username && user.passwordHash == passwordHash);
}

/*
// Example usage
void test() {
    User newUser = {"john_doe", "hash123"};
    addUser(newUser);

    User foundUser = searchUser("john_doe");
    if (foundUser.username == "") {
        Serial.println("User not found");
    } else {
        Serial.print("Found user: ");
        Serial.println(foundUser.username);
        if (checkPasswordHash(newUser.username, newUser.passwordHash)) {
            Serial.println("Password matches");
        } else {
            Serial.println("Password does not match");
        }
    }
}
*/
