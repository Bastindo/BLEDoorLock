#include "ble_server.h"

#include "opendoor.h"

NimBLECharacteristic *pUserCharacteristic;
NimBLECharacteristic *pPassCharacteristic;
NimBLECharacteristic *pLockStateCharacteristic;
NimBLECharacteristic *pAdminCharacteristic;
NimBLECharacteristic *pAdminPassCharacteristic;
NimBLECharacteristic *pAddUserCharacteristic;
NimBLECharacteristic *pAddPassCharacteristic;
NimBLECharacteristic *pAdminActionCharacteristic;
struct ClientConnection {
    uint16_t conn_handle;
    uint64_t join_time;
    NimBLEServer *pServer;
};

std::queue<ClientConnection> clientQueue;

hw_timer_t *logoutTimer = NULL;

void nextLogoutTimer() {
    if (clientQueue.empty()) {
        Serial.println("No further clients to disconnect");
        return;
    }
    ClientConnection client = clientQueue.front();
    if (client.join_time + LOGOUT_TIME - 100 < timerReadMilis(logoutTimer)) {
        logVerboseln("[BLE Server] Next logout timer already expired");
        popClientConnection();
    } else {
        timerAlarmWrite(logoutTimer, client.join_time + LOGOUT_TIME, false);
        timerAlarmEnable(logoutTimer);
        logVerboseln("[BLE Server] Next logout timer set");
    }
}

void popClientConnection() {
    if (clientQueue.empty()) {
        Serial.println("No clients to disconnect");
        return;
    }
    ClientConnection client = clientQueue.front();

    clientQueue.pop();
    logVerboseln(("[BLE Server] Disconnecting client: " + String(client.conn_handle)).c_str());
    NimBLEServer *server = client.pServer;

    if (server == nullptr) {
        logWarnln("[BLE Server] Server is null");
        return;
    } else {
        logVerboseln("[BLE Server] Server is not null");
        logVerboseln(
            ("[BLE Server] Number of connected devices: " + String(server->getConnectedCount()))
                .c_str());
        server->disconnect(client.conn_handle);
    }
    // NimBLEDevice::getServer()->disconnect(client.conn_handle, 0);
    // NimBLEConnInfo conn = NimBLEDevice::getServer()->getPeerIDInfo(client.conn_handle);
    // logVerboseln(("[BLE Server] Disconnecting client: " + String(conn.getAddress())).c_str());
    logVerboseln("[BLE Server] Disconnected client");
    nextLogoutTimer();
}

void pushClientConnection(uint16_t conn_handle, NimBLEServer *pServer) {
    // use time from logoutTime
    ClientConnection client = {conn_handle, timerReadMilis(logoutTimer), pServer};
    logVerboseln(("[BLE Server] Adding client: " + String(client.conn_handle) + " at " +
                  String(client.join_time))
                     .c_str());
    clientQueue.push(client);
    if (clientQueue.size() == 1) {
        nextLogoutTimer();
    };
}

void logoutTimerISR() {
    Serial.println("Logout timer expired");

    popClientConnection();
}

void setupLogoutTimer() {
    Serial.println("Setting up Logout timer");
    // Short logout Timer
    logoutTimer = timerBegin(1, getCpuFrequencyMhz() * 1000, true);
    timerAttachInterrupt(logoutTimer, &logoutTimerISR, true);
    // timerAlarmWrite(logoutTimer, 2000, false);
}

void delayDisconnectClientDelayed(uint16_t conn_handle, NimBLEServer *pServer) {
    logVerboseln("[BLE Server] Starting logout timer");
    pushClientConnection(conn_handle, pServer);
}

/*
void disconnectClients() {
    std::vector<uint16_t> connections = NimBLEDevice::getServer()->getPeerDevices();
    for (uint16_t conn_handle : connections) {
        logVerboseln(("[BLE Server] Disconnecting client: " + String(conn_handle)).c_str());
        NimBLEDevice::getServer()->disconnect(conn_handle);
    }
    logVerboseln("[BLE Server] Disconnected clients");
}


void IRAM_ATTR logoutTimerISR() {
    Serial.println("Logout timer expired");
    disconnectClients();
}

void setupLogoutTimer() {
    Serial.println("Setting up Logout timer");
    // Short logout Timer
    logoutTimer = timerBegin(1, getCpuFrequencyMhz() * 1000, true);
    timerAttachInterrupt(logoutTimer, &logoutTimerISR, true);
    timerAlarmWrite(logoutTimer, 2000, false);
}

void asyncDisconnectClientDelayed() {
    logVerboseln("[BLE Server] Starting logout timer");
    logVerbose(("[BLE Server] Timer value: " + String(timerAlarmRead(logoutTimer))).c_str());

    timerAlarmEnable(logoutTimer);
    timerWrite(logoutTimer, 0);
}*/

