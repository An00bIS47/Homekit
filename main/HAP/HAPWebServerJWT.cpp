//
// HAPWebServer.cpp
// Homekit
//
//  Created on: 14.12.2018
//      Author: michael
//


#include "HAPWebServerJWT.hpp"
#include "HAPLogger.hpp"
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>

#include <ArduinoJson.h>

#include "base64url.h"

/**
 * Return a string representation of an mbedtls error code
 */
char* HAPWebServerJWT::mbedtlsError(int errnum) {
    static char buffer[200];
    mbedtls_strerror(errnum, buffer, sizeof(buffer));
    return buffer;
} // mbedtlsError



/**
 * Create a JWT token for GCP.
 * For full details, perform a Google search on JWT.  However, in summary, we build two strings.  One that represents the
 * header and one that represents the payload.  Both are JSON and are as described in the GCP and JWT documentation.  Next
 * we base64url encode both strings.  Note that is distinct from normal/simple base64 encoding.  Once we have a string for
 * the base64url encoding of both header and payload, we concatenate both strings together separated by a ".".   This resulting
 * string is then signed using RSASSA which basically produces an SHA256 message digest that is then signed.  The resulting
 * binary is then itself converted into base64url and concatenated with the previously built base64url combined header and
 * payload and that is our resulting JWT token.
 * @param projectId The GCP project.
 * @param privateKey The PEM or DER of the private key.
 * @param privateKeySize The size in bytes of the private key.
 * @returns A JWT token for transmission to GCP.
 */
char* HAPWebServerJWT::createGCPJWT(const char* audience, const char* issuer, uint8_t* privateKey, size_t privateKeySize) {
    
    const char header[] = "{\"alg\":\"RS256\",\"typ\":\"JWT\"}";    
    char base64Header[BASE64_ENCODE_OUT_SIZE(strlen(header))];
    base64url_encode(
        (unsigned char *)header,   // Data to encode.
        strlen(header),            // Length of data to encode.
        base64Header);             // Base64 encoded data.

    time_t now;
    time(&now);
    uint32_t iat = now;              // Set the time now.
    uint32_t exp = iat + 60*60;      // Set the expiry time.

    char payload[160];
    if (issuer == NULL) {
        sprintf(payload, "{\"iat\":%d,\"exp\":%d,\"aud\":\"%s\"}", iat, exp, audience);
    } else {
        sprintf(payload, "{\"iat\":%d,\"exp\":%d,\"aud\":\"%s\",\"iss\":\"%s\"}", iat, exp, audience, issuer);
    }

    char base64Payload[BASE64_ENCODE_OUT_SIZE(strlen(payload))];
    base64url_encode(
        (unsigned char *)payload,  // Data to encode.
        strlen(payload),           // Length of data to encode.
        base64Payload);            // Base64 encoded data.

    uint8_t headerAndPayload[BASE64_ENCODE_OUT_SIZE(strlen(header)) + BASE64_ENCODE_OUT_SIZE(strlen(payload)) + 1];
    sprintf((char*)headerAndPayload, "%s.%s", base64Header, base64Payload);

    // At this point we have created the header and payload parts, converted both to base64 and concatenated them
    // together as a single string.  Now we need to sign them using RSASSA

    mbedtls_pk_context pk_context;
    mbedtls_pk_init(&pk_context);
    int rc = mbedtls_pk_parse_key(&pk_context, privateKey, privateKeySize, NULL, 0);
    if (rc != 0) {
        printf("Failed to mbedtls_pk_parse_key: %d (-0x%x): %s\n", rc, -rc, mbedtlsError(rc));
        return nullptr;
    }

    bool is_rsa = mbedtls_pk_can_do(&pk_context, MBEDTLS_PK_RSA);
    // Serial.println("is_rsa: + " + String(is_rsa));


    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_entropy_init(&entropy);

    const char* pers="MyEntropy";
                
    mbedtls_ctr_drbg_seed(
        &ctr_drbg,
        mbedtls_entropy_func,
        &entropy,
        (const unsigned char*)pers,
        strlen(pers));
    

    uint8_t digest[32];
    rc = mbedtls_md(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), headerAndPayload, strlen((char*)headerAndPayload), digest);
    if (rc != 0) {
        printf("Failed to mbedtls_md: %d (-0x%x): %s\n", rc, -rc, mbedtlsError(rc));
        return nullptr;        
    }

    size_t retSize;
    size_t key_len = mbedtls_pk_get_len(&pk_context);
    size_t sig_len = (is_rsa ? key_len : key_len * 2 + 10);
    unsigned char *oBuf = (unsigned char *) calloc(1, sig_len);
    if (oBuf == NULL) {
        LogE("Unable to allocate memory for jwt", true);
        return nullptr;
    }

    rc = mbedtls_pk_sign(&pk_context, MBEDTLS_MD_SHA256, digest, sizeof(digest), oBuf, &retSize, mbedtls_ctr_drbg_random, &ctr_drbg);
    if (rc != 0) {
        printf("Failed to mbedtls_pk_sign: %d (-0x%x): %s\n", rc, -rc, mbedtlsError(rc));
        return nullptr;        
    }


    char base64Signature[BASE64_ENCODE_OUT_SIZE(retSize)];
    base64url_encode((unsigned char *)oBuf, retSize, base64Signature);

    char* retData = (char*)malloc(strlen((char*)headerAndPayload) + 1 + strlen((char*)base64Signature) + 1);

    sprintf(retData, "%s.%s", headerAndPayload, base64Signature);

    mbedtls_pk_free(&pk_context);
    free(oBuf);

    return retData;
}

