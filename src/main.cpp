#include <Arduino.h>
#include <ESP32Servo.h>

#include "servo.h"
#include "ble_server.h"

void setup() {
  Serial.begin(115200);
  setupServo();
  setupBLE();
}

void loop() {
  if (Serial.available()) {
    char input = Serial.read();
    if (input == 'o') {
      servoOpen();
      Serial.println("Lock Open");
    } else if (input == 'c') {
      servoClose();
      Serial.println("Lock Closed");
    }
  }
}