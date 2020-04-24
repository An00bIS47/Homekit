//
// HAPEncryption.hpp
// Homekit
//
//  Created on: 08.05.2018
//      Author: michael
//

#ifndef HAPENCRYPTION_HPP_
#define HAPENCRYPTION_HPP_

#include <Arduino.h>
#include "HAPGlobals.hpp"

#if !defined (__APPLE__)
#include <sodium.h>
#endif


#ifndef HAP_ENCRYPTION_AAD_SIZE
#define HAP_ENCRYPTION_AAD_SIZE 			2
#endif

#ifndef HAP_ENCRYPTION_NONCE_SIZE
#define HAP_ENCRYPTION_NONCE_SIZE 			12		// Don't change!
#endif

#ifndef HAP_ENCRYPTION_HMAC_SIZE
#define HAP_ENCRYPTION_HMAC_SIZE			16		// Don't change!
#endif

#ifndef HAP_ENCRYPTION_KEY_SIZE
#define HAP_ENCRYPTION_KEY_SIZE				32		// Don't change!
#endif

#ifndef CHACHA20_POLY1305_AUTH_TAG_LENGTH
#define CHACHA20_POLY1305_AUTH_TAG_LENGTH	16
#endif

#ifndef CHACHA20_POLY1305_NONCE_LENGTH
#define CHACHA20_POLY1305_NONCE_LENGTH      12
#endif

// #define HAP_ENCRYPTION_AAD_SIZE 			2		// Don't change!


#define HAP_ENCRYPTION_DEBUG				0		// Be careful, it will print all keys on console
#define HAP_ENCRYPTION_EXIT_ON_FAILURE 		0		// 0 = ignore 
													// 1 = Don't ignore and return failure

#define HAP_ENCRYPTION_SUPPRESS_WARNINGS 	1		// Surpress crypto_verify_16 warning

class HAPEncryption {

public:

	static int begin();

	// Set the following values to ... to get the padded buffer size 
	//		buf 		  	= NULL
	//  	max_buflen 		= 0
	static int pad(size_t *padded_buflen_p, uint8_t *msg, 
			const uint8_t *buf, size_t unpadded_buflen, 
			size_t blocksize, size_t max_msglen, bool zeroPadded = true);

	static size_t paddedLength(size_t unpadded_buflen, size_t blocksize) {
			size_t padded_buflen = 0;
			
			pad(&padded_buflen, NULL, NULL, unpadded_buflen, blocksize, 0);
		return padded_buflen;
	}

	// only working for non-zero padded
	static int unpad(size_t *unpadded_buflen_p, const unsigned char *buf,
			size_t padded_buflen, size_t blocksize);


#if !defined (__APPLE__)
	static int computePoly1305(uint8_t* hmac, uint8_t* cipherText, 
			size_t cipherTextLength, uint8_t* AAD, uint8_t *nonce, 
			uint8_t *key);


	static int verifyAndDecrypt(uint8_t *decrypted, uint8_t cipherText[], uint16_t length, 
			uint8_t mac[], uint8_t aad[], int decryptCount, uint8_t key[]);
#endif	
	
	// not needed
	// static size_t encrypt(uint8_t *message, size_t length, uint8_t* buffer, uint8_t* key, uint16_t encryptCount);
	
	static uint8_t* encrypt(uint8_t *message, size_t length, int* encrypted_len, uint8_t* key, uint16_t encryptCount);
};

#endif /* HAPENCRYPTION_HPP_ */