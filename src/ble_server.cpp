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

class UserCharacteristicCallbacks: public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (value.length() == 0) {
            return;
        }
        logVerboseln(("[BLE Server] User Characteristic Value: " + (String)pCharacteristic->getValue().c_str()).c_str());
    };
};
class PassCharacteristicCallbacks: public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (value.length() == 0) {
            return;
        }
        logVerboseln(("[BLE Server] Pass Characteristic Value: " + (String)pCharacteristic->getValue().c_str()).c_str());
    };
};
class LockStateCharacteristicCallbacks: public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (value.length() == 0) {
            return;
        }
        logVerboseln(("[BLE Server] Lock State Characteristic Value: " + (String)pCharacteristic->getValue().c_str()).c_str());
        switch (atoi(value.c_str())) {
        case 0:
            servoClose();
            logVerboseln("[BLE Server] Lock closed");
            break;
        case 1:
            servoOpen();
            logVerboseln("[BLE Server] Lock opened");
            break;
        case 2: // short open
            servoOpen();
            delay(10000);
            servoClose();
            pCharacteristic->setValue("0");
            logVerboseln("[BLE Server] Lock short opened and closed");
            break;
        default:
            logErrorln(("[BLE Server] Invalid lock state: " + (String)pCharacteristic->getValue().c_str()).c_str());
            break;
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
    /*old
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
    );*/
    //new
    NimBLECharacteristic *pUserCharacteristic = pService->createCharacteristic(
        UUID_USER_CHARACTERISTIC, 
        NIMBLE_PROPERTY::WRITE |
        NIMBLE_PROPERTY::WRITE_ENC |
        NIMBLE_PROPERTY::WRITE_AUTHEN
    );
    NimBLECharacteristic *pPassCharacteristic = pService->createCharacteristic(
        UUID_PASS_CHARACTERISTIC, 
        NIMBLE_PROPERTY::WRITE |
        NIMBLE_PROPERTY::WRITE_ENC |
        NIMBLE_PROPERTY::WRITE_AUTHEN
    );
    NimBLECharacteristic *pLockStateCharacteristic = pService->createCharacteristic(
        UUID_LOCKSTATE_CHARACTERISTIC, 
        NIMBLE_PROPERTY::READ | //for getting the lock state
        NIMBLE_PROPERTY::WRITE
    );



    pService->start();
    /*old
    pNonSecureCharacteristic->setValue("idle");
    pNonSecureCharacteristic->setCallbacks(&chrCallbacks);

    pSecureCharacteristic->setValue("idle");
    pSecureCharacteristic->setCallbacks(&chrCallbacks);
    */
    //new
    pUserCharacteristic->setCallbacks(new UserCharacteristicCallbacks());
    pPassCharacteristic->setCallbacks(new PassCharacteristicCallbacks());
    pLockStateCharacteristic->setValue("not initialized");
    pLockStateCharacteristic->setCallbacks(new LockStateCharacteristicCallbacks());

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