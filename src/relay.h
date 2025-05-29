#ifndef RELAY_H
#define RELAY_H


#include <Arduino.h>
#include <ESP32Servo.h>
#include <sstream>

#include "config.h"

#include "log.h"

void setupRelay();
void setRelayState(bool state);
void relayOpen();
void relayClose();

#endif