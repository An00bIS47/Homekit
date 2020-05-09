/* See RFC 5869 */

#ifndef _M_HKDF_H
#define _M_HKDF_H

#include <Arduino.h>
#include <mbedtls/md.h>
#include <mbedtls/hkdf.h>


/**
 *  \name X509 Error codes
 *  \{
 */
#define MBEDTLS_ERR_HKDF_BAD_PARAM  -0x5300  /**< Bad parameter */
/* \} name */


#ifndef CHACHA20_POLY1305_AEAD_KEYSIZE
#define CHACHA20_POLY1305_AEAD_KEYSIZE      32
#endif

#ifndef HKDF_KEY_LEN
#define HKDF_KEY_LEN      CHACHA20_POLY1305_AEAD_KEYSIZE
#endif

#ifdef __cplusplus
extern "C" {
#endif

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

#endif  // !_M_HKDF_H