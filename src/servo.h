#ifndef SERVO_H
#define SERVO_H

#include <Arduino.h>
#include <ESP32Servo.h>

#include "config.h"

#include "log.h"

void setupServo();
void setServoAngle(int angle);
void servoOpen();
void servoClose();

#endif