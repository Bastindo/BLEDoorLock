#ifndef OPENDOOR_H
#define OPENDOOR_H

#include "ble_server2.h"
#include "config.h"
#include "log.h"
#include "relay.h"

void setupOpener();
LockState getLockState();
void setLockStateByUser(LockState state, std::string username);
void setLockState(LockState state);

#endif