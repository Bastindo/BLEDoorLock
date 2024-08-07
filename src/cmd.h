#ifndef CMD_H
#define CMD_H

#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>

#include "config.h"

#include "ble_server.h"
#include "log.h"
#include "opendoor.h"
#include "user_auth.h"

void cmdLoop(bool debugMode);
void setupCmd();

#endif