#include "ble_server2.h"

#include "m_crypto.h"

// User login + Lockstate
BLEService UserService(UUID_USER_SERVICE);
BLECharacteristic pUserCharacteristic(UUID_USER_CHARACTERISTIC, BLEWrite, 32,
                                      false);
BLECharacteristic pPassCharacteristic(UUID_PASS_CHARACTERISTIC, BLEWrite, 32,
                                      false);
BLECharacteristic pLockStateCharacteristic(UUID_LOCKSTATE_CHARACTERISTIC,
                                           BLEWrite, 32, false);

// Security
BLEService CryptoService(UUID_CRYPTO_SERVICE);
BLECharacteristic pKeyCharacteristic(
    UUID_KEY_CHARACTERISTIC,
    BLERead |  // BLENotify |
        BLEWrite,
    256, true);  // 256 bytes for receiving chunked RSA key, will send 128 bytes
                 // for encrypted AES response

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
                                             BLEWrite, 32, false);

// Chunked RSA key reception buffer with header support
uint8_t g_rsaKeyBuffer[RSA_KEY_SIZE];
size_t g_rsaKeyBytesReceived = 0;
size_t g_expectedTotalSize = 0;
bool g_rsaKeyComplete = false;
uint8_t g_lastChunkNum = 255;  // Track chunk sequence

// Key exchange state management
enum KeyExchangeState {
  IDLE = 0,            // Ready for new key exchange
  RECEIVING_KEY = 1,   // Currently receiving RSA key chunks
  PROCESSING_KEY = 2,  // Processing complete RSA key
  AES_KEY_READY = 3,   // AES key encrypted and ready to read
  ERROR_STATE = 4      // Error occurred, need reset
};

volatile KeyExchangeState g_keyExchangeState = IDLE;
volatile bool g_notifyClientReady = false;

// Global response buffer for encrypted AES key
uint8_t response_buffer[RSA_ENCRYPTED_SIZE];