class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer *pServer) {
        logInfoln("[BLE Server] Device connected");
        NimBLEDevice::startAdvertising();
    };

    void onConnect(NimBLEServer *pServer, ble_gap_conn_desc *desc) {
        logVerboseln(("[BLE Server] Client address: " +
                      String(NimBLEAddress(desc->peer_ota_addr).toString().c_str()))
                         .c_str());
        pServer->updateConnParams(desc->conn_handle, 24, 48, 0, 60);
        delayDisconnectClientDelayed(desc->conn_handle, pServer);
        logVerboseln("[BLE Server] Updated Connection Params");
    };

    void onDisconnect(NimBLEServer *pServer) {
        logInfoln("[BLE Server] Device disconnected");
        NimBLEDevice::startAdvertising();
        logVerboseln("[BLE Server] Started advertising after disconnect");
    };
};

class LockStateCharacteristicCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic *pCharacteristic) {
        std::string user = pUserCharacteristic->getValue();
        std::string pass = pPassCharacteristic->getValue();
        std::string lockState = pCharacteristic->getValue();
        // asyncDisconnectClientDelayed();
        if (user.length() == 0 || pass.length() == 0 || lockState.length() == 0) {
            logWarnln("[BLE Server] LockState: Input missing");
            if (DEBUG_MODE) {
                logWarnln(("[BLE Server] Input - User: " + user).c_str());
                logWarnln(("[BLE Server] Input - Pass: " + pass).c_str());
                logWarnln(("[BLE Server] Input - LockState: " + lockState).c_str());
            }
            return;
        }

        if (checkAccess(user, pass)) {
            logInfoln("[BLE Server] Authentication successful");
            logVerboseln(("[BLE Server] Lock State Characteristic Value: " +
                          (String)pCharacteristic->getValue().c_str())
                             .c_str());
            switch (atoi(lockState.c_str())) {
                case 0:
                    setLockState(LOCKED);
                    logVerboseln("[BLE Server] Lock closed");
                    break;
                case 1:
                    setLockState(UNLOCKED);
                    logVerboseln("[BLE Server] Lock opened");
                    break;
                case 2:  // short open
                    setLockState(SHORT_UNLOCK);
                    logVerboseln("[BLE Server] Lock short opened");
                    break;
                default:
                    setLockState(LOCKED);
                    pCharacteristic->setValue("0");
                    logWarnln(("[BLE Server] Invalid lock state: " +
                               (String)pCharacteristic->getValue().c_str())
                                  .c_str());
                    break;
            }
        } else {
            logWarnln("[BLE Server] Authentication failed");
        }
        return;
    };
};

class AdminActionCharacteristicCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic *pCharacteristic) {
        std::string adminName = pAdminCharacteristic->getValue();
        std::string adminpass = pAdminPassCharacteristic->getValue();
        std::string user = pAddUserCharacteristic->getValue();
        std::string userpass = pAddPassCharacteristic->getValue();
        std::string action = pCharacteristic->getValue();
        if (adminName.length() == 0 || adminpass.length() == 0 || user.length() == 0 ||
            action.length() == 0) {
            logWarnln("[BLE Server] Admin: Input missing");
            if (DEBUG_MODE) {
                logWarnln(("[BLE Server] Input - Admin: " + adminName).c_str());
                logWarnln(("[BLE Server] Input - AdminPass: " + adminpass).c_str());
                logWarnln(("[BLE Server] Input - User: " + user).c_str());
                logWarnln(("[BLE Server] Input - UserPass: " + userpass).c_str());
                logWarnln(("[BLE Server] Input - Action: " + action).c_str());
            }
            return;
        }

