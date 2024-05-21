#include <Arduino.h>
#include <ESP32Servo.h>

#include "servo.h"

Servo servo;

void setupServo() {
    servo.setPeriodHertz(50);
    servo.attach(SERVO_PIN, 500, 2500);
    servo.write(0);
}

void setServoAngle(int angle) {
    servo.write(angle);
}

void servoOpen() {
    servo.write(50);
}

void servoClose() {
    servo.write(0);
}