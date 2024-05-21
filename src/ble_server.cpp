#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <Arduino.h>

#include "ble_server.h"
#include "servo.h"

class DoorLockCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (value.length() == 0) {
            return;
        }

        if (value[0] == 'o') {
            servoOpen();
            Serial.println("[Servo] Lock Open");
        } else if (value[0] == 'c') {
            servoClose();
            Serial.println("[Servo] Lock Closed");
        }
    }
};

class ServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer *pServer) {
        Serial.println("[BLE Server] Device Connected");
    }

    void onDisconnect(BLEServer *pServer) {
        Serial.println("[BLE Server] Device Disconnected");
    }
};

void setupBLE() {
    BLEDevice::init("Servo Lock");
    BLEServer *pServer = BLEDevice::createServer();
    BLEService *pService = pServer->createService(UUID_SERVICE);
    BLECharacteristic *pCharacteristic = pService->createCharacteristic(
        UUID_CHARACTERISTIC,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE
    );

    pServer->setCallbacks(new ServerCallbacks());

    BLEDescriptor *doorlockDescriptor = new BLEDescriptor((uint16_t)0x2901);
    doorlockDescriptor->setValue("Door Lock");

    pCharacteristic->setValue("hi");
    pCharacteristic->addDescriptor(doorlockDescriptor);
    pCharacteristic->setCallbacks(new DoorLockCallbacks());

    pService->start();

    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->addServiceUUID(UUID_SERVICE);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    pAdvertising->setAppearance(0x0708);    // 0x0708: Door Lock
    pAdvertising->start();

    Serial.println("BLE Ready");
}