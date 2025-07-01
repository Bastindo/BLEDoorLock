#ifndef CMD_H
#define CMD_H

#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>

// Screw NimBLE
// #include "ble_server.h"

// using BLE.h instead
#include "ble_server2.h"
#include "config.h"
#include "log.h"
#include "opendoor.h"
#include "tcp.h"
#include "user_auth.h"
void cmdLoop(bool debugMode);
void setupCmd();

#endif