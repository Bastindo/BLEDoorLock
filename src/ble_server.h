#ifndef BLE_SERVER_H
#define BLE_SERVER_H

#include <NimBLEDevice.h>

#include <queue>

#include "config.h"
#include "log.h"
#include "opendoor.h"
#include "user_auth.h"

void setupBLE();
void setLockStateFromBLE(LockState state);

#endif