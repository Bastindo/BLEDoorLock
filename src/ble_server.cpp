#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include "ble_server.h"
#include "log.h"
#include "servo.h"

class DoorLockCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (value.length() == 0) {
            return;
        }

        if (value[0] == 'o') {
            servoOpen();
            logVerboseln(("[BLE Server] Characteristic Value: " + (String)pCharacteristic->getValue().c_str()).c_str());
            pCharacteristic->setValue("idle");
            logVerboseln(("[BLE Server] Characteristic Value: " + (String)pCharacteristic->getValue().c_str()).c_str());

        } else if (value[0] == 'c') {
            servoClose();
            logVerboseln(("[BLE Server] Characteristic Value: " + (String)pCharacteristic->getValue().c_str()).c_str());
            pCharacteristic->setValue("idle");
            logVerboseln(("[BLE Server] Characteristic Value: " + (String)pCharacteristic->getValue().c_str()).c_str());
        }
    }
};

class ServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer *pServer) {
        logInfoln("[BLE Server] Device Connected");
    }

    void onDisconnect(BLEServer *pServer) {
        logInfoln("[BLE Server] Device Disconnected");
    }
};

void setupBLE() {
    BLEDevice::init("Servo Lock");
    logVerboseln("[BLE Server] Initialized as Servo Lock");
    BLEServer *pServer = BLEDevice::createServer();
    BLEService *pService = pServer->createService(UUID_SERVICE);
    BLECharacteristic *pCharacteristic = pService->createCharacteristic(
        UUID_CHARACTERISTIC,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE
    );
    logVerboseln("[BLE Server] Created Characteristic");

    pServer->setCallbacks(new ServerCallbacks());

    BLEDescriptor *doorlockDescriptor = new BLEDescriptor((uint16_t)0x2901);
    doorlockDescriptor->setValue("Door Lock");
    logVerboseln("[BLE Server] Added door lock descriptor");

    pCharacteristic->setValue("idle");
    logVerboseln(("[BLE Server] Characteristic Value: " + (String)pCharacteristic->getValue().c_str()).c_str());

    pCharacteristic->addDescriptor(doorlockDescriptor);
    pCharacteristic->setCallbacks(new DoorLockCallbacks());

    pService->start();

    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->addServiceUUID(UUID_SERVICE);
    logVerboseln(("[BLE Server] Advertising with Service UUID: " + (String)UUID_SERVICE.toString().c_str()).c_str());

    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    pAdvertising->setAppearance(0x0708);    // 0x0708: Door Lock
    pAdvertising->start();
    logVerboseln("[BLE Server] Advertising Started (Appearance: 0x0708 - Door Lock)");

    logInfoln("[BLE Server] BLE Ready");
}