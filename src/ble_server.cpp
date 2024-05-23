#include "ble_server.h"

NimBLECharacteristic *pUserCharacteristic;
NimBLECharacteristic *pPassCharacteristic;
NimBLECharacteristic *pLockStateCharacteristic;

hw_timer_t *lockTimer = NULL;

    void IRAM_ATTR lockTimerISR() {
        servoClose();
        pLockStateCharacteristic->setValue("0");
    }

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
// unnecessary
class UserCharacteristicCallbacks: public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        logVerboseln(("[BLE Server] User Characteristic Value: " + (String)pCharacteristic->getValue().c_str() + value.c_str()).c_str());
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
// end unnecessary

class LockStateCharacteristicCallbacks: public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (value.length() == 0) {
            return;
        }
        std::string user = pUserCharacteristic->getValue();
        std::string pass = pPassCharacteristic->getValue();
        Serial.println(pUserCharacteristic->getValue().c_str());
        
        if (checkAccess(user, pass)) {
            logInfoln("[BLE Server] Authentication successful");
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
                timerAlarmEnable(lockTimer);
                logVerboseln("[BLE Server] Lock short opened");
                break;
            default:
                logErrorln(("[BLE Server] Invalid lock state: " + (String)pCharacteristic->getValue().c_str()).c_str());
                break;
            }
        } else {
            logErrorln("[BLE Server] Authentication failed");
        }
        return;
    };
};

void setupBLE() {
    logInfoln("[BLE Server] Setting up BLE");
    NimBLEDevice::init("NimBLE Servo Lock");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);

    NimBLEDevice::setSecurityAuth(true, true, true);
    NimBLEDevice::setSecurityPasskey(BLE_PIN);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);
    NimBLEServer *pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());
    
    NimBLEService *pService = pServer->createService(UUID_SERVICE);
    pUserCharacteristic = pService->createCharacteristic(
        UUID_USER_CHARACTERISTIC, 
        NIMBLE_PROPERTY::WRITE |
        NIMBLE_PROPERTY::WRITE_ENC |
        NIMBLE_PROPERTY::WRITE_AUTHEN
    );
    pPassCharacteristic = pService->createCharacteristic(
        UUID_PASS_CHARACTERISTIC, 
        NIMBLE_PROPERTY::WRITE |
        NIMBLE_PROPERTY::WRITE_ENC |
        NIMBLE_PROPERTY::WRITE_AUTHEN
    );
    pLockStateCharacteristic = pService->createCharacteristic(
        UUID_LOCKSTATE_CHARACTERISTIC, 
        NIMBLE_PROPERTY::READ | //for getting the lock state
        NIMBLE_PROPERTY::WRITE
    );

    // Short Lock Timer
    lockTimer = timerBegin(0, 16000, true);     // 16 Mhz clock / 16000 -> 1ms timer
    timerAttachInterrupt(lockTimer, &lockTimerISR, true);
    timerAlarmWrite(lockTimer, SHORT_UNLOCK_TIME, false);

    pService->start();
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