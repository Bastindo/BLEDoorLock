#include "cmd.h"

char buffer[CMD_BUFFER_SIZE];
bool commandReceived = false;

char* trimWhitespace(char* str) {
  char* end;
  while(isspace((unsigned char)*str)) str++;
  if(*str == 0)
    return str;
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;
  end[1] = '\0';
  return str;
}

void processCommand(char* cmd, bool debugMode) {
  cmd = trimWhitespace(cmd);
  Serial.println();

  if (strcmp(cmd, "help") == 0 && debugMode) {
    Serial.println("Available commands:");
    Serial.println("  help - Displays this help message");
    Serial.println("  open - Opens the lock");
    Serial.println("  close - Closes the lock");
    Serial.println("  shortopen - Opens the lock for " + String((float) SHORT_UNLOCK_TIME / 1000) + "s");
    Serial.println("  cat <file> - Displays the contents of a file on the LittleFS");
  } 
  else if (strcmp(cmd, "help") == 0 && !debugMode) {
    Serial.println("Available commands:");
    Serial.println("  help - Displays this help message");
    Serial.println("  cat <file> - Displays the contents of a file on the LittleFS");
  }
  else if (strcmp(cmd, "open") == 0 && debugMode) {
    servoOpen();
  } 
  else if (strcmp(cmd, "close") == 0 && debugMode) {
    servoClose();
  } 
  else if (strcmp(cmd, "shortopen") == 0 && debugMode) {
    servoOpen();
    delay(SHORT_UNLOCK_TIME);
    servoClose();
  } 
  else if (strncmp(cmd, "cat ", 4) == 0) {
    char* filename = cmd + 4;
    
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
  }
  else {
    Serial.println("Unknown command: " + String(cmd));
  }
}

void cmdLoop(bool debugMode) {
  static unsigned int idx = 0; // Index for reading characters into the buffer

  while (Serial.available() > 0) {
    char ch = Serial.read();

    if (ch!= '\n' && ch!= '\r') {
      Serial.print(ch);
    }

    if (ch == '\n' || ch == '\r') {
      buffer[idx] = '\0';
      commandReceived = true;
      break;
    } else {
      // Only store printable ASCII characters
      if (ch >= 32 && ch <= 126) {
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