        if (checkAdminAccess(adminName, adminpass)) {
            logInfoln("[BLE Server] Admin Authentication successful");
            switch (atoi(action.c_str())) {
                case 0:
                    removeUser(user);
                    logInfoln(("[BLE Server] User " + user + " removed").c_str());
                    break;
                case 1:
                    if (userpass.length() == 0) {
                        logWarnln("[BLE Server] Add User: Password missing");
                        return;
                    }
                    addUser({user, hashPassword(userpass), UserRole::User});
                    logInfoln(("[BLE Server] User " + user + " added").c_str());
                    break;
                default:
                    logWarnln(("[BLE Server] Invalid action: " +
                               (String)pCharacteristic->getValue().c_str())
                                  .c_str());
                    break;
            }
        } else {
            logWarnln("[BLE Server] Admin Authentication failed");
        }
        return;
    };
};

void setupBLE() {
    logInfoln("[BLE Server] Setting up BLE");
    NimBLEDevice::init(BLE_NAME);
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
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_ENC | NIMBLE_PROPERTY::WRITE_AUTHEN);
    pPassCharacteristic = pUserService->createCharacteristic(
        UUID_PASS_CHARACTERISTIC,
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_ENC | NIMBLE_PROPERTY::WRITE_AUTHEN);
    pLockStateCharacteristic =
        pUserService->createCharacteristic(UUID_LOCKSTATE_CHARACTERISTIC,
                                           NIMBLE_PROPERTY::READ |  // for getting the lock state
                                               NIMBLE_PROPERTY::WRITE);

    // Admin Service
    NimBLEService *pAdminService = pServer->createService(UUID_ADMIN_SERVICE);
    pAdminCharacteristic = pAdminService->createCharacteristic(
        UUID_ADMIN_CHARACTERISTIC,
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_ENC | NIMBLE_PROPERTY::WRITE_AUTHEN);
    pAdminPassCharacteristic = pAdminService->createCharacteristic(
        UUID_ADMINPASS_CHARACTERISTIC,
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_ENC | NIMBLE_PROPERTY::WRITE_AUTHEN);
    pAddUserCharacteristic = pAdminService->createCharacteristic(
        UUID_ADDUSER_CHARACTERISTIC,
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_ENC | NIMBLE_PROPERTY::WRITE_AUTHEN);
    pAddPassCharacteristic = pAdminService->createCharacteristic(
        UUID_ADDPASS_CHARACTERISTIC,
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_ENC | NIMBLE_PROPERTY::WRITE_AUTHEN);
    pAdminActionCharacteristic = pAdminService->createCharacteristic(
        UUID_ADMINACTION_CHARACTERISTIC, NIMBLE_PROPERTY::WRITE);

    pUserService->start();
    pLockStateCharacteristic->setValue("not initialized");
    pLockStateCharacteristic->setCallbacks(new LockStateCharacteristicCallbacks());
    pAdminService->start();
    pAdminActionCharacteristic->setCallbacks(new AdminActionCharacteristicCallbacks());

    NimBLEAdvertising *pUserAdvertising = NimBLEDevice::getAdvertising();
    pUserAdvertising->addServiceUUID(UUID_USER_SERVICE);
    logVerboseln("[BLE Server] Advertising with User Service UUID: " UUID_USER_SERVICE);
    pUserAdvertising->setScanResponse(true);
    pUserAdvertising->setAppearance(0x0708);  // 0x0708: Door Lock
    pUserAdvertising->start();
    NimBLEAdvertising *pAdminAdvertising = NimBLEDevice::getAdvertising();
    pAdminAdvertising->addServiceUUID(UUID_ADMIN_SERVICE);
    logVerboseln("[BLE Server] Advertising with Admin Service UUID: " UUID_ADMIN_SERVICE);
    pAdminAdvertising->setScanResponse(true);
    pAdminAdvertising->start();
    setupLogoutTimer();
    logInfoln("[BLE Server] BLE Ready");
}

void setLockStateFromBLE(LockState state) {
    switch (state) {
        case LOCKED:
            pLockStateCharacteristic->setValue("0");
            break;
        case UNLOCKED:
            pLockStateCharacteristic->setValue("1");
            break;
        case SHORT_UNLOCK:
            pLockStateCharacteristic->setValue("2");
            break;
        default:
            pLockStateCharacteristic->setValue("0");
            break;
    }
}
