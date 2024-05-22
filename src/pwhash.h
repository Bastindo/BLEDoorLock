#include <string>

bool verify_password_with_salt(const char *password, const char *salt, const unsigned char *stored_hash);
void hash_password_with_salt(const char *password, const char *salt, unsigned char output[32]);