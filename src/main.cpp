#include <Arduino.h>

#include "ble_server.h"
#include "log.h"
#include "servo.h"

// waits for serial and enables servo operations via serial input
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
}

void loop() {
  #if DEBUG_MODE == 1
    if (Serial.available()) {
      char input = Serial.read();
      if (input == 'o') {
        servoOpen();
      } else if (input == 'c') {
        servoClose();
      }
    }
  #endif
}