#include "cmd.h"
#include "wifi_client.h"

char buffer[CMD_BUFFER_SIZE];
bool commandReceived = false;

char *trimWhitespace(char *str) {
  char *end;
  while (isspace((unsigned char)*str))
    str++;
  if (*str == 0)
    return str;
  end = str + strlen(str) - 1;
  while (end > str && isspace((unsigned char)*end))
    end--;
  end[1] = '\0';
  return str;
}

void processCommand(char *cmd, bool debugMode) {
  cmd = trimWhitespace(cmd);
  Serial.println();

  if (strcmp(cmd, "help") == 0 && debugMode) {
    Serial.println("Available commands [Debug mode]:");
    Serial.println("  help - Displays this help message");
    Serial.println("  open - Opens the lock");
    Serial.println("  close - Closes the lock");
    Serial.println("  shortopen - Opens the lock for " +
                   String((float)SHORT_UNLOCK_TIME / 1000) + "s");
    Serial.println(
        "  cat <file> - Displays the contents of a file on the LittleFS");
    Serial.println("  ls - Lists the files on the LittleFS");
    Serial.println(
        "  adduser <username> <password> - Adds a user to the user database");
    Serial.println(
        "  removeuser <username> - Removes a user from the user database");
    Serial.println("  addadmin <username> <password> - Adds an admin account "
                   "to the admin database");
    Serial.println("  removeadmin <username> - Removes an admin account from "
                   "the admin database");
    Serial.println("  hashpassword <password> - Hashes a password");
    Serial.println("  sysinfo - Displays system information");
    Serial.println("  reboot - Reboots the device");
    Serial.println("  rm - Removes a file from the LittleFS");
    Serial.println("  tcp - sends \"test\" to the defined target");
    Serial.println("  ip - Returns the IP Configuration");
  } else if (strcmp(cmd, "help") == 0 && !debugMode) {
    Serial.println("Available commands:");
    Serial.println("  help - Displays this help message");
    Serial.println(
        "  cat <file> - Displays the contents of a file on the LittleFS");
    Serial.println("  ls - Lists the files on the LittleFS");
    Serial.println(
        "  adduser <username> <password> - Adds a user to the user database");
    Serial.println(
        "  removeuser <username> - Removes a user from the user database");
    Serial.println("  addadmin <username> <password> - Adds an admin account "
                   "to the admin database");
    Serial.println("  removeadmin <username> - Removes an admin account from "
                   "the admin database");
    Serial.println("  hashpassword <password> - Hashes a password");
    Serial.println("  sysinfo - Displays system information");
    Serial.println("  reboot - Reboots the device");
    Serial.println("  rm - Removes a file from the LittleFS");
    Serial.println("  ping - Returns 'pong'");
    Serial.println("  tcp - sends \"test\" to the defined target");
    Serial.println("  ip - Returns the IP Configuration");
  } else if (strcmp(cmd, "open") == 0 && debugMode) {
    setLockState(UNLOCKED);
  } else if (strcmp(cmd, "close") == 0 && debugMode) {
    setLockState(LOCKED);
  } else if (strcmp(cmd, "shortopen") == 0 && debugMode) {
    setLockState(SHORT_UNLOCK);
  } else if (strncmp(cmd, "cat ", 4) == 0) {
    char *filename = cmd + 4;

    // add "/" to filename if it doesn't start with it
    if (filename[0] != '/') {
      char temp[CMD_BUFFER_SIZE];
      strcpy(temp, "/");
      strcat(temp, filename);
      strcpy(filename, temp);
    }

    File file = LittleFS.open(filename, "r");
    if (!file) {
      Serial.println("Failed to open file: " + String(filename));
    } else {
      while (file.available()) {
        Serial.write(file.read());
      }
      file.close();
    }
  } else if (strncmp(cmd, "adduser ", 8) == 0) {
    char *username = strtok(cmd + 8, " ");
    char *password = strtok(NULL, " ");
    addUser({username, hashPassword(password), UserRole::User});
  } else if (strncmp(cmd, "removeuser ", 11) == 0) {
    char *username = cmd + 11;
    removeUser(username);
  } else if (strncmp(cmd, "hashpassword ", 13) == 0) {
    char *password = cmd + 13;
    Serial.println(hashPassword(password).c_str());
  } else if (strcmp(cmd, "sysinfo") == 0) {
    Serial.println("System information:");
    Serial.println("  Chip");
    Serial.println("  ├ Chip model: " + String(ESP.getChipModel()));
    Serial.println("  ├ Chip ID: " + String(ESP.getEfuseMac()));
    Serial.println("  ├ Chip revision: " + String(ESP.getChipRevision()));
    Serial.println("  └ CPU frequency: " + String(ESP.getCpuFreqMHz()));
    Serial.println("  Heap");
    Serial.println("  └ Free heap: " + String(ESP.getFreeHeap()));
    Serial.println("  Flash");
    Serial.println("  ├ Flash chip size: " + String(ESP.getFlashChipSize()));
    Serial.println("  └ Flash chip speed: " + String(ESP.getFlashChipSpeed()));
    Serial.println("  Misc");
    Serial.println("  └ SDK version: " + String(ESP.getSdkVersion()));
  } else if (strcmp(cmd, "reboot") == 0) {
    ESP.restart();
  } else if (strncmp(cmd, "addadmin ", 9) == 0) {
    char *username = strtok(cmd + 9, " ");
    char *password = strtok(NULL, " ");
    addUser({username, hashPassword(password), UserRole::Admin});
  } else if (strncmp(cmd, "removeadmin ", 12) == 0) {
    char *username = cmd + 12;
    removeUser(username);
  } else if (strcmp(cmd, "ls") == 0) {
    File root = LittleFS.open("/");
    if (!root || !root.isDirectory()) {
      Serial.println("Failed to open root directory");
      return;
    }

    File entry = root.openNextFile();
    while (entry) {
      Serial.println(entry.name());
      entry.close();
      entry = root.openNextFile();
    }
    root.close();
  } else if (strncmp(cmd, "rm ", 3) == 0) {
    char *filename = cmd + 3;

    // add "/" to filename if it doesn't start with it
    if (filename[0] != '/') {
      char temp[CMD_BUFFER_SIZE];
      strcpy(temp, "/");
      strcat(temp, filename);
      strcpy(filename, temp);
    }

    if (LittleFS.remove(filename)) {
      Serial.println("Removed file: " + String(filename));
    } else {
      Serial.println("Failed to remove file: " + String(filename));
    }
  } else if (strcmp(cmd, "tcp") == 0) {
    tcpWrite("test");
  } else if (strcmp(cmd, "ping") == 0) {
    Serial.println("pong");
  } else if (strcmp(cmd, "ip") == 0) {
    printWiFiInfo();
  } else if (strcmp(cmd, "ble") == 0) {
    printBLEaddress();
  } else if (strcmp(cmd, "") == 0) {
    // Do nothing
  } else {
    Serial.println("Unknown command: " + String(cmd));
  }
}

void cmdLoop(bool debugMode) {
  static unsigned int idx = 0; // Index for reading characters into the buffer

  while (Serial.available() > 0) {
    char ch = Serial.read();

    if (ch == 8 || ch == 127) { // ASCII values for backspace
      if (idx > 0) {
        idx--;
        Serial.print(
            "\b \b"); // Move cursor back, overwrite with space, move back again
      }
    } else if (ch != '\n' && ch != '\r') {
      Serial.print(ch);
    }

    if (ch == '\n' || ch == '\r') {
      buffer[idx] = '\0';
      commandReceived = true;
      break;
    } else {
      // Only store printable ANSI characters
      if (ch >= 32 && ch <= 254) {
        buffer[idx++] = ch;
      }
    }
  }

  if (commandReceived) {
    processCommand(buffer, debugMode);
    idx = 0;
    commandReceived = false;
    Serial.print("> ");
  }
}

void setupCmd() {
  Serial.println("Type 'help' for a list of commands");
  Serial.print("> ");
}
