#include <Arduino.h>

#include "ble_server.h"
#include "cmd.h"
#include "log.h"
#include "servo.h"
#include "UserAuth.h"

// debug mode waits for serial and adds more commands
#define DEBUG_MODE 1

void setup() {
  Serial.begin(115200);

  #if DEBUG_MODE == 1
    while(!Serial && !Serial.available()){}
  #endif

  setupLog();

  logInfoln("Starting up...");

  setupServo();
  setupBLE();
  setupUserAuth();
  setupCmd();
}

void loop() {
  cmdLoop(DEBUG_MODE);
}