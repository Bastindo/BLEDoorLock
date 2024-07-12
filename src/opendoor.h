#ifndef OPENDOOR_H
#define OPENDOOR_H

#include "servo.h"
#include "relay.h"
#include "log.h"
#include "config.h"
#include "ble_server.h"


void setupOpener();
LockState getLockState();
void setLockState(LockState state);

#endif