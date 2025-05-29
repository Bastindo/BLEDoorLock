
#include "m_crypto.h"

uint8_t m_aes_key[AES_KEY_SIZE] = {0};
uint8_t m_aes_iv[12] = {0x46, 0x61, 0x63, 0x68, 0x73, 0x63,
                        0x68, 0x61, 0x66, 0x74, 0x45, 0x54};

void generate_aes_key() {
  for (int i = 0; i < AES_KEY_SIZE; i++) {
    m_aes_key[i] = rand() % 256; // Zufallswerte zwischen 0-255
  }
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
                          MBEDTLS_MD_SHA256); // PKCS#1 OAEP mit SHA-256

  // Zufallszahlengenerator für OAEP
  mbedtls_entropy_context entropy;
  mbedtls_ctr_drbg_context ctr_drbg;
  mbedtls_entropy_init(&entropy);
  mbedtls_ctr_drbg_init(&ctr_drbg);
  mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, NULL, 0);

  // Verschlüssele den AES-Schlüssel mit RSA-1024 und OAEP
  ret = mbedtls_rsa_rsaes_oaep_encrypt(rsa, mbedtls_ctr_drbg_random, &ctr_drbg,
                                       MBEDTLS_RSA_PUBLIC, NULL, 0, 32,
                                       m_aes_key, encrypted_aes_key);

  // Speicher freigeben
  mbedtls_pk_free(&pk);
  mbedtls_entropy_free(&entropy);
  mbedtls_ctr_drbg_free(&ctr_drbg);

  return ret;
}

int aes_decrypt_gcm(const unsigned char *input, unsigned char *output,
                    const unsigned char *tag) {
  mbedtls_gcm_context gcm;
  mbedtls_gcm_init(&gcm);

  // Schlüssel setzen (128 Bit)
  mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, m_aes_key, AES_KEY_SIZE);

  // Entschlüsseln mit Authentifizierungsprüfung
  int ret = mbedtls_gcm_auth_decrypt(&gcm, AES_KEY_SIZE, m_aes_iv, 16, NULL, 0,
                                     tag, 12, input, output);

  mbedtls_gcm_free(&gcm);
  return ret;
}