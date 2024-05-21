#include <Arduino.h>
#include <ESP32Servo.h>

#include "servo.h"

Servo servo;

void setupServo() {
    servo.setPeriodHertz(50);
    servo.attach(SERVO_PIN, 500, 2500);
    servo.write(0);
    Serial.println("[Servo] Setup Done");
}

void setServoAngle(int angle) {
    servo.write(angle);
}

void servoOpen() {
    servo.write(50);
    Serial.println("[Servo] Lock Open");
}

void servoClose() {
    servo.write(0);
    Serial.println("[Servo] Lock Closed");
}