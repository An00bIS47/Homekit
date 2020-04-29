//
// HAPEncryption.cpp
// Homekit
//
//  Created on: 08.05.2018
//      Author: michael
//

#include "HAPEncryption.hpp"
#include "HAPLogger.hpp"
#include "HAPHelper.hpp"
#include "HAPGlobals.hpp"

#if HAP_USE_MBEDTLS_POLY
#include "m_chacha20_poly1305.h"
#else
#include "chacha20_poly1305.h"
#endif

#include "mbedtls/chachapoly.h"

#define TAG "CHACHA20_POLY1305"


int HAPEncryption::pad(size_t *padded_buflen_p, uint8_t *msg, 
        const uint8_t *buf, size_t unpadded_buflen, size_t blocksize, 
        size_t max_msglen, bool zeroPadded )
{
    unsigned char          *tail;
    size_t                  i;
    size_t                  xpadlen;
    size_t                  xpadded_len;
    volatile unsigned char  mask;
    unsigned char           barrier_mask;

    if (blocksize <= 0U) {
        return -1;
    }
    xpadlen = blocksize - 1U;

    if ((blocksize & (blocksize - 1U)) == 0U) {
        xpadlen -= unpadded_buflen & (blocksize - 1U);
    } else {
        xpadlen -= unpadded_buflen % blocksize;
    }
    if ((size_t) SIZE_MAX - unpadded_buflen <= xpadlen) {
        //ESP_LOGE("Missuse");
        return -1;
    }
    xpadded_len = unpadded_buflen + xpadlen;
    

    if (max_msglen != 0) {
        if (xpadded_len >= max_msglen) {
            return -1;
        } 
    }
   
    
    if (padded_buflen_p != NULL) {
        *padded_buflen_p = xpadded_len + 1U;
    }
    
    if (msg == NULL) {
        return -2;
    }


    memcpy(msg, buf, unpadded_buflen);
    tail = &msg[xpadded_len];

    mask = 0U;
    for (i = 0; i < blocksize; i++) {
        barrier_mask = (unsigned char) (((i ^ xpadlen) - 1U) >> 8);

        if (zeroPadded) {
            tail[-i] = (tail[-i] & mask) | (0x00 & barrier_mask);
        } else {
            tail[-i] = (tail[-i] & mask) | (0x80 & barrier_mask);    
        }
        
        mask |= barrier_mask;
    }
    return 0;
}


int HAPEncryption::begin(){

#if !defined (__APPLE__)    
    int result = sodium_init();
#else
    int result = 0;    
#endif

    if (result < 0) {
        /* panic! the library couldn't be initialized, it is not safe to use */
        LogE("[ERROR] Sodium couldn't be initialized!", true);
    }

    return result;
}

int HAPEncryption::unpad(size_t *unpadded_buflen_p, const unsigned char *buf,
             size_t padded_buflen, size_t blocksize)
{
    const unsigned char *tail;
    unsigned char        acc = 0U;
    unsigned char        c;
    unsigned char        valid = 0U;
    volatile size_t      pad_len = 0U;
    size_t               i;
    size_t               is_barrier;

    if (padded_buflen < blocksize || blocksize <= 0U) {
        return -1;
    }
    tail = &buf[padded_buflen - 1U];

    for (i = 0U; i < blocksize; i++) {
        c = tail[-i];
        is_barrier =
            (( (acc - 1U) & (pad_len - 1U) & ((c ^ 0x80) - 1U) ) >> 8) & 1U;
        acc |= c;
        pad_len |= i & (1U + ~is_barrier);
        valid |= (unsigned char) is_barrier;
    }
    *unpadded_buflen_p = padded_buflen - 1U - pad_len;

    return (int) (valid - 1U);
}




// function computePoly1305(cipherText, AAD, nonce, key) {
//     if (AAD == null) {
//         AAD = Buffer.alloc(0);
//     }

//     const msg =
//         Buffer.concat([
//             AAD,
//             getPadding(AAD, 16),
//             cipherText,
//             getPadding(cipherText, 16),
//             UInt53toBufferLE(AAD.length),
//             UInt53toBufferLE(cipherText.length)
//         ])

//     const polyKey = Sodium.crypto_stream_chacha20(32, nonce, key);
//     const computed_hmac = Sodium.crypto_onetimeauth(msg, polyKey);
//     polyKey.fill(0);

//     return computed_hmac;
// }

