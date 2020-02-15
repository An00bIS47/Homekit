//
// HAPVerifyContext.hpp
// Homekit
//
//  Created on: 31.03.2018
//      Author: michael
//

#ifndef HAPVERIFYCONTEXT_HPP_
#define HAPVERIFYCONTEXT_HPP_

#include <Arduino.h>
#include "ed25519.h"
#include "HAPGlobals.hpp"
#include "HAPKeysizes.hpp"

struct HAPLongTermContext {
	// uint8_t publicKey[ED25519_PUBLIC_KEY_LENGTH];
	uint8_t* publicKey;
	uint8_t publicKeyLength;

	// uint8_t privateKey[ED25519_PUBLIC_KEY_LENGTH];
	uint8_t* privateKey;
	uint8_t privateKeyLength;

	HAPLongTermContext() 
	: publicKeyLength(ED25519_PUBLIC_KEY_LENGTH)
	, privateKeyLength(ED25519_PRIVATE_KEY_LENGTH) {
		publicKey = (uint8_t*) malloc(sizeof(uint8_t) * publicKeyLength);
		privateKey = (uint8_t*) malloc(sizeof(uint8_t) * privateKeyLength);
	}

	~HAPLongTermContext(){
		free(publicKey);
		free(privateKey);
	}
};


struct HAPEncryptionContext {
	uint8_t encryptKey[CURVE25519_SECRET_LENGTH];
	uint8_t decryptKey[CURVE25519_SECRET_LENGTH];


	uint16_t encryptCount;
    uint16_t decryptCount;

    HAPEncryptionContext()
    : encryptCount(0)
	, decryptCount(0){

	}
	
};


struct HAPVerifyContext {
	uint8_t *secret;
	uint8_t secretLength;

	uint8_t *sessionKey; 
	uint8_t sessionKeyLength;
	
	uint8_t *accessoryLTPK;
	uint8_t accessoryLTPKLength;

	uint8_t *deviceLTPK;
	uint8_t deviceLTPKLength;	


	HAPVerifyContext() 
	: secretLength(HKDF_KEY_LEN)
	, sessionKeyLength(CURVE25519_SECRET_LENGTH)
	, accessoryLTPKLength(ED25519_PUBLIC_KEY_LENGTH)
	, deviceLTPKLength(ED25519_PUBLIC_KEY_LENGTH) {

		secret = (uint8_t*) malloc(sizeof(uint8_t) * secretLength);
		sessionKey = (uint8_t*) malloc(sizeof(uint8_t) * sessionKeyLength);
		accessoryLTPK = (uint8_t*) malloc(sizeof(uint8_t) * accessoryLTPKLength);
		deviceLTPK = (uint8_t*) malloc(sizeof(uint8_t) * deviceLTPKLength);
	}

	~HAPVerifyContext(){
		free(secret);
		free(sessionKey);
		free(accessoryLTPK);
		free(deviceLTPK);
	}
};

#endif /* HAPVERIFYCONTEXT_HPP_ */
