#include "sdkconfig.h"
#if !CONFIG_HOMEKIT_USE_MBEDTLS

#ifndef _HKDF_H_
#define _HKDF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <wolfssl/wolfcrypt/chacha20_poly1305.h>

#ifndef CHACHA20_POLY1305_AEAD_KEYSIZE
#define CHACHA20_POLY1305_AEAD_KEYSIZE      32
#endif

#define HKDF_KEY_LEN      CHACHA20_POLY1305_AEAD_KEYSIZE

enum hkdf_key_type {
    HKDF_KEY_TYPE_PAIR_SETUP_ENCRYPT,
    HKDF_KEY_TYPE_PAIR_SETUP_CONTROLLER,
    HKDF_KEY_TYPE_PAIR_SETUP_ACCESSORY,
    HKDF_KEY_TYPE_PAIR_VERIFY_ENCRYPT,
    HKDF_KEY_TYPE_CONTROL_READ,
    HKDF_KEY_TYPE_CONTROL_WRITE,
    HKDF_KEY_TYPE_LENGTH,
};

int hkdf_key_get(enum hkdf_key_type type, uint8_t* inkey, int inkey_len, uint8_t* outkey);

#ifdef __cplusplus
}
#endif

#endif //#ifndef _HKDF_H_

#endif