#if !defined (__APPLE__)
int HAPEncryption::computePoly1305(uint8_t* hmac, uint8_t* cipherText, 
            size_t cipherTextLength, uint8_t* AAD, uint8_t *nonce, 
            uint8_t *key) {

    begin();

    if (AAD == nullptr) {
        //AAD = Buffer.alloc(0);
    }

    size_t block_size = 16;

    int paddedCipherLength  = paddedLength(cipherTextLength, block_size);
    int paddedAADLength     = paddedLength(HAP_ENCRYPTION_AAD_SIZE, block_size);

    int paddedAADLengthNum       = paddedLength(HAP_ENCRYPTION_AAD_SIZE, 8);
    int paddedCipherLengthNum    = paddedLength( HAPHelper::numDigits(paddedCipherLength), 8);

    int paddedLength        = paddedAADLength 
                            + paddedCipherLength 
                            + paddedAADLengthNum 
                            + paddedCipherLengthNum;


    uint8_t msg[paddedLength];
    
#if HAP_DEBUG_ENCRYPTION    
    Serial.printf("paddedLength: %d\n", paddedLength);
#endif
    
    int aad_len = HAP_ENCRYPTION_AAD_SIZE;

    memcpy(msg, AAD, aad_len);
    memcpy(msg + paddedAADLength, cipherText, cipherTextLength);
    memcpy(msg + paddedAADLength + paddedCipherLength, &aad_len, 1);
    memcpy(msg + paddedAADLength + paddedCipherLength + paddedAADLengthNum, &cipherTextLength, HAPHelper::numDigits(cipherTextLength) );


#if HAP_DEBUG_ENCRYPTION
    Serial.printf("msg: %d = %d\n", paddedLength, sizeof msg);
    HAPHelper::arrayPrint(msg, paddedLength);
#endif

    uint8_t polyKey[HAP_ENCRYPTION_KEY_SIZE] = { 0, };
    if ( crypto_stream_chacha20_ietf(polyKey, HAP_ENCRYPTION_KEY_SIZE, nonce, key) != 0 ) {
        LogE("[ERROR] Generating polyKey failed!", true);
        return -1;
    }

#if HAP_DEBUG_ENCRYPTION    
    Serial.println("polyKey: ");
    HAPHelper::arrayPrint(polyKey, HAP_ENCRYPTION_KEY_SIZE);
#endif

    // uint8_t hmac[crypto_onetimeauth_BYTES];

    if ( crypto_onetimeauth(hmac, msg, paddedLength, polyKey) != 0 ) {
        LogE("[ERROR] Generating crypto_onetimeauth!", true);
        return -1;
    }
    
#if HAP_DEBUG_ENCRYPTION    
    Serial.println("generated hmac:");
    HAPHelper::arrayPrint(hmac, crypto_onetimeauth_BYTES);
    //Serial.printf("msg: %d\n", sizeof msg);
    //HAPHelper::arrayPrint(msg, sizeof msg);
#endif


    return 0;
}
#endif


