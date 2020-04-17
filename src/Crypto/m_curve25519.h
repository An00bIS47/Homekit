
#ifndef _M_CURVE25519_H_
#define _M_CURVE25519_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ecdh.h"

#include <stdint.h>


#define CURVE25519_KEY_LENGTH       32
#define CURVE25519_SECRET_LENGTH    32

int m_curve25519_key_generate(mbedtls_ecdh_context ctx_srv, mbedtls_ctr_drbg_context ctr_drbg, uint8_t public_key[], uint8_t private_key[]);
int m_curve25519_shared_secret(mbedtls_ecdh_context ctx_srv, mbedtls_ctr_drbg_context ctr_drbg, uint8_t public_key[], 
        uint8_t* secret, int* secret_length);

void m_reverse (uint8_t* buf, size_t sz);

#ifdef __cplusplus
}
#endif

#endif //#ifndef _M_CURVE25519_H_
