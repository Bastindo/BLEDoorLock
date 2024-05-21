#include <Arduino.h>

#include "ble_server.h"
#include "log.h"
#include "servo.h"

void setup() {
  Serial.begin(115200);
  while(!Serial && !Serial.available()){}
  setupLog();

  logverbose("Starting up...");

  setupServo();
  setupBLE();
}

void loop() {
  if (Serial.available()) {
    char input = Serial.read();
    if (input == 'o') {
      servoOpen();
    } else if (input == 'c') {
      servoClose();
    }
  }
}