// i'd really prefer for this to be a direct call to
// Sodium.crypto_aead_chacha20poly1305_decrypt()
// but unfortunately the way it constructs the message to
// calculate the HMAC is not compatible with homekit
// (long story short, it uses [ AAD, AAD.length, CipherText, CipherText.length ]
// whereas homekit expects [ AAD, CipherText, AAD.length, CipherText.length ]
// 
// function verifyAndDecrypt(cipherText, mac, AAD, nonce, key) {
//     const matches =
//         Sodium.crypto_verify_16(
//             mac,
//             computePoly1305(cipherText, AAD, nonce, key)
//         );
//
//     if (matches === 0) {
//         return Sodium
//             .crypto_stream_chacha20_xor_ic(cipherText, nonce, 1, key);
//     }
//
//     return null;
// }
#if !defined (__APPLE__)
int HAPEncryption::verifyAndDecrypt(uint8_t *decrypted, uint8_t cipherText[], 
            uint16_t length, uint8_t mac[], uint8_t aad[], 
            int decryptCount, uint8_t key[]){


    begin();

    if ( length > 1024 + HAP_ENCRYPTION_AAD_SIZE + HAP_ENCRYPTION_HMAC_SIZE ){
        LogE("NOWNOW!!", true);
    }
    
    // nonce
    // the nonce is 12 byte long
    // 
    // the first 4 bytes are always 
    //      
    //      00 00 00 00 
    // 
    // the nonce is incremented each time a decryption with the same
    // encryption key is performed
    // 
    // !!! the counter starts at the last bits (10, 11) (!started by 0!)
    // 
    // thus the remaining 8 bytes will look as follows
    // 
    //      | 00 00 00 00 | 00 00 00 00 00 00 00 01 
    //          first 4       remaining 8 bytes   ^
    //                                            | -> this will be incremented each time this function is called
    //  
    //                                               
    uint8_t nonce[HAP_ENCRYPTION_NONCE_SIZE] = { 0, };            
    nonce[4] = decryptCount % 256;
    nonce[5] = decryptCount++ / 256;
    

#if HAP_DEBUG_ENCRYPTION    
    LogD("decryptCount: " + String(decryptCount), true);

    Serial.println("nonce:");
    HAPHelper::arrayPrint(nonce, HAP_ENCRYPTION_NONCE_SIZE);

    Serial.printf("cipherText: %d\n", length);
    HAPHelper::arrayPrint(cipherText, length);

    Serial.println("AAD:");
    HAPHelper::arrayPrint(aad, HAP_ENCRYPTION_AAD_SIZE);

    Serial.println("mac:");
    HAPHelper::arrayPrint(mac, HAP_ENCRYPTION_HMAC_SIZE);

    Serial.println("key:");
    HAPHelper::arrayPrint(key, HAP_ENCRYPTION_KEY_SIZE);

#endif

    // 16 bytes long
    uint8_t hmac[HAP_ENCRYPTION_HMAC_SIZE] = {0,};

    if ( computePoly1305(hmac, cipherText, length, aad, nonce, key) != 0 ) {
        LogE("[ERROR] computePoly1305 failed!", true);
        return -1;
    }

#if HAP_DEBUG_ENCRYPTION  
    Serial.println("computed hmac:");
    HAPHelper::arrayPrint(hmac, HAP_ENCRYPTION_HMAC_SIZE);
#endif

    if ( crypto_verify_16(mac, hmac) != 0 ) {
#if !HAP_ENCRYPTION_SUPPRESS_WARNINGS      
        LogW("[WARNING] crypto_verify_16 failed! - Trying to decrypt it anyway ...", true);
#endif

#if HAP_ENCRYPTION_EXIT_ON_FAILURE      
        return -1;
#endif
    }

    
    // 
    // The output from the AEAD is twofold:
    //   -  A ciphertext of the same length as the plaintext.
    //   -  A 128-bit tag, which is the output of the Poly1305 function.
    // 
    //uint8_t decrypted[length];    
    if ( crypto_stream_chacha20_ietf_xor_ic(decrypted, cipherText, length, nonce, 1, key) != 0 ) {
        LogE("[ERROR] crypto_stream_chacha20_xor_ic failed!", true);
        return -1;
    }

#if HAP_DEBUG_ENCRYPTION  
    Serial.printf("decrypted: %d\n", length );
    HAPHelper::arrayPrint(decrypted, length);
    Serial.println((char*) decrypted);
#endif
    
    return 0;
}

#endif

// size_t HAPEncryption::encrypt(Stream& stream, uint8_t* buffer, uint8_t* key, uint16_t encryptCount){

//     uint8_t * buf = (uint8_t *)malloc(1360);
//     if(!buf){
//         return 0;
//     }



//     return 0;
// }


#if 0
size_t HAPEncryption::encrypt(uint8_t *message, size_t length, uint8_t* buffer, uint8_t* key, uint16_t encryptCount){

    size_t encrypted_len = 0;
	// uint8_t* decrypted_ptr = (uint8_t*)message;
	// uint8_t* encrypted_ptr = (uint8_t*)encrypted;
    uint8_t nonce[12] = {0,};

    int err_code = 0;

    int chunk_len = (length < 1024) ? length : 1024;

    uint8_t aad[HAP_ENCRYPTION_AAD_SIZE];
    aad[0] = chunk_len % 256;
    aad[1] = chunk_len / 256;

    memcpy(buffer, aad, HAP_ENCRYPTION_AAD_SIZE);
    buffer += HAP_ENCRYPTION_AAD_SIZE;
    encrypted_len += HAP_ENCRYPTION_AAD_SIZE;

    nonce[4] = encryptCount % 256;
    nonce[5] = encryptCount++ / 256;

    err_code = chacha20_poly1305_encrypt_with_nonce(nonce, key, aad, HAP_ENCRYPTION_AAD_SIZE, message, chunk_len, buffer);	

    if (err_code != 0 ) {
        LogE("[ERROR] Encrypting failed!", true);
    }

    // decrypted_ptr += chunk_len;
    // encrypted_ptr += chunk_len + CHACHA20_POLY1305_AUTH_TAG_LENGTH;
    encrypted_len += chunk_len + CHACHA20_POLY1305_AUTH_TAG_LENGTH;



    return encrypted_len;
}
#endif

// ToDo: Move to hapClient ?
/**
 * @brief 
 * 
 * @param message 
 * @param length 
 * @param encrypted_len 
 * @param key 
 * @param encryptCount 
 * @return char* 
 */
