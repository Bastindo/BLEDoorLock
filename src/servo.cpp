#include "servo.h"

Servo servo;

void setupServo() {
  servo.setPeriodHertz(50);
  servo.attach(SERVO_PIN, 500, 2500);
  servo.write(SERVO_CLOSE_ANGLE);
  logVerboseln(("[Servo] Servo attached on pin " + String(SERVO_PIN)).c_str());
  logInfoln("[Servo] Setup Done");
}

void setServoAngle(int angle) { servo.write(angle); }

void servoOpen() {
  servo.write(SERVO_OPEN_ANGLE);
  logInfoln("[Servo] Lock Open");
}

void servoClose() {
  servo.write(SERVO_CLOSE_ANGLE);
  logInfoln("[Servo] Lock Closed");
}