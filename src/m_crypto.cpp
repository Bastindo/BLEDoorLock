#include "m_crypto.h"

uint8_t m_aes_key[AES_KEY_SIZE] = {0};
uint8_t m_aes_iv[AES_IV_SIZE] = {0x46, 0x61, 0x63, 0x68, 0x73, 0x63,
                                 0x68, 0x61, 0x66, 0x74, 0x45, 0x54};

void generate_aes_key() {
  // Use mbedtls random number generator for cryptographically secure randomness
  mbedtls_entropy_context entropy;
  mbedtls_ctr_drbg_context ctr_drbg;

  mbedtls_entropy_init(&entropy);
  mbedtls_ctr_drbg_init(&ctr_drbg);

  int ret =
      mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, NULL, 0);
  if (ret == 0) {
    mbedtls_ctr_drbg_random(&ctr_drbg, m_aes_key, AES_KEY_SIZE);
  }

  mbedtls_entropy_free(&entropy);
  mbedtls_ctr_drbg_free(&ctr_drbg);
}
// OAEP mit SHA-256 für die RSA-Verschlüsselung eines AES-Keys
int encrypt_aes_key_with_rsa(const uint8_t *rsa_pub_key, size_t rsa_key_len,
                             uint8_t *encrypted_aes_key) {
  mbedtls_pk_context pk;
  mbedtls_pk_init(&pk);

  // Public Key aus PEM oder DER laden
  int ret = mbedtls_pk_parse_public_key(&pk, rsa_pub_key, rsa_key_len);
  if (ret != 0) {
    // Serial.print("Fehler beim Parsen des Public Keys! Fehlercode:\n");
    return ret;
  }

  if (!mbedtls_pk_can_do(&pk, MBEDTLS_PK_RSA)) {
    Serial.print("Kein gültiger RSA-Schlüssel!\n");
    return -1;
  }

  mbedtls_rsa_context *rsa = mbedtls_pk_rsa(pk);
  mbedtls_rsa_set_padding(rsa, MBEDTLS_RSA_PKCS_V21,
                          MBEDTLS_MD_SHA256);  // PKCS#1 OAEP mit SHA-256

  // Zufallszahlengenerator für OAEP
  mbedtls_entropy_context entropy;
  mbedtls_ctr_drbg_context ctr_drbg;
  mbedtls_entropy_init(&entropy);
  mbedtls_ctr_drbg_init(&ctr_drbg);
  mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, NULL, 0);

  // Verschlüssele den AES-Schlüssel mit RSA und OAEP
  ret = mbedtls_rsa_rsaes_oaep_encrypt(
      rsa, mbedtls_ctr_drbg_random, &ctr_drbg, MBEDTLS_RSA_PUBLIC, NULL, 0,
      AES_KEY_SIZE, m_aes_key, encrypted_aes_key);

  // Speicher freigeben
  mbedtls_pk_free(&pk);
  mbedtls_entropy_free(&entropy);
  mbedtls_ctr_drbg_free(&ctr_drbg);

  return ret;
}

// Parse DER-formatted RSA public key (from Dart client)
int encrypt_aes_key_with_rsa_der(const uint8_t *rsa_pub_key_der, size_t der_len,
                                 uint8_t *encrypted_aes_key) {
  mbedtls_pk_context pk;
  mbedtls_pk_init(&pk);

  // Parse DER format public key
  int ret = mbedtls_pk_parse_public_key(&pk, rsa_pub_key_der, der_len);
  if (ret != 0) {
    mbedtls_pk_free(&pk);
    return ret;
  }

  if (!mbedtls_pk_can_do(&pk, MBEDTLS_PK_RSA)) {
    mbedtls_pk_free(&pk);
    return -1;
  }

  mbedtls_rsa_context *rsa = mbedtls_pk_rsa(pk);
  mbedtls_rsa_set_padding(rsa, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA256);

  // Random number generator for OAEP
  mbedtls_entropy_context entropy;
  mbedtls_ctr_drbg_context ctr_drbg;
  mbedtls_entropy_init(&entropy);
  mbedtls_ctr_drbg_init(&ctr_drbg);
  mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, NULL, 0);

  // Encrypt AES key with RSA OAEP
  ret = mbedtls_rsa_rsaes_oaep_encrypt(
      rsa, mbedtls_ctr_drbg_random, &ctr_drbg, MBEDTLS_RSA_PUBLIC, NULL, 0,
      AES_KEY_SIZE, m_aes_key, encrypted_aes_key);

  // Cleanup
  mbedtls_pk_free(&pk);
  mbedtls_entropy_free(&entropy);
  mbedtls_ctr_drbg_free(&ctr_drbg);

  return ret;
}

