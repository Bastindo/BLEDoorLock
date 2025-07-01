#include "ble_server2.h"
#include "m_crypto.h"

// User login + Lockstate
BLEService UserService(UUID_USER_SERVICE);
BLECharacteristic pUserCharacteristic(UUID_USER_CHARACTERISTIC, BLEWrite, 32,
                                      false);
BLECharacteristic pPassCharacteristic(UUID_PASS_CHARACTERISTIC, BLEWrite, 32,
                                      false);
BLECharacteristic pLockStateCharacteristic(UUID_LOCKSTATE_CHARACTERISTIC,
                                           BLEWrite, 1, true);

// Security
BLEService CryptoService(UUID_CRYPTO_SERVICE);
BLECharacteristic pKeyCharacteristic(UUID_KEY_CHARACTERISTIC,
                                     BLERead | BLEWrite, 219, false);

BLEService AdminService(UUID_ADMIN_SERVICE);
BLECharacteristic pAdminCharacteristic(UUID_ADMIN_CHARACTERISTIC, BLEWrite, 32,
                                       false);
BLECharacteristic pAdminPassCharacteristic(UUID_ADMINPASS_CHARACTERISTIC,
                                           BLEWrite, 32, false);
// admins can add users
BLECharacteristic pAddUserCharacteristic(UUID_ADDUSER_CHARACTERISTIC, BLEWrite,
                                         32, false);
BLECharacteristic pAddPassCharacteristic(UUID_ADDPASS_CHARACTERISTIC, BLEWrite,
                                         32, false);
// admin action
BLECharacteristic pAdminActionCharacteristic(UUID_ADMINACTION_CHARACTERISTIC,
                                             BLEWrite, 1, false);

void setupBLE() {
  if (!BLE.begin()) {
    logFatalln("starting Bluetooth Low Energy module failed!");

    while (1)
      ;
  }

  BLE.setLocalName(BLE_NAME);
  // add the characteristic to the service
  UserService.addCharacteristic(pUserCharacteristic);
  UserService.addCharacteristic(pPassCharacteristic);
  UserService.addCharacteristic(pLockStateCharacteristic);
  pLockStateCharacteristic.setEventHandler(BLEWritten, onUserWrite);

  CryptoService.addCharacteristic(pKeyCharacteristic);
  pKeyCharacteristic.setEventHandler(BLEWritten, onCryptoWrite);

  AdminService.addCharacteristic(pAdminCharacteristic);
  AdminService.addCharacteristic(pAdminPassCharacteristic);
  AdminService.addCharacteristic(pAddUserCharacteristic);
  AdminService.addCharacteristic(pAddPassCharacteristic);
  AdminService.addCharacteristic(pAdminActionCharacteristic);
  pAdminActionCharacteristic.setEventHandler(BLEWritten, onAdminWrite);

  // add service
  BLE.addService(UserService);
  BLE.addService(AdminService);
  BLE.addService(CryptoService);

  // start advertising
  BLE.advertise();
}

void onUserWrite(BLEDevice central, BLECharacteristic characteristic) {
  uint8_t m_aes_tag[16] = {0};
  uint8_t m_aes_cipher[16] = {0};
  uint8_t m_aes_text[16] = {0};
  const unsigned char *user = pUserCharacteristic.value();
#if DEBUG_MODE == 1
  for (size_t i = 0; i < 32; i++) {
    Serial.print(user[i], HEX); // Output as hexadecimal
    Serial.print(" ");
  }
  Serial.print("\n");
#endif
  memcpy(m_aes_cipher, user, 16);
  memcpy(m_aes_tag, user+16, 16);
  aes_decrypt_gcm(m_aes_cipher, m_aes_tag, m_aes_text);
  std::string username(m_aes_text, m_aes_text + 16);
  Serial.print(("username: "+username+"\n").c_str());
  const unsigned char *pass = pPassCharacteristic.value();
#if DEBUG_MODE == 1
  for (size_t i = 0; i < 32; i++) {
    Serial.print(pass[i], HEX); // Output as hexadecimal
    Serial.print(" ");
  }
  Serial.print("\n");
#endif
  memcpy(m_aes_cipher, pass, 16);
  memcpy(m_aes_tag, pass+16, 16);
  aes_decrypt_gcm(m_aes_cipher, m_aes_tag, m_aes_text);
  std::string userpass(m_aes_text, m_aes_text + 16);
  Serial.print(("userpass: "+userpass+"\n").c_str());
  const unsigned char *lockstate = characteristic.value();
#if DEBUG_MODE == 1
  for (size_t i = 0; i < 1; i++) {
    Serial.print(lockstate[i], HEX); // Output as hexadecimal
    Serial.print(" ");
  }
  Serial.print("\n");
#endif
  return;
}

