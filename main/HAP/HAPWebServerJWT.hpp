//
// HAPWebServerJWT.hpp
// Homekit
//
//  Created on: 30.12.2018
//      Author: michael
//

#ifndef HAPWEBSERVERJWT_HPP
#define HAPWEBSERVERJWT_HPP

#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <mbedtls/pk.h>
#include <mbedtls/error.h>



class HAPWebServerJWT {

public:
    static char* createGCPJWT(const char* audience, const char* issuer, uint8_t* privateKey, size_t privateKeySize);
    static bool verifyToken(const char* token, const char* audience, const char* issuer, const uint8_t* publicKey, size_t publicKeySize);
private:
    static char* mbedtlsError(int errnum);

    
};






#endif /* HAPWEBSERVERJWT_HPP */

