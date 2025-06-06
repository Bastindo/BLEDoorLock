#ifndef BLE_SERVER2_H
#define BLE_SERVER2_H

#include <ArduinoBLE.h>

#include "config.h"
#include "log.h"
#include "opendoor.h"
#include "user_auth.h"
#include "m_crypto.h"

void setupBLE();
void setLockStateFromBLE(LockState state);
void printBLEaddress();
void onUserWrite(BLEDevice central, BLECharacteristic characteristic); 
void onAdminWrite(BLEDevice central, BLECharacteristic characteristic); 
void onCryptoWrite(BLEDevice central, BLECharacteristic characteristic);
#endif