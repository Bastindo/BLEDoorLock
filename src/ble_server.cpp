#include "ble_server.h"

NimBLECharacteristic *pUserCharacteristic;
NimBLECharacteristic *pPassCharacteristic;
NimBLECharacteristic *pLockStateCharacteristic;
NimBLECharacteristic *pAdminCharacteristic;
NimBLECharacteristic *pAdminPassCharacteristic;
NimBLECharacteristic *pAddUserCharacteristic;
NimBLECharacteristic *pAddPassCharacteristic;
NimBLECharacteristic *pAdminActionCharacteristic;

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

class LockStateCharacteristicCallbacks: public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) {
        std::string user = pUserCharacteristic->getValue();
        std::string pass = pPassCharacteristic->getValue();
        std::string lockState = pCharacteristic->getValue();
        if (user.length() == 0 || pass.length() == 0 || lockState.length() == 0) {
            logErrorln("[BLE Server] LockState: Input missing");
            return;
        }
        
        if (checkAccess(user, pass)) {
            logInfoln("[BLE Server] Authentication successful");
            logVerboseln(("[BLE Server] Lock State Characteristic Value: " + (String)pCharacteristic->getValue().c_str()).c_str());
            switch (atoi(lockState.c_str())) {
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
                servoClose();
                pCharacteristic->setValue("0");
                logErrorln(("[BLE Server] Invalid lock state: " + (String)pCharacteristic->getValue().c_str()).c_str());
                break;
            }
        } else {
            logErrorln("[BLE Server] Authentication failed");
        }
        return;
    };
};

class AdminActionCharacteristicCallbacks: public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) {
        std::string admin = pAdminCharacteristic->getValue();
        std::string adminpass = pAdminPassCharacteristic->getValue();
        std::string user = pAddUserCharacteristic->getValue();
        std::string userpass = pAddPassCharacteristic->getValue();
        std::string action = pCharacteristic->getValue();
        if (admin.length() == 0 || adminpass.length() == 0 || user.length() == 0 || action.length() == 0) {
            logErrorln("[BLE Server] Admin: Input missing");
            return;
        }
        
        if (checkAdminAccess(admin, adminpass)) {
            logInfoln("[BLE Server] Admin Authentication successful");
            switch (atoi(action.c_str())) {
            case 0:
                removeUser(user);
                logInfoln(("[BLE Server] User " + user + " removed").c_str());
                break;
            case 1:
                if (userpass.length() == 0) {
                    logErrorln("[BLE Server] Add User: Password missing");
                    return;
                }
                addUser({user, hashPassword(userpass)});
                logInfoln(("[BLE Server] User " + user + " added").c_str());
                break;
            default:
                logErrorln(("[BLE Server] Invalid action: " + (String)pCharacteristic->getValue().c_str()).c_str());
                break;
            }
        } else {
            logErrorln("[BLE Server] Admin Authentication failed");
        }
        return;
    };
};

void setupBLE() {
    logInfoln("[BLE Server] Setting up BLE");
    NimBLEDevice::init("NimBLE Servo Lock 2");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);

    NimBLEDevice::setSecurityAuth(true, true, true);
    NimBLEDevice::setSecurityPasskey(BLE_PIN);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);
    NimBLEServer *pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());
    
    // User Service
    NimBLEService *pUserService = pServer->createService(UUID_USER_SERVICE);
    pUserCharacteristic = pUserService->createCharacteristic(
        UUID_USER_CHARACTERISTIC, 
        NIMBLE_PROPERTY::WRITE |
        NIMBLE_PROPERTY::WRITE_ENC |
        NIMBLE_PROPERTY::WRITE_AUTHEN
    );
    pPassCharacteristic = pUserService->createCharacteristic(
        UUID_PASS_CHARACTERISTIC, 
        NIMBLE_PROPERTY::WRITE |
        NIMBLE_PROPERTY::WRITE_ENC |
        NIMBLE_PROPERTY::WRITE_AUTHEN
    );
    pLockStateCharacteristic = pUserService->createCharacteristic(
        UUID_LOCKSTATE_CHARACTERISTIC, 
        NIMBLE_PROPERTY::READ | //for getting the lock state
        NIMBLE_PROPERTY::WRITE
    );

    // Admin Service
    NimBLEService *pAdminService = pServer->createService(UUID_ADMIN_SERVICE);
    pAdminCharacteristic = pAdminService->createCharacteristic(
        UUID_ADMIN_CHARACTERISTIC, 
        NIMBLE_PROPERTY::WRITE |
        NIMBLE_PROPERTY::WRITE_ENC |
        NIMBLE_PROPERTY::WRITE_AUTHEN
    );
    pAdminPassCharacteristic = pAdminService->createCharacteristic(
        UUID_ADMINPASS_CHARACTERISTIC, 
        NIMBLE_PROPERTY::WRITE |
        NIMBLE_PROPERTY::WRITE_ENC |
        NIMBLE_PROPERTY::WRITE_AUTHEN
    );
    pAddUserCharacteristic = pAdminService->createCharacteristic(
        UUID_ADDUSER_CHARACTERISTIC, 
        NIMBLE_PROPERTY::WRITE |
        NIMBLE_PROPERTY::WRITE_ENC |
        NIMBLE_PROPERTY::WRITE_AUTHEN
    );
    pAddPassCharacteristic = pAdminService->createCharacteristic(
        UUID_ADDPASS_CHARACTERISTIC, 
        NIMBLE_PROPERTY::WRITE |
        NIMBLE_PROPERTY::WRITE_ENC |
        NIMBLE_PROPERTY::WRITE_AUTHEN
    );
    pAdminActionCharacteristic = pAdminService->createCharacteristic(
        UUID_ADMINACTION_CHARACTERISTIC, 
        NIMBLE_PROPERTY::WRITE
    );

    // Short Lock Timer
    lockTimer = timerBegin(0, 16000, true);     // 16 Mhz clock / 16000 -> 1ms timer
    timerAttachInterrupt(lockTimer, &lockTimerISR, true);
    timerAlarmWrite(lockTimer, SHORT_UNLOCK_TIME, false);

    pUserService->start();
    pLockStateCharacteristic->setValue("not initialized");
    pLockStateCharacteristic->setCallbacks(new LockStateCharacteristicCallbacks());
    pAdminService->start();
    pAdminActionCharacteristic->setCallbacks(new AdminActionCharacteristicCallbacks());


    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(UUID_USER_SERVICE);
    logVerbose("[BLE Server] Advertising with User Service UUID: ");
    logVerboseln(UUID_USER_SERVICE);
    pAdvertising->addServiceUUID(UUID_ADMIN_SERVICE);
    logVerbose("[BLE Server] Advertising with Admin Service UUID: ");
    logVerboseln(UUID_ADMIN_SERVICE);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    pAdvertising->setAppearance(0x0708);    // 0x0708: Door Lock
    pAdvertising->start();
    logInfoln("[BLE Server] BLE Ready");
}