int aes_encrypt_gcm(const unsigned char *input, size_t input_len,
                    unsigned char *output, unsigned char *tag) {
  mbedtls_gcm_context gcm;
  mbedtls_gcm_init(&gcm);

  // Schlüssel setzen (AES-256)
  int ret = mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, m_aes_key,
                               AES_KEY_SIZE * 8);
  if (ret != 0) {
    mbedtls_gcm_free(&gcm);
    return ret;
  }

  // Verschlüsseln mit Authentifizierung
  ret = mbedtls_gcm_crypt_and_tag(&gcm, MBEDTLS_GCM_ENCRYPT, input_len,
                                  m_aes_iv, AES_IV_SIZE, NULL, 0, input, output,
                                  AES_TAG_SIZE, tag);

  mbedtls_gcm_free(&gcm);
  return ret;
}

int aes_decrypt_gcm(const unsigned char *input, size_t input_len,
                    unsigned char *output, const unsigned char *tag) {
  mbedtls_gcm_context gcm;
  mbedtls_gcm_init(&gcm);

  // Schlüssel setzen (AES-256)
  int ret = mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, m_aes_key,
                               AES_KEY_SIZE * 8);
  if (ret != 0) {
    mbedtls_gcm_free(&gcm);
    return ret;
  }

  // Entschlüsseln mit Authentifizierungsprüfung
  ret = mbedtls_gcm_auth_decrypt(&gcm, input_len, m_aes_iv, AES_IV_SIZE, NULL,
                                 0, tag, AES_TAG_SIZE, input, output);

  mbedtls_gcm_free(&gcm);
  return ret;
}

// PEM format RSA encryption function
int encrypt_aes_key_with_rsa_pem(const char *pem_key, size_t pem_len,
                                 uint8_t *encrypted_aes_key) {
  mbedtls_pk_context pk;
  mbedtls_pk_init(&pk);

  // Parse PEM key
  int ret =
      mbedtls_pk_parse_public_key(&pk, (const unsigned char *)pem_key, pem_len);
  if (ret != 0) {
    mbedtls_pk_free(&pk);
    return ret;
  }

  // Get RSA context and set OAEP padding with SHA-256
  mbedtls_rsa_context *rsa = mbedtls_pk_rsa(pk);
  mbedtls_rsa_set_padding(rsa, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA256);

  // Encrypt AES key
  mbedtls_ctr_drbg_context ctr_drbg;
  mbedtls_entropy_context entropy;
  mbedtls_entropy_init(&entropy);
  mbedtls_ctr_drbg_init(&ctr_drbg);
  mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, NULL, 0);

  ret = mbedtls_rsa_pkcs1_encrypt(rsa, mbedtls_ctr_drbg_random, &ctr_drbg,
                                  MBEDTLS_RSA_PUBLIC, AES_KEY_SIZE, m_aes_key,
                                  encrypted_aes_key);

  mbedtls_entropy_free(&entropy);
  mbedtls_ctr_drbg_free(&ctr_drbg);
  mbedtls_pk_free(&pk);
  return ret;
}