void setupBLE() {
  if (!BLE.begin()) {
    logFatalln("starting Bluetooth Low Energy module failed!");

    while (1);
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
  const unsigned char* user = pUserCharacteristic.value();
#if DEBUG_MODE == 1
  for (size_t i = 0; i < 32; i++) {
    Serial.print(user[i], HEX);  // Hexadezimal ausgeben
    Serial.print(" ");
  }
  Serial.print("\n");
#endif
  memcpy(m_aes_cipher, user, 16);
  memcpy(m_aes_tag, user + 16, 16);
  aes_decrypt_gcm(m_aes_cipher, 16, m_aes_text, m_aes_tag);
  std::string username(m_aes_text, m_aes_text + 16);
  Serial.print(("username: " + username + "\n").c_str());
  const unsigned char* pass = pPassCharacteristic.value();
#if DEBUG_MODE == 1
  for (size_t i = 0; i < 32; i++) {
    Serial.print(pass[i], HEX);  // Hexadezimal ausgeben
    Serial.print(" ");
  }
  Serial.print("\n");
#endif
  memcpy(m_aes_cipher, pass, 16);
  memcpy(m_aes_tag, pass + 16, 16);
  aes_decrypt_gcm(m_aes_cipher, 16, m_aes_text, m_aes_tag);
  std::string userpass(m_aes_text, m_aes_text + 16);
  Serial.print(("userpass: " + userpass + "\n").c_str());
  const unsigned char* lockstate = characteristic.value();
#if DEBUG_MODE == 1
  Serial.println("Received encrypted lockstate:");
  for (size_t i = 0; i < 32; i++) {
    Serial.print(lockstate[i], HEX);  // Hexadezimal ausgeben
    Serial.print(" ");
  }
  Serial.println();
#endif

  // Decrypt lock state
  uint8_t lockstate_cipher[16] = {0};
  uint8_t lockstate_tag[16] = {0};
  uint8_t lockstate_text[16] = {0};

  memcpy(lockstate_cipher, lockstate, 16);
  memcpy(lockstate_tag, lockstate + 16, 16);

  int decrypt_result =
      aes_decrypt_gcm(lockstate_cipher, 16, lockstate_text, lockstate_tag);
  if (decrypt_result != 0) {
    Serial.println("Failed to decrypt lock state");
    return;
  }

  // Convert decrypted text to string and extract lock state
  std::string lockstate_str(lockstate_text, lockstate_text + 16);
  // Remove null padding
  size_t null_pos = lockstate_str.find('\0');
  if (null_pos != std::string::npos) {
    lockstate_str.resize(null_pos);
  }

  Serial.print("Decrypted lockstate: ");
  Serial.println(lockstate_str.c_str());

  // Process the lock state command
  if (checkAccess(username, userpass)) {
    logInfoln("[BLE Server] Authentication successful");
    int lock_command = atoi(lockstate_str.c_str());
    switch (lock_command) {
      case 0:
        setLockStateByUser(LOCKED, username);
        logVerboseln("[BLE Server] Lock closed");
        break;
      case 1:
        setLockStateByUser(UNLOCKED, username);
        logVerboseln("[BLE Server] Lock opened");
        break;
      case 2:  // short open
        setLockStateByUser(SHORT_UNLOCK, username);
        logVerboseln("[BLE Server] Lock short opened");
        break;
      default:
        setLockStateByUser(LOCKED, username);
        logWarnln("[BLE Server] Invalid lock state, defaulting to locked");
        break;
    }
  } else {
    logWarnln("[BLE Server] Authentication failed");
  }
  return;
}

void onAdminWrite(BLEDevice central, BLECharacteristic characteristic) {
  uint8_t admin_cipher[16] = {0}, admin_tag[16] = {0}, admin_text[16] = {0};
  uint8_t adminpass_cipher[16] = {0}, adminpass_tag[16] = {0},
          adminpass_text[16] = {0};
  uint8_t username_cipher[16] = {0}, username_tag[16] = {0},
          username_text[16] = {0};
  uint8_t userpass_cipher[16] = {0}, userpass_tag[16] = {0},
          userpass_text[16] = {0};

  // Decrypt admin name
  const unsigned char* admin = pAdminCharacteristic.value();
#if DEBUG_MODE == 1
  Serial.println("Received encrypted admin name:");
  for (size_t i = 0; i < 32; i++) {
    Serial.print(admin[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
#endif
  memcpy(admin_cipher, admin, 16);
  memcpy(admin_tag, admin + 16, 16);
  aes_decrypt_gcm(admin_cipher, 16, admin_text, admin_tag);

  // Decrypt admin password
  const unsigned char* adminPass = pAdminPassCharacteristic.value();
#if DEBUG_MODE == 1
  Serial.println("Received encrypted admin password:");
  for (size_t i = 0; i < 32; i++) {
    Serial.print(adminPass[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
#endif
  memcpy(adminpass_cipher, adminPass, 16);
  memcpy(adminpass_tag, adminPass + 16, 16);
  aes_decrypt_gcm(adminpass_cipher, 16, adminpass_text, adminpass_tag);

  // Decrypt user name
  const unsigned char* userName = pAddUserCharacteristic.value();
#if DEBUG_MODE == 1
  Serial.println("Received encrypted user name:");
  for (size_t i = 0; i < 32; i++) {
    Serial.print(userName[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
#endif
  memcpy(username_cipher, userName, 16);
  memcpy(username_tag, userName + 16, 16);
  aes_decrypt_gcm(username_cipher, 16, username_text, username_tag);

  // Decrypt user password
  const unsigned char* userPass = pAddPassCharacteristic.value();
#if DEBUG_MODE == 1
  Serial.println("Received encrypted user password:");
  for (size_t i = 0; i < 32; i++) {
    Serial.print(userPass[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
#endif
  memcpy(userpass_cipher, userPass, 16);
  memcpy(userpass_tag, userPass + 16, 16);
  aes_decrypt_gcm(userpass_cipher, 16, userpass_text, userpass_tag);

  // Decrypt admin action
  const unsigned char* action = characteristic.value();
#if DEBUG_MODE == 1
  Serial.println("Received encrypted admin action:");
  for (size_t i = 0; i < 32; i++) {
    Serial.print(action[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
#endif

  uint8_t action_cipher[16] = {0};
  uint8_t action_tag[16] = {0};
  uint8_t action_text[16] = {0};

  memcpy(action_cipher, action, 16);
  memcpy(action_tag, action + 16, 16);

  int decrypt_result =
      aes_decrypt_gcm(action_cipher, 16, action_text, action_tag);
  if (decrypt_result != 0) {
    Serial.println("Failed to decrypt admin action");
    return;
  }

  // Convert all decrypted data to strings and remove null padding
  std::string admin_name(admin_text, admin_text + 16);
  std::string admin_pass(adminpass_text, adminpass_text + 16);
  std::string user_name(username_text, username_text + 16);
  std::string user_pass(userpass_text, userpass_text + 16);
  std::string action_str(action_text, action_text + 16);

  // Remove null padding from all strings
  size_t null_pos;
  null_pos = admin_name.find('\0');
  if (null_pos != std::string::npos) admin_name.resize(null_pos);
  null_pos = admin_pass.find('\0');
  if (null_pos != std::string::npos) admin_pass.resize(null_pos);
  null_pos = user_name.find('\0');
  if (null_pos != std::string::npos) user_name.resize(null_pos);
  null_pos = user_pass.find('\0');
  if (null_pos != std::string::npos) user_pass.resize(null_pos);
  null_pos = action_str.find('\0');
  if (null_pos != std::string::npos) action_str.resize(null_pos);

  Serial.print("Decrypted admin: ");
  Serial.println(admin_name.c_str());
  Serial.print("Decrypted user: ");
  Serial.println(user_name.c_str());
  Serial.print("Decrypted action: ");
  Serial.println(action_str.c_str());

  // Process admin action
  if (checkAdminAccess(admin_name, admin_pass)) {
    logInfoln("[BLE Server] Admin Authentication successful");
    int admin_command = atoi(action_str.c_str());
    switch (admin_command) {
      case 0:
        removeUser(user_name);
        logInfoln(("[BLE Server] User " + user_name + " removed").c_str());
        break;
      case 1:
        if (user_pass.length() == 0) {
          logWarnln("[BLE Server] Add User: Password missing");
          return;
        }
        addUser({user_name, hashPassword(user_pass), UserRole::User});
        logInfoln(("[BLE Server] User " + user_name + " added").c_str());
        break;
      default:
        logWarnln(("[BLE Server] Invalid admin action: " + action_str).c_str());
        break;
    }
  } else {
    logWarnln("[BLE Server] Admin Authentication failed");
  }
  return;
}

void onCryptoWrite(BLEDevice central, BLECharacteristic characteristic) {
  // Check if we're in a valid state to receive data
  if (g_keyExchangeState == PROCESSING_KEY) {
#if DEBUG_MODE == 1
    Serial.println(
        "*** ERROR: Still processing previous key, ignoring new data ***");
#endif
    return;
  }

  // If we're in AES_KEY_READY state and get new data, reset to start new
  // exchange
  if (g_keyExchangeState == AES_KEY_READY) {
#if DEBUG_MODE == 1
    Serial.println("*** Starting new key exchange, resetting state ***");
#endif
    resetKeyExchangeState();
  }

  const uint8_t* data = characteristic.value();
  size_t dataSize = characteristic.valueLength();

#if DEBUG_MODE == 1
  Serial.print("Received RSA key data: ");
  Serial.print(dataSize);
  Serial.print(" bytes, state: ");
  Serial.print(g_keyExchangeState);
  Serial.print(", first byte: 0x");
  Serial.print(data[0], HEX);
  Serial.print(", header: ");
  for (size_t i = 0; i < min(dataSize, (size_t)8); i++) {
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  // Check for actual data content vs buffer size issue
  if (dataSize == 256) {
    Serial.println(
        "*** WARNING: Received 256 bytes - this might be the buffer size, not "
        "actual data! ***");
    // For chunked data, look for the actual chunk size
    if (data[0] == 0xFF && dataSize >= 4) {
      // This looks like a chunk, determine actual size by finding data end
      // Chunk format: [0xFF][size_low][size_high][chunk_num][data...]
      size_t expectedDataInChunk = 16;  // Expected data per chunk
      size_t actualChunkSize = 4 + expectedDataInChunk;  // header + data

      // Verify this makes sense by checking for padding zeros
      bool foundPadding = false;
      for (size_t i = actualChunkSize; i < min(dataSize, actualChunkSize + 10);
           i++) {
        if (data[i] == 0x7b ||
            data[i] == 0xe4) {  // Look for the pattern we saw in logs
          foundPadding = true;
          break;
        }
      }

      if (foundPadding) {
        Serial.print("Detected chunk with actual size: ");
        Serial.print(actualChunkSize);
        Serial.println(" bytes (rest is buffer padding)");
        dataSize = actualChunkSize;
      }
    }
  }
#endif

  // Check if this looks like PEM data (contains "-----BEGIN")
  const char* pem_begin = "-----BEGIN";
  if (dataSize > 10) {
    char data_str[25];
    size_t check_len = min(dataSize, (size_t)24);
    memcpy(data_str, data, check_len);
    data_str[check_len] = '\0';

    if (strstr(data_str, pem_begin)) {
#if DEBUG_MODE == 1
      Serial.println("Detected PEM format data, processing as single write...");
#endif
      g_keyExchangeState = PROCESSING_KEY;
      handleFullData(data, dataSize, characteristic);
      return;
    }
  }

  // Check for chunked data with 4-byte header
  if (dataSize >= 4 && dataSize <= 32 && data[0] == 0xFF) {
    // This looks like a proper chunk (4-byte header + up to 28 bytes data)
#if DEBUG_MODE == 1
    Serial.println("Detected individual chunk, processing...");
#endif
    if (g_keyExchangeState == IDLE) {
      g_keyExchangeState = RECEIVING_KEY;
    }
    handleChunkedData(data, dataSize, characteristic);
  } else if (dataSize >= 140 && dataSize <= 300 && data[0] == 0x30) {
    // This looks like DER-formatted RSA key (starts with 0x30) - fallback
#if DEBUG_MODE == 1
    Serial.println("Detected DER RSA key, processing as single write...");
#endif
    g_keyExchangeState = PROCESSING_KEY;
    handleFullData(data, dataSize, characteristic);
  } else {
    // For any other case, treat as full data (e.g., PEM that doesn't start with
    // -----BEGIN)
#if DEBUG_MODE == 1
    Serial.println("Processing as full data (fallback)...");
#endif
    g_keyExchangeState = PROCESSING_KEY;
    handleFullData(data, dataSize, characteristic);
  }
}

void handleChunkedData(const uint8_t* chunk, size_t chunkSize,
                       BLECharacteristic& characteristic) {
  // Minimum chunk size check (header = 4 bytes)
  if (chunkSize < 4) {
    logWarnln("Invalid chunk size, too small for header");
    return;
  }

  // Parse header: [0xFF][size_low][size_high][chunk_num]
  if (chunk[0] != 0xFF) {
    logWarnln("Invalid chunk header magic byte");
    return;
  }

  size_t totalSize = chunk[1] | (chunk[2] << 8);
  uint8_t chunkNum = chunk[3];
  size_t dataSize = chunkSize - 4;  // Actual data in this chunk

#if DEBUG_MODE == 1
  Serial.print("Raw header bytes: 0x");
  Serial.print(chunk[0], HEX);
  Serial.print(" 0x");
  Serial.print(chunk[1], HEX);
  Serial.print(" 0x");
  Serial.print(chunk[2], HEX);
  Serial.print(" 0x");
  Serial.print(chunk[3], HEX);
  Serial.print(" -> total_size=");
  Serial.print(totalSize);
  Serial.print(", chunk=");
  Serial.println(chunkNum);
#endif

  // Sanity check on total size
  if (totalSize < 100 || totalSize > 300) {
    logWarnln("Invalid total size in header, ignoring chunk");
#if DEBUG_MODE == 1
    Serial.print("Total size ");
    Serial.print(totalSize);
    Serial.println(" is out of range (100-300)");
#endif
    return;
  }

  // Sanity check on chunk size vs received size
  if (chunkSize > 32) {
    logWarnln("Chunk size too large for individual chunk");
#if DEBUG_MODE == 1
    Serial.print("Chunk size ");
    Serial.print(chunkSize);
    Serial.println(" > 32, not a valid individual chunk");
#endif
    return;
  }

#if DEBUG_MODE == 1
  Serial.print("Processing chunk ");
  Serial.print(chunkNum);
  Serial.print(", total size: ");
  Serial.print(totalSize);
  Serial.print(", chunk data: ");
  Serial.print(dataSize);
  Serial.print(" bytes, buffer progress: ");
  Serial.print(g_rsaKeyBytesReceived);
  Serial.print("/");
  Serial.print(g_expectedTotalSize);
  Serial.print(", last chunk: ");
  Serial.println(g_lastChunkNum);
#endif

  // First chunk initialization
  if (chunkNum == 0) {
    if (totalSize > RSA_KEY_SIZE) {
      logWarnln("Total size exceeds buffer capacity");
      resetChunkedReception();
      return;
    }

    g_expectedTotalSize = totalSize;
    g_rsaKeyBytesReceived = 0;
    g_rsaKeyComplete = false;
    g_lastChunkNum = 255;  // Will be set to 0 after processing this chunk

#if DEBUG_MODE == 1
    Serial.print("Starting new chunked key transfer, expected total size: ");
    Serial.println(g_expectedTotalSize);
#endif
  }
  // Validate chunk sequence (skip validation for chunk 0)
  else if (chunkNum != g_lastChunkNum + 1) {
    logWarnln("Chunk sequence error, resetting");
#if DEBUG_MODE == 1
    Serial.print("Expected chunk ");
    Serial.print(g_lastChunkNum + 1);
    Serial.print(", got ");
    Serial.println(chunkNum);
#endif
    resetChunkedReception();
    return;
  }

  // Check if we have expected size set
  if (g_expectedTotalSize == 0 && chunkNum != 0) {
    logWarnln("Received non-first chunk without initialization, resetting");
    resetChunkedReception();
    return;
  }

  // Check buffer space against actual buffer size, not expected size
  if (g_rsaKeyBytesReceived + dataSize > RSA_KEY_SIZE) {
    logWarnln("Buffer overflow detected, resetting");
#if DEBUG_MODE == 1
    Serial.print("Would overflow buffer: ");
    Serial.print(g_rsaKeyBytesReceived);
    Serial.print(" + ");
    Serial.print(dataSize);
    Serial.print(" > ");
    Serial.println(RSA_KEY_SIZE);
#endif
    resetChunkedReception();
    return;
  }

  // Also check against expected total size for data integrity
  if (g_rsaKeyBytesReceived + dataSize > g_expectedTotalSize) {
    logWarnln("Data size mismatch detected");
#if DEBUG_MODE == 1
    Serial.print("Data size issue: ");
    Serial.print(g_rsaKeyBytesReceived);
    Serial.print(" + ");
    Serial.print(dataSize);
    Serial.print(" > expected ");
    Serial.println(g_expectedTotalSize);
#endif
    // Don't reset here, just warn - could be padding or extra data
  }

  // Copy data (skip 4-byte header)
  memcpy(g_rsaKeyBuffer + g_rsaKeyBytesReceived, chunk + 4, dataSize);
  g_rsaKeyBytesReceived += dataSize;
  g_lastChunkNum = chunkNum;

#if DEBUG_MODE == 1
  Serial.print("Successfully copied ");
  Serial.print(dataSize);
  Serial.print(" bytes to buffer position ");
  Serial.print(g_rsaKeyBytesReceived - dataSize);
  Serial.print(", new total: ");
  Serial.print(g_rsaKeyBytesReceived);
  Serial.print("/");
  Serial.println(g_expectedTotalSize);
#endif

  // Check if transfer is complete
  if (g_rsaKeyBytesReceived >= g_expectedTotalSize) {
    g_rsaKeyComplete = true;
    g_keyExchangeState = PROCESSING_KEY;

#if DEBUG_MODE == 1
    Serial.println(
        "*** Complete RSA key received via chunked transfer, processing... "
        "***");
    Serial.print("Final buffer size: ");
    Serial.print(g_rsaKeyBytesReceived);
    Serial.print(" bytes (expected ");
    Serial.print(g_expectedTotalSize);
    Serial.println(" bytes)");
#endif

    // Process the complete RSA key
    processCompleteRSAKey(characteristic);

    // Reset for next key exchange
    resetChunkedReception();
  } else {
#if DEBUG_MODE == 1
    Serial.print("Waiting for more chunks, progress: ");
    Serial.print(g_rsaKeyBytesReceived);
    Serial.print("/");
    Serial.println(g_expectedTotalSize);
#endif
  }
}

void handleFullData(const uint8_t* data, size_t dataSize,
                    BLECharacteristic& characteristic) {
#if DEBUG_MODE == 1
  Serial.print("Received full RSA key data: ");
  Serial.print(dataSize);
  Serial.println(" bytes");
#endif

  // Copy full data to buffer
  memcpy(g_rsaKeyBuffer, data, dataSize);
  g_expectedTotalSize = dataSize;
  g_rsaKeyBytesReceived = dataSize;

#if DEBUG_MODE == 1
  Serial.println(
      "*** Complete RSA key received in single write, processing... ***");
#endif

  // Process the complete RSA key
  processCompleteRSAKey(characteristic);

  // Reset for next key exchange
  resetChunkedReception();
}

void processCompleteRSAKey(BLECharacteristic& characteristic) {
  // Check if we received DER data (fallback) or PEM data
  bool is_der_data = false;
  if (g_expectedTotalSize >= 3 && g_rsaKeyBuffer[0] == 0x30 &&
      g_rsaKeyBuffer[1] == 0x81 && g_rsaKeyBuffer[2] == 0x89) {
    is_der_data = true;
  }

#if DEBUG_MODE == 1
  if (is_der_data) {
    Serial.println(
        "*** PROCESSING COMPLETE RSA KEY (DER FORMAT - FALLBACK) ***");
    Serial.print("DER key size: ");
    Serial.println(g_expectedTotalSize);
    Serial.print("DER key (hex, first 32 bytes): ");
    for (size_t i = 0; i < min(g_expectedTotalSize, (size_t)32); i++) {
      if (g_rsaKeyBuffer[i] < 0x10) Serial.print("0");
      Serial.print(g_rsaKeyBuffer[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  } else {
    Serial.println("*** PROCESSING COMPLETE RSA KEY (PEM FORMAT) ***");
    Serial.print("Received PEM key (");
    Serial.print(g_expectedTotalSize);
    Serial.println(" bytes):");

    // Ensure null termination for string printing
    char pem_str[g_expectedTotalSize + 1];
    memcpy(pem_str, g_rsaKeyBuffer, g_expectedTotalSize);
    pem_str[g_expectedTotalSize] = '\0';
    Serial.println(pem_str);
    Serial.println("*** END PEM KEY ***");
  }
#endif

  // Initialize the encrypted AES key buffer to zeros
  uint8_t encrypted_aes_key[RSA_ENCRYPTED_SIZE];
  memset(encrypted_aes_key, 0, RSA_ENCRYPTED_SIZE);

  // Generate a new AES key
  generate_aes_key();

#if DEBUG_MODE == 1
  Serial.print("Generated AES key (32 bytes): ");
  for (size_t i = 0; i < AES_KEY_SIZE; i++) {
    if (m_aes_key[i] < 0x10) Serial.print("0");
    Serial.print(m_aes_key[i], HEX);
    Serial.print(" ");
    if ((i + 1) % 16 == 0) Serial.println();
  }
  if (AES_KEY_SIZE % 16 != 0) Serial.println();
#endif

  int encrypt_result;

  if (is_der_data) {
    // Use DER encryption function
    encrypt_result = encrypt_aes_key_with_rsa_der(
        g_rsaKeyBuffer, g_expectedTotalSize, encrypted_aes_key);
#if DEBUG_MODE == 1
    Serial.println("*** Using DER encryption function (fallback) ***");
#endif
  } else {
    // Use PEM encryption function
    // Ensure null-terminated string for PEM
    char pem_key[g_expectedTotalSize + 1];
    memcpy(pem_key, g_rsaKeyBuffer, g_expectedTotalSize);
    pem_key[g_expectedTotalSize] = '\0';

#if DEBUG_MODE == 1
    Serial.println("*** Using PEM encryption function ***");
    Serial.print("PEM string length: ");
    Serial.println(strlen(pem_key));
#endif

    encrypt_result = encrypt_aes_key_with_rsa_pem(pem_key, strlen(pem_key) + 1,
                                                  encrypted_aes_key);
  }

#if DEBUG_MODE == 1
  Serial.print("*** RSA Encryption Debug ***");
  Serial.print(" Key format: ");
  Serial.print(is_der_data ? "DER" : "PEM");
  Serial.print(", Result: ");
  Serial.println(encrypt_result);
#endif

  if (encrypt_result == 0) {
    logInfoln("AES-256 Key successfully encrypted with RSA-1024 & OAEP!");
#if DEBUG_MODE == 1
    Serial.println("RSA encryption successful!");
#endif
  } else {
    logFatalln("Error during encryption!");
#if DEBUG_MODE == 1
    Serial.print("RSA encryption failed with error code: ");
    Serial.println(encrypt_result);

    // Common mbedTLS error codes
    if (encrypt_result == -0x4280) {
      Serial.println("MBEDTLS_ERR_PK_KEY_INVALID_FORMAT - Invalid key format");
    } else if (encrypt_result == -0x3800) {
      Serial.println("MBEDTLS_ERR_PK_INVALID_PUBKEY - Invalid public key");
    } else if (encrypt_result == -0x4200) {
      Serial.println("MBEDTLS_ERR_PK_TYPE_MISMATCH - Key type mismatch");
    }

    // Zero out the response to avoid sending garbage
    memset(encrypted_aes_key, 0, RSA_ENCRYPTED_SIZE);
#endif
  }

  // Send the response (encrypted key or zeros if encryption failed)
  // Ensure encrypted data is placed at the beginning of the characteristic
  // buffer
  uint8_t response_buffer[RSA_ENCRYPTED_SIZE];
  memcpy(response_buffer, encrypted_aes_key, RSA_ENCRYPTED_SIZE);
  characteristic.writeValue(response_buffer, RSA_ENCRYPTED_SIZE);

#if DEBUG_MODE == 1
  Serial.print("*** Sent ");
  Serial.print(RSA_ENCRYPTED_SIZE);
  Serial.println(" bytes of encrypted AES key to characteristic ***");
  Serial.println("Client should extract first 128 bytes from response");
#endif

  // Update state to indicate AES key is ready
  if (encrypt_result == 0) {
    g_keyExchangeState = AES_KEY_READY;
    logInfoln("AES-256 Key response sent successfully");
  } else {
    g_keyExchangeState = ERROR_STATE;
    logWarnln("AES-256 Key encryption failed, error response sent");
  }

#if DEBUG_MODE == 1
  if (encrypt_result == 0) {
    Serial.println("*** AES-256 Key response sent successfully ***");
    Serial.print("Encrypted AES Key (first 64 bytes): ");
    for (size_t i = 0; i < min((size_t)64, (size_t)RSA_ENCRYPTED_SIZE); i++) {
      if (encrypted_aes_key[i] < 0x10) Serial.print("0");
      Serial.print(encrypted_aes_key[i], HEX);
      Serial.print(" ");
      if ((i + 1) % 16 == 0) Serial.println();
    }
    if (64 % 16 != 0) Serial.println();

    Serial.print("*** STATE: AES_KEY_READY (");
    Serial.print(g_keyExchangeState);
    Serial.println(") - CLIENT CAN READ ***");
  } else {
    Serial.println("*** Sent zero response due to encryption failure ***");
    Serial.print("*** STATE: ERROR_STATE (");
    Serial.print(g_keyExchangeState);
    Serial.println(") ***");
  }
#endif
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

void handleConcatenatedChunks(const uint8_t* data, size_t dataSize,
                              BLECharacteristic& characteristic) {
#if DEBUG_MODE == 1
  Serial.print("Analyzing concatenated data of ");
  Serial.print(dataSize);
  Serial.println(" bytes...");

  // Show first 64 bytes as hex for debugging
  Serial.println("Raw data (first 64 bytes as hex):");
  for (size_t i = 0; i < min(dataSize, (size_t)64); i++) {
    if (data[i] < 0x10) Serial.print("0");
    Serial.print(data[i], HEX);
    Serial.print(" ");
    if ((i + 1) % 16 == 0) Serial.println();
  }
  if (dataSize > 0 && dataSize % 16 != 0) Serial.println();
#endif

  // For PEM development mode: look for "-----BEGIN PUBLIC KEY-----"
  const char* pem_start = "-----BEGIN PUBLIC KEY-----";
  const char* pem_end = "-----END PUBLIC KEY-----";

  // Convert data to string for PEM search
  char data_str[dataSize + 1];
  memcpy(data_str, data, dataSize);
  data_str[dataSize] = '\0';

#if DEBUG_MODE == 1
  Serial.println("Data as string (first 100 chars):");
  for (size_t i = 0; i < min(dataSize, (size_t)100); i++) {
    char c = data[i];
    if (c >= 32 && c <= 126) {  // Printable ASCII
      Serial.print(c);
    } else {
      Serial.print('.');
    }
  }
  Serial.println();
#endif

  char* begin_pos = strstr(data_str, pem_start);
  char* end_pos = strstr(data_str, pem_end);

  if (begin_pos && end_pos && end_pos > begin_pos) {
    // Found complete PEM key
    size_t pem_len = (end_pos - begin_pos) + strlen(pem_end);

#if DEBUG_MODE == 1
    Serial.println("*** Found complete PEM key in concatenated data ***");
    Serial.print("PEM length: ");
    Serial.println(pem_len);
#endif

    // Copy PEM data to buffer
    memcpy(g_rsaKeyBuffer, begin_pos, pem_len);
    g_expectedTotalSize = pem_len;
    g_rsaKeyBytesReceived = pem_len;

    processCompleteRSAKey(characteristic);
    resetChunkedReception();
    return;
  }

  // Fallback: check if this looks like DER data for backwards compatibility
  if (dataSize == 256 && data[0] == 0xFF && data[3] == 0) {
    // This is chunked DER data - extract the DER portion
    if (dataSize >= 4 + 3 && data[4] == 0x30 && data[5] == 0x81 &&
        data[6] == 0x89) {
      // Found DER sequence, copy 140 bytes
      memcpy(g_rsaKeyBuffer, data + 4, 140);
      g_expectedTotalSize = 140;
      g_rsaKeyBytesReceived = 140;

#if DEBUG_MODE == 1
      Serial.println(
          "*** Found DER data in concatenated chunks (fallback mode) ***");
#endif

      processCompleteRSAKey(characteristic);
      resetChunkedReception();
      return;
    }
  }

  logWarnln("Could not extract valid PEM or DER key from concatenated data");
}

void resetChunkedReception() {
  g_rsaKeyBytesReceived = 0;
  g_expectedTotalSize = 0;
  g_rsaKeyComplete = false;
  g_lastChunkNum = 255;
#if DEBUG_MODE == 1
  Serial.println("Chunked reception state reset");
#endif
}

void resetKeyExchangeState() {
  g_keyExchangeState = IDLE;
  resetChunkedReception();
#if DEBUG_MODE == 1
  Serial.println("*** Key exchange state reset to IDLE ***");
#endif
}