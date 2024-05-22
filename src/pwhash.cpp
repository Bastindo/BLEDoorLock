#include <mbedtls/sha256.h>
#include <mbedtls/pkcs5.h>
#include <mbedtls/md.h>
#include <string.h>
#include <string>

#include "pwhash.h"
#include "log.h"

#include <iostream>
#include <string>
#include <mbedtls/sha256.h>

void hash_password_with_salt(const char *password, const char *salt, unsigned char output[32]) {
    mbedtls_sha256_context sha256;
    mbedtls_sha256_init(&sha256);
    mbedtls_sha256_starts_ret(&sha256, 0);

    char salted_password[strlen(password) + strlen(salt) + 1];
    strcpy(salted_password, salt);
    strcat(salted_password, password);

    mbedtls_sha256_update_ret(&sha256, (unsigned char *)salted_password, strlen(salted_password));
    mbedtls_sha256_finish_ret(&sha256, output);
    mbedtls_sha256_free(&sha256);
}

bool verify_password_with_salt(const char *password, const char *salt, const unsigned char *stored_hash) {
    unsigned char computed_hash[32];
    hash_password_with_salt(password, salt, computed_hash);
    
    return memcmp(computed_hash, stored_hash, 32) == 0;
}