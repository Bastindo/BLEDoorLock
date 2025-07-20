#ifndef BLE_SERVER2_H
#define BLE_SERVER2_H

#include <ArduinoBLE.h>

#include "config.h"
#include "log.h"
#include "m_crypto.h"
#include "opendoor.h"
#include "user_auth.h"

void setupBLE();
void setLockStateFromBLE(LockState state);
void printBLEaddress();
void onUserWrite(BLEDevice central, BLECharacteristic characteristic);
void onAdminWrite(BLEDevice central, BLECharacteristic characteristic);
void onCryptoWrite(BLEDevice central, BLECharacteristic characteristic);

// Key exchange helper functions
void resetKeyExchangeState();
void resetChunkedReception();
void handleFullData(const uint8_t* data, size_t dataSize,
                    BLECharacteristic& characteristic);
void handleChunkedData(const uint8_t* data, size_t dataSize,
                       BLECharacteristic& characteristic);
void processCompleteRSAKey(BLECharacteristic& characteristic);

#endif