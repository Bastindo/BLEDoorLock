#include <NimBLEDevice.h>

#include "ble_server.h"
#include "log.h"
#include "servo.h"

class ServerCallbacks: public NimBLEServerCallbacks {
    void onConnect(NimBLEServer *pServer) {
        logInfoln("[BLE Server] Device connected");
        NimBLEDevice::startAdvertising();
    };

    void onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) {
        logVerboseln(("[BLE Server] Client address: " + String(NimBLEAddress(desc->peer_ota_addr).toString().c_str())).c_str());
        pServer->updateConnParams(desc->conn_handle, 24, 48, 0, 60);
    };

    void onDisconnect(NimBLEServer *pServer) {
        logInfoln("[BLE Server] Device disconnected");
        NimBLEDevice::startAdvertising();
    };

    void onMTUChange(uint16_t MTU, ble_gap_conn_desc* desc) {
        char *logstr; 
        snprintf(logstr, 60, "[BLE Server] MTU updated: %u for connection ID: %u\n", MTU, desc->conn_handle);
        logVerboseln(logstr);
    };
};

class CharacteristicCallbacks: public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) {
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
    };
};

static CharacteristicCallbacks chrCallbacks;

void setupBLE() {
    logInfoln("[BLE Server] Setting up BLE");
    NimBLEDevice::init("NimBLE Servo Lock");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);

    NimBLEDevice::setSecurityAuth(true, true, true);
    NimBLEDevice::setSecurityPasskey(123456);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);
    NimBLEServer *pServer = NimBLEDevice::createServer();

    pServer->setCallbacks(new ServerCallbacks());

    NimBLEService *pService = pServer->createService(UUID_SERVICE);
    NimBLECharacteristic *pNonSecureCharacteristic = pService->createCharacteristic(
        "2940", 
        NIMBLE_PROPERTY::READ | 
        NIMBLE_PROPERTY::WRITE
    );
    NimBLECharacteristic *pSecureCharacteristic = pService->createCharacteristic(
        UUID_CHARACTERISTIC, 
        NIMBLE_PROPERTY::READ | 
        NIMBLE_PROPERTY::READ_ENC | 
        NIMBLE_PROPERTY::READ_AUTHEN |
        NIMBLE_PROPERTY::WRITE |
        NIMBLE_PROPERTY::WRITE_ENC |
        NIMBLE_PROPERTY::WRITE_AUTHEN
    );

    pService->start();

    pNonSecureCharacteristic->setValue("idle");
    pNonSecureCharacteristic->setCallbacks(&chrCallbacks);

    pSecureCharacteristic->setValue("idle");
    pSecureCharacteristic->setCallbacks(&chrCallbacks);

    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(UUID_SERVICE);
    logVerboseln(("[BLE Server] Advertising with Service UUID: " + (String)UUID_SERVICE.toString().c_str()).c_str());
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    pAdvertising->setAppearance(0x0708);    // 0x0708: Door Lock
    pAdvertising->start();
    logInfoln("[BLE Server] BLE Ready");
}