void onAdminWrite(BLEDevice central, BLECharacteristic characteristic) {
   uint8_t m_aes_tag[16] = {0};
  uint8_t m_aes_cipher[16] = {0};
  uint8_t m_aes_text[16] = {0};
  const unsigned char *admin = pAdminCharacteristic.value();
#if DEBUG_MODE == 1
  for (size_t i = 0; i < 32; i++) {
    Serial.print(admin[i], HEX); // Output as hexadecimal
    Serial.print(" ");
  }
  Serial.print("\n");
#endif
  memcpy(m_aes_cipher, admin, 16);
  memcpy(m_aes_tag, admin+16, 16);
  aes_decrypt_gcm(m_aes_cipher, m_aes_tag, m_aes_text);

  const unsigned char *adminPass = pAdminPassCharacteristic.value();
#if DEBUG_MODE == 1
  for (size_t i = 0; i < 32; i++) {
    Serial.print(adminPass[i], HEX); // Output as hexadecimal
    Serial.print(" ");
  }
  Serial.print("\n");
#endif
  memcpy(m_aes_cipher, adminPass, 16);
  memcpy(m_aes_tag, adminPass+16, 16);
  aes_decrypt_gcm(m_aes_cipher, m_aes_tag, m_aes_text);

  const unsigned char *userName = pAddUserCharacteristic.value();
#if DEBUG_MODE == 1
  for (size_t i = 0; i < 32; i++) {
    Serial.print(userName[i], HEX); // Output as hexadecimal
    Serial.print(" ");
  }
  Serial.print("\n");
#endif
  memcpy(m_aes_cipher, userName, 16);
  memcpy(m_aes_tag, userName+16, 16);
  aes_decrypt_gcm(m_aes_cipher, m_aes_tag, m_aes_text);

  const unsigned char *userPass = pAddPassCharacteristic.value();
#if DEBUG_MODE == 1
  for (size_t i = 0; i < 32; i++) {
    Serial.print(userPass[i], HEX); // Output as hexadecimal
    Serial.print(" ");
  }
  Serial.print("\n");
#endif
  memcpy(m_aes_cipher, userPass, 16);
  memcpy(m_aes_tag, userPass+16, 16);
  aes_decrypt_gcm(m_aes_cipher, m_aes_tag, m_aes_text);

  const unsigned char *action = characteristic.value();
#if DEBUG_MODE == 1
  for (size_t i = 0; i < 1; i++) {
    Serial.print(action[i], HEX); // Output as hexadecimal
    Serial.print(" ");
  }
  Serial.print("\n");
#endif
  return;
}

void onCryptoWrite(BLEDevice central, BLECharacteristic characteristic) {
  uint8_t m_publicKey[RSA_KEY_SIZE];
  memcpy(m_publicKey, characteristic.value(), RSA_KEY_SIZE);
#if DEBUG_MODE == 1
  for (size_t i = 0; i < RSA_KEY_SIZE; i++) {
    Serial.print(m_publicKey[i], HEX); // Output as hexadecimal
    Serial.print(" ");
  }
  Serial.print("\n");
#endif
  // create AES key
  uint8_t encrypted_aes_key[RSA_KEY_SIZE];
  generate_aes_key();
  // encrypt AES Key
  if (encrypt_aes_key_with_rsa(m_publicKey, RSA_KEY_SIZE,
                               encrypted_aes_key) == 0) {
    logInfoln("AES-256 Key encrypted successfully with RSA-1024 & OAEP!");
  } else {
    logFatalln("Error during encryption!");
  }
  characteristic.writeValue(encrypted_aes_key, RSA_KEY_SIZE);
  logInfoln("Set AES-256 Key");
}

void setLockStateFromBLE(LockState state) {
  switch (state) {
  case LOCKED:
    pLockStateCharacteristic.setValue("0");
    break;
  case UNLOCKED:
    pLockStateCharacteristic.setValue("1");
    break;
  case SHORT_UNLOCK:
    pLockStateCharacteristic.setValue("2");
    break;
  default:
    pLockStateCharacteristic.setValue("0");
    break;
  }
}

void printBLEaddress() { Serial.println(BLE.address()); }