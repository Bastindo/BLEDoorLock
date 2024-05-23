#ifndef SERVO_H
#define SERVO_H

#define SERVO_PIN 2

#include <Arduino.h>
#include <ESP32Servo.h>

#include "log.h"

void setupServo();
void setServoAngle(int angle);
void servoOpen();
void servoClose();

#endif