bool HAPWebServerJWT::verifyToken(const char* token, const char* audience, const char* issuer, const uint8_t* publicKey, size_t publicKeySize){
    

    const char pattern[]=". ";
    char *ptr = strchr(token, ' ');
    
    int headerStart = ptr - token + 1;
        
    char *jwt = (char*)token + headerStart;
    char * pch;

    bool passedHeader = false;
    bool passedPayload = false;
    bool passedSignature = false;


    String headerAndPayload = "";
    int index = 0;
    pch = strtok (jwt, pattern);
    while (pch != NULL)
    {                
        if (index == 0) {
            // header
            Serial.printf ("header: %s\n", pch);

            char header[BASE64_DECODE_OUT_SIZE(strlen(pch)) + 1];
            base64url_decode(
                (const char *)pch,          // Data to decode.
                strlen(pch),                // Length of data to encode.
                (unsigned char *) header);  // Base64 decoded data.

            //header[BASE64_DECODE_OUT_SIZE(strlen(pch))] = '\0';


            const size_t bufferSize = JSON_OBJECT_SIZE(2) + 20;
            DynamicJsonDocument root(bufferSize);            
            DeserializationError error = deserializeJson(root, header);

           if (error) {                
                LogW("JWT Verification: Error parsing header of jwt failed!", true);
                return false;
            }
            
            serializeJsonPretty(root, Serial);            

            passedHeader = true;
            headerAndPayload += String(pch) + ".";
        } else if (index == 1) {
            // payload
            Serial.printf ("payload encoded: %s\n", pch);

            char payload[BASE64_DECODE_OUT_SIZE(strlen(pch)) + 1];
            base64url_decode(
                (const char *)pch,          // Data to decode.
                strlen(pch),                // Length of data to encode.
                (unsigned char *) payload);  // Base64 decoded data.

            //payload[BASE64_DECODE_OUT_SIZE(strlen(pch))] = '\0';            

            Serial.printf ("payload: %s\n", payload);

            const size_t bufferSize = JSON_OBJECT_SIZE(4) + 96;
            DynamicJsonDocument root(bufferSize);
            DeserializationError error = deserializeJson(root, payload);

            if (error) {                
                LogW("JWT Verification: Error parsing jwt failed!", true);
                return false;
            }
            
            serializeJsonPretty(root, Serial);


            //long iat = root["iat"]; // 1546378963
            long exp = root["exp"]; // 1546382563
            const char* aud = root["aud"]; // "api"            

            time_t now;
            time(&now);

            if  (strcmp(aud, audience) == 0) {                
                LogD("JWT Verification: audience passed", true);
            } else {
                LogW("JWT Verification: audience failed", true);
                return false;
            }

            if ( exp >= now ) {
                LogD("JWT Verification: exp passed", true);
            } else {
                LogW("JWT Verification: exp failed", true);
                return false;
            }

            if (issuer != NULL) {
                if (root.containsKey("iss")) {
                    const char* iss = root["iss"]; // iss
                    if ( strcmp(iss, issuer) == 0) {
                        LogD("JWT Verification: issuer passed", true);
                    } else {
                        LogW("JWT Verification: issuer failed", true);
                        return false;
                    }
                } else {
                    LogW("JWT Verification: issuer failed", true);
                    return false;
                }
            }

            passedPayload = true;
            headerAndPayload += String(pch);
        } else if (index == 2) {
            // signature
            Serial.printf ("signature: %s\n", pch);

            unsigned char signature[BASE64_DECODE_OUT_SIZE(strlen(pch)) + 1];
            base64url_decode(
                (const char *)pch,          // Data to decode.
                strlen(pch),                // Length of data to encode.
                signature);  // Base64 decoded data.

            //payload[BASE64_DECODE_OUT_SIZE(strlen(pch))] = '\0';   


            mbedtls_pk_context pk_context;
            mbedtls_pk_init(&pk_context);
            int rc = mbedtls_pk_parse_public_key(&pk_context, publicKey, publicKeySize);
            if (rc != 0) {
                Serial.printf("Failed to mbedtls_pk_parse_public_key: %d (-0x%x): %s\n", rc, -rc, mbedtlsError(rc));
                return false;
            }

            Serial.println("Parsed public key");

            unsigned char payload[headerAndPayload.length()];
            Serial.println(headerAndPayload);
            headerAndPayload.toCharArray((char*)payload, headerAndPayload.length());

            uint8_t digest[32];
            rc = mbedtls_md(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), payload, headerAndPayload.length(), digest);
            if (rc != 0) {
                Serial.printf("Failed to mbedtls_md: %d (-0x%x): %s\n", rc, -rc, mbedtlsError(rc));
                return false;        
            }

            Serial.println("Parsed hash key");
            // mbedtls_pk_verify()

            if( ( rc = mbedtls_pk_verify( &pk_context, MBEDTLS_MD_SHA256, digest, 32, (const unsigned char*)signature, strlen((const char*)signature) ) ) != 0 )
            {
                Serial.printf( " failed\n  ! mbedtls_pk_verify returned -0x%04x\n", -rc );
                
            }


            passedSignature = true;

        } else {
            LogW("JWT Verification: Error parsing jwt failed!", true);
        }
        pch = strtok (NULL, pattern);
        index++;
    }

    if (passedHeader && passedPayload && passedSignature) {
        return true;
    }

    return false;
}