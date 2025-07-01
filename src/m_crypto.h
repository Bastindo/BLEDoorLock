#ifndef M_CRYPTO_H
#define M_CRYPTO_H

#include "config.h"
#include "log.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mbedtls/aes.h>
#include <mbedtls/rsa.h>
#include <mbedtls/pk.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/md.h>
#include "mbedtls/gcm.h"
// Size of AES-256 key (16 bytes)
#define AES_KEY_SIZE 16
#define RSA_KEY_SIZE 128 // 1024-bit RSA (128 bytes)

// Generates a random AES-256 key
void generate_aes_key();

// Encrypts the AES key with an RSA-1024 public key
int encrypt_aes_key_with_rsa(const uint8_t *rsa_pub_key, size_t rsa_key_len, uint8_t *encrypted_aes_key);
int aes_decrypt_gcm(const unsigned char *input, unsigned char *output, const unsigned char *tag);
#endif
