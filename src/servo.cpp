#include "servo.h"

Servo servo;

void setupServo() {
    servo.setPeriodHertz(50);
    servo.attach(SERVO_PIN, 500, 2500);
    servo.write(0);
    logVerboseln(("[Servo] Servo attached on pin " + String(SERVO_PIN)).c_str());
    logInfoln("[Servo] Setup Done");
}

void setServoAngle(int angle) {
    servo.write(angle);
}

void servoOpen() {
    servo.write(50);
    logInfoln("[Servo] Lock Open");
}

void servoClose() {
    servo.write(0);
    logInfoln("[Servo] Lock Closed");
}