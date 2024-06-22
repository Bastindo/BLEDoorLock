#ifndef CMD_H
#define CMD_H

#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>

#include "ble_server.h"
#include "config.h"
#include "log.h"
#include "servo.h"
#include "user_auth.h"

void cmdLoop(bool debugMode);
void setupCmd();

#endif