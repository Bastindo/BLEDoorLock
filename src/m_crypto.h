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
// Größe des AES-256 Schlüssels (16 Bytes)
#define AES_KEY_SIZE 16
#define RSA_KEY_SIZE 256  // 1024-bit RSA (256 Bytes)

// Erzeugt einen zufälligen AES-256 Schlüssel
void generate_aes_key();

// Verschlüsselt den AES-Schlüssel mit einem RSA-1024 Public Key
int encrypt_aes_key_with_rsa(const uint8_t *rsa_pub_key, size_t rsa_key_len,
                             uint8_t *encrypted_aes_key);
int aes_decrypt_gcm(const unsigned char *input, unsigned char *output,
                    const unsigned char *tag);
#endif
