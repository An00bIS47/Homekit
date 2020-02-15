//
// HAPKeysizes.hpp
// Homekit
//
//  Created on: 20.08.2017
//      Author: michael
//

#ifndef HAPKEYSIZES_HPP_
#define HAPKEYSIZES_HPP_

#ifndef CURVE25519_KEY_LENGTH
#define CURVE25519_KEY_LENGTH       	32
#endif

#ifndef CURVE25519_SECRET_LENGTH
#define CURVE25519_SECRET_LENGTH    	32
#endif

#ifndef ED25519_PUBLIC_KEY_LENGTH
#define ED25519_PUBLIC_KEY_LENGTH    	32
#endif

#ifndef ED25519_PRIVATE_KEY_LENGTH
#define ED25519_PRIVATE_KEY_LENGTH    	64
#endif

#ifndef ED25519_SIGN_LENGTH
#define ED25519_SIGN_LENGTH    			64
#endif

#ifndef HAP_ENCRYPTION_NONCE_SIZE
#define HAP_ENCRYPTION_NONCE_SIZE 		12		// Don't change!
#endif

#ifndef HAP_ENCRYPTION_HMAC_SIZE
#define HAP_ENCRYPTION_HMAC_SIZE		16		// Don't change!
#endif

#ifndef HAP_ENCRYPTION_KEY_SIZE
#define HAP_ENCRYPTION_KEY_SIZE			32		// Don't change!
#endif

#ifndef HAP_ENCRYPTION_AAD_SIZE
#define HAP_ENCRYPTION_AAD_SIZE 		2
#endif

#ifndef CHACHA20_POLY1305_AEAD_KEYSIZE
#define CHACHA20_POLY1305_AEAD_KEYSIZE      32
#endif

#ifndef HKDF_KEY_LEN
#define HKDF_KEY_LEN      CHACHA20_POLY1305_AEAD_KEYSIZE
#endif


#endif