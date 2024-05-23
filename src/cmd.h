#ifndef CMD_H
#define CMD_H

#define CMD_BUFFER_SIZE 256

#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>

#include "ble_server.h"
#include "log.h"
#include "servo.h"
#include "UserAuth.h"

void cmdLoop(bool debugMode);
void setupCmd();

#endif