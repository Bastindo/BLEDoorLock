#ifndef M_CRYPTO_H
#define M_CRYPTO_H

#include <mbedtls/aes.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/md.h>
#include <mbedtls/pk.h>
#include <mbedtls/rsa.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "log.h"
#include "mbedtls/gcm.h"
// Größe des AES-256 Schlüssels (32 Bytes)
#define AES_KEY_SIZE 32
#define AES_IV_SIZE 12
#define AES_TAG_SIZE 16
#define RSA_KEY_SIZE 256        // 1024-bit RSA (256 Bytes)
#define RSA_ENCRYPTED_SIZE 128  // RSA-1024 encryption output size

// Erzeugt einen zufälligen AES-256 Schlüssel
void generate_aes_key();

// Verschlüsselt den AES-Schlüssel mit einem RSA-1024 Public Key
int encrypt_aes_key_with_rsa(const uint8_t *rsa_pub_key, size_t rsa_key_len,
                             uint8_t *encrypted_aes_key);

// PEM format encryption function
int encrypt_aes_key_with_rsa_pem(const char *pem_key, size_t pem_len,
                                 uint8_t *encrypted_aes_key);

int encrypt_aes_key_with_rsa_der(const uint8_t *rsa_pub_key_der, size_t der_len,
                                 uint8_t *encrypted_aes_key);

// AES key storage
extern uint8_t m_aes_key[AES_KEY_SIZE];
int aes_encrypt_gcm(const unsigned char *input, size_t input_len,
                    unsigned char *output, unsigned char *tag);
int aes_decrypt_gcm(const unsigned char *input, size_t input_len,
                    unsigned char *output, const unsigned char *tag);
#endif