uint8_t* HAPEncryption::encrypt(uint8_t *message, size_t length, int* encrypted_len, uint8_t* key, uint16_t encryptCount) {

	// ToDo: Take care of bigger than hap encryp buffer
    //uint32_t tmp = length + (length / HAP_ENCRYPTION_BUFFER_SIZE + 1) * (HAP_ENCRYPTION_AAD_SIZE + CHACHA20_POLY1305_AUTH_TAG_LENGTH) + 1;
    // Serial.printf(">>>>>>>> tmp:                                %d\n", tmp);
    // Serial.printf(">>>>>>>> length:                             %d\n", length);
    // Serial.printf(">>>>>>>> HAP_ENCRYPTION_BUFFER_SIZE:         %d\n", HAP_ENCRYPTION_BUFFER_SIZE);
    // Serial.printf(">>>>>>>> HAP_ENCRYPTION_AAD_SIZE:            %d\n", HAP_ENCRYPTION_AAD_SIZE);
    // Serial.printf(">>>>>>>> CHACHA20_POLY1305_AUTH_TAG_LENGTH:  %d\n", CHACHA20_POLY1305_AUTH_TAG_LENGTH);

    uint32_t tmp = (length + HAP_ENCRYPTION_AAD_SIZE + CHACHA20_POLY1305_AUTH_TAG_LENGTH) < HAP_ENCRYPTION_BUFFER_SIZE ? length + HAP_ENCRYPTION_AAD_SIZE + CHACHA20_POLY1305_AUTH_TAG_LENGTH : HAP_ENCRYPTION_BUFFER_SIZE;
    // Serial.printf(">>>>>>>> tmp:                                %d\n", tmp);

    uint8_t* encrypted = (uint8_t*) calloc(1, tmp);

	uint8_t nonce[12] = {0,};
	uint8_t* decrypted_ptr = (uint8_t*)message;
	uint8_t* encrypted_ptr = (uint8_t*)encrypted;

	while (length > 0) {
        int err_code = 0;

		int chunk_len = (length < HAP_ENCRYPTION_BUFFER_SIZE) ? length : HAP_ENCRYPTION_BUFFER_SIZE;
		length -= chunk_len;

		uint8_t aad[HAP_ENCRYPTION_AAD_SIZE];
		aad[0] = chunk_len % 256;
		aad[1] = chunk_len / 256;

		memcpy(encrypted_ptr, aad, HAP_ENCRYPTION_AAD_SIZE);
		encrypted_ptr += HAP_ENCRYPTION_AAD_SIZE;
		*encrypted_len += HAP_ENCRYPTION_AAD_SIZE;

		nonce[4] = encryptCount % 256;
		nonce[5] = encryptCount++ / 256;


#if 1

#if HAP_DEBUG_ENCRYPTION
        Serial.println("=========================================================");
        HAPHelper::array_print("nonce", nonce, 12);
        HAPHelper::array_print("key", key, strlen((const char*)key));
        HAPHelper::array_print("aad", aad, HAP_ENCRYPTION_AAD_SIZE);
        HAPHelper::array_print("decrypted_ptr", decrypted_ptr, strlen((const char*)decrypted_ptr));
        printf("chunk_len %d\n", chunk_len);
        printf("length %zu\n", length);
#endif
		err_code = chacha20_poly1305_encrypt_with_nonce(nonce, key, aad, HAP_ENCRYPTION_AAD_SIZE, decrypted_ptr, chunk_len, encrypted_ptr);	

#if HAP_DEBUG_ENCRYPTION
        HAPHelper::array_print("encrypted_ptr", encrypted_ptr, chunk_len + 16);
        Serial.println("=========================================================");
#endif

#else
        

        mbedtls_chachapoly_context chachapoly_ctx;
        mbedtls_chachapoly_init(&chachapoly_ctx);

        mbedtls_chachapoly_setkey(&chachapoly_ctx,key);        


        err_code = mbedtls_chachapoly_starts( &chachapoly_ctx, 
                                   nonce, 
                                   MBEDTLS_CHACHAPOLY_ENCRYPT );
        
        if (err_code != 0 ) {
            LogE("[ERROR] Encrypting failed! 1", true);
        }

        err_code = mbedtls_chachapoly_update_aad( &chachapoly_ctx, aad, HAP_ENCRYPTION_AAD_SIZE );
        if (err_code != 0 ) {
            LogE("[ERROR] Encrypting failed! 1", true);
        }

        err_code = mbedtls_chachapoly_update( &chachapoly_ctx, chunk_len, decrypted_ptr, encrypted_ptr );
        if (err_code != 0 ) {
            LogE("[ERROR] Encrypting failed! 2", true);
        }

        err_code = mbedtls_chachapoly_finish( &chachapoly_ctx, encrypted_ptr + sizeof(decrypted_ptr) );    
#endif        

		if (err_code != 0 ) {
			LogE("[ERROR] Encrypting failed!", true);
		}

		decrypted_ptr += chunk_len;
		encrypted_ptr += chunk_len + CHACHA20_POLY1305_AUTH_TAG_LENGTH;
		*encrypted_len += (chunk_len + CHACHA20_POLY1305_AUTH_TAG_LENGTH);
	}

	//_pairSetup->encryptCount = 0;

	return encrypted;
}