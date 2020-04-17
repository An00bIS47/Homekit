
#ifdef __cplusplus
extern "C" {
#endif

#ifndef _M_CHACHA20_POLY1305_H_
#define _M_CHACHA20_POLY1305_H_


#include <stdint.h>

#ifndef CHACHA20_POLY1305_AUTH_TAG_LENGTH
#define CHACHA20_POLY1305_AUTH_TAG_LENGTH       16
#endif

#ifndef CHACHA20_POLY1305_NONCE_LENGTH
#define CHACHA20_POLY1305_NONCE_LENGTH          12
#endif

enum chacha20_poly1305_type {
    CHACHA20_POLY1305_TYPE_PS05,
    CHACHA20_POLY1305_TYPE_PS06,
    CHACHA20_POLY1305_TYPE_PV02,
    CHACHA20_POLY1305_TYPE_PV03,
};

int chacha20_poly1305_decrypt(enum chacha20_poly1305_type type, uint8_t* key, 
        uint8_t* aad, int aad_len,
        uint8_t* encrypted, int encrypted_len, uint8_t* decrypted);
int chacha20_poly1305_decrypt_with_nonce(uint8_t* nonce, uint8_t* key, uint8_t* aad, int add_len, uint8_t* encrypted, int encrypted_len, uint8_t* decrypted);


int chacha20_poly1305_encrypt(enum chacha20_poly1305_type type, uint8_t* key, 
        uint8_t* aad, int aad_len,
        uint8_t* plain_text, int plain_text_length, 
        uint8_t* encrypted);

int chacha20_poly1305_encrypt_with_nonce(uint8_t nonce[CHACHA20_POLY1305_NONCE_LENGTH], uint8_t* key, 
        uint8_t* aad, int aad_len,
        uint8_t* plain_text, int plain_text_length, 
        uint8_t* encrypted);


#endif //#ifndef _M_CHACHA20_POLY1305_H_

#ifdef __cplusplus
}
#endif