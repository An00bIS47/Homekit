//
// HAPKeystore.cpp
// Homekit
//
//  Created on: 15.05.2020
//      Author: michael
//

#include "HAPKeystore.hpp"
#include "HAPLogger.hpp"
#include "HAPHelper.hpp"
#include "HAPServer.hpp"

#include "mbedtls/error.h"
#include "mbedtls/md.h"
#include "mbedtls/pk.h"



HAPKeystore::HAPKeystore(){
    init();

    _isValid = false;
    sprintf(_currentPartition, "%s", HAP_KEYSTORE_PARTITION_LABEL);
    _isStarted = false;    
}

void HAPKeystore::init(){
    _rootCa = nullptr;
    _rootCaPublicKeySignature  = nullptr;

    _deviceClientCert = nullptr;
    _devicePrivateKey = nullptr;
    _devicePublicKey = nullptr;
    _deviceWebserverCert = nullptr;    
    
    _updateServerCert = nullptr;
 
    _pluginServerCert_0 = nullptr;
    _pluginServerCert_1 = nullptr;
    _pluginServerCert_2 = nullptr;
    _pluginServerCert_3 = nullptr;


    _rootCaLength = 0;
    _rootCaPublicKeySignatureLength = 0;

    _deviceClientCertLength = 0;
    _devicePrivateKeyLength = 0;    
    _devicePublicKeyLength = 0;
    _deviceWebserverCertLength = 0;

    _updateServerCertLength = 0;

    _pluginServerCert_0Length = 0;
    _pluginServerCert_1Length = 0;
    _pluginServerCert_2Length = 0;
    _pluginServerCert_3Length = 0;

    _containerId = 0;
}

HAPKeystore::~HAPKeystore(){
    clear();
}

bool HAPKeystore::setCurrentPartition(const char* partitionName){
    
    if (strlen(partitionName) > 15) {
        LogE("ERROR: Partition label is too long!", true);
        return false;
    } else {
        LogD("Set current keystore partition to " + String(partitionName), true);
        sprintf(_currentPartition, "%s", partitionName);
        return true;
    }    
}

const char* HAPKeystore::getCurrentPartition(){
    return _currentPartition;
}

const char* HAPKeystore::getAlternatePartition(){
    if (strcmp(_currentPartition, HAP_KEYSTORE_PARTITION_LABEL) == 0) {
        return HAP_KEYSTORE_PARTITION_LABEL_ALT;
    } 
    return HAP_KEYSTORE_PARTITION_LABEL;
}


bool HAPKeystore::begin(){
    if (!_isStarted) {
        return begin(_currentPartition, HAP_KEYSTORE_STORAGE_LABEL, true);        
    } 
    return true;
}


bool HAPKeystore::begin(const char* partitionName, const char* name, bool readOnly){
    bool result = _prefs.begin(partitionName, name, readOnly);

    if (result == false) {
        LogE("ERROR: Failed to open partition!", true);
    } else {
        setCurrentPartition(partitionName);

        _isValid = _prefs.getBool("isValid", false);          
        if (_isValid) {
            _containerId = _prefs.getUChar("0xFD", 0);          
        }        
    }

    _isStarted = result;
    return result;
}

bool HAPKeystore::isValid(){  

    if (_isValid == false){
        LogE("ERROR: Keystore is not valid!", true);
    }      

    return _isValid;
}

void HAPKeystore::end(){
    _isStarted = false;
    _prefs.end();
    _isValid = false;
    clear();
}

// bool HAPKeystore::readFromNVS(const char* entryName, uint8_t* buffer, uint16_t* len) {

//     LogD(HAPServer::timeString() + " " + "HAPKeystore" + "->" + String(__FUNCTION__) + " [   ] " + "Reading nvs entry " + String(entryName), true);

//     *len = _prefs.getBytesLength(entryName);

//     if (*len == 0){
//         LogD("Length was 0", true);
//         return false;
//     }

//     buffer = (uint8_t*) malloc(sizeof(uint8_t) * (*len));
//     size_t read = _prefs.getBytes(entryName, buffer, *len);        

// #if HAP_DEBUG_KEYSTORE
//     HAPHelper::array_print(entryName, buffer, *len);    
// #endif

//     if ( read != *len){
//         LogE("ERROR: Failed to read " + String(entryName) + "from keystore", true );
//         return false;
//     }
//     return true;
// }


void HAPKeystore::clear(){

    if (_rootCa != nullptr) free(_rootCa);
    if (_rootCaPublicKeySignature != nullptr) free(_rootCaPublicKeySignature);

    if (_deviceClientCert != nullptr) free(_deviceClientCert);
    if (_devicePrivateKey != nullptr) free(_devicePrivateKey);
    if (_devicePublicKey != nullptr) free(_devicePublicKey);
    if (_deviceWebserverCert != nullptr) free(_deviceWebserverCert);

    if (_updateServerCert != nullptr) free(_updateServerCert);

    if (_pluginServerCert_0 != nullptr) free(_pluginServerCert_0);
    if (_pluginServerCert_1 != nullptr) free(_pluginServerCert_1);
    if (_pluginServerCert_2 != nullptr) free(_pluginServerCert_2);
    if (_pluginServerCert_3 != nullptr) free(_pluginServerCert_3);


    init();    
}



// bool HAPKeystore::load(const char* partitionName, const char* name){

//     if (open(partitionName, name, true) == false) return false;

//     // _rootCa
//     {
//         const char* entryName = "0x00";
//         _rootCaLength = _prefs.getBytesLength(entryName);
        

//         if (_rootCaLength == 0){
//             LogD("Length was 0", true);
//             return false;
//         }

//         _rootCa = (uint8_t*) malloc(sizeof(uint8_t) * _rootCaLength);
//         size_t read = _prefs.getBytes(entryName, _rootCa, _rootCaLength);        

// #if HAP_DEBUG_KEYSTORE
//         HAPHelper::array_print(entryName, _rootCa, _rootCaLength);    
// #endif

//         if ( read != _rootCaLength){
//             LogE("ERROR: Failed to read " + String(entryName) + "from keystore", true );
//             return false;
//         }
//     }

//     // _rootCAPublicKeySignature
//     if (readFromNVS("0x01", _rootCaPublicKeySignature, &_rootCaPublicKeySignatureLength) == false) {  
//         LogD("Keystore has no entry for device client cert", true);      
//     }

//     // _deviceClientCert
//     if (readFromNVS("0x10", _deviceClientCert, &_deviceClientCertLength) == false) {  
//         LogD("Keystore has no entry for device client cert", true);      
//     }

//     // _devicePrivateKey
//     // if (readFromNVS("0x11", _devicePrivateKey, &_devicePrivateKeyLength) == false) {  
//     //     LogD("Keystore has no entry for device private key", true);      
//     // }
//     {
//         const char* entryName = "0x11";
//         _devicePrivateKeyLength = _prefs.getBytesLength(entryName);
        

//         if (_devicePrivateKeyLength == 0){
//             LogD("Length was 0", true);
//             return false;
//         }

//         _devicePrivateKey = (uint8_t*) malloc(sizeof(uint8_t) * _devicePrivateKeyLength);
//         size_t read = _prefs.getBytes(entryName, _devicePrivateKey, _devicePrivateKeyLength);        

// #if HAP_DEBUG_KEYSTORE
//         HAPHelper::array_print(entryName, _devicePrivateKey, _devicePrivateKeyLength);    
// #endif

//         if ( read != _devicePrivateKeyLength){
//             LogE("ERROR: Failed to read " + String(entryName) + "from keystore", true );
//             return false;
//         }
//     }

//     // _devicePublicKey
//     if (readFromNVS("0x12", _devicePublicKey, &_devicePublicKeyLength) == false) {  
//         LogD("Keystore has no entry for device public key", true);      
//     }

//     // _deviceWebserverCert
//     // if (readFromNVS("0x13", _deviceWebserverCert, &_deviceWebserverCertLength) == false) {  
//     //     LogD("Keystore has no entry for device webserver cert", true);      
//     // }


//     // _updateServerCert
//     if (readFromNVS("0x20", _updateServerCert, &_updateServerCertLength) == false) {  
//         LogD("Keystore has no entry for update server cert", true);      
//     }

//     // _pluginServerCert_0
//     if (readFromNVS("0x30", _pluginServerCert_0, &_pluginServerCert_0Length) == false) {  
//         LogD("Keystore has no entry for _pluginServerCert_0", true);      
//     }

//     // _pluginServerCert_1
//     if (readFromNVS("0x31", _pluginServerCert_1, &_pluginServerCert_1Length) == false) {  
//         LogD("Keystore has no entry for _pluginServerCert_1", true);      
//     }

//     // _pluginServerCert_2
//     if (readFromNVS("0x32", _pluginServerCert_2, &_pluginServerCert_2Length) == false) {  
//         LogD("Keystore has no entry for _pluginServerCert_2", true);      
//     }

//     // _pluginServerCert_3
//     if (readFromNVS("0x33", _pluginServerCert_3, &_pluginServerCert_3Length) == false) {  
//         LogD("Keystore has no entry for _pluginServerCert_3", true);      
//     }

//     _initialized = true;
//     return true;
// }

uint8_t HAPKeystore::getContainerId(){    
    return _containerId;
}


uint8_t* HAPKeystore::getDeviceWebserverCert(){

    if (!_isStarted) begin();

    if (isValid() == false) return nullptr;



    const char* entryName = "0x13";

    if (_deviceWebserverCertLength > 0) {
        return _deviceWebserverCert;
    }

    _deviceWebserverCertLength = _prefs.getBytesLength(entryName);
    

    if (_deviceWebserverCertLength == 0){
        LogD("ERROR: Failed to read " + String(entryName) + "from keystore! Size was 0", true);
        return nullptr;
    }

    _deviceWebserverCert = (uint8_t*) malloc(sizeof(uint8_t) * _deviceWebserverCertLength);
    size_t read = _prefs.getBytes(entryName, _deviceWebserverCert, _deviceWebserverCertLength);        

#if HAP_DEBUG_KEYSTORE
    HAPHelper::array_print(entryName, _deviceWebserverCert, _deviceWebserverCertLength);    
#endif

    if ( read != _deviceWebserverCertLength){
        LogE("ERROR: Failed to read " + String(entryName) + "from keystore", true );
        return nullptr;
    }
    
    return _deviceWebserverCert;
}




uint8_t* HAPKeystore::getRootCaPublicKeySignature(){

    if (!_isStarted) begin();

    if (isValid() == false) return nullptr;

    const char* entryName = "0x01";

    if (_rootCaPublicKeySignatureLength > 0) {        
        return _rootCaPublicKeySignature;
    }

    
    _rootCaPublicKeySignatureLength = _prefs.getBytesLength(entryName);
    

    if (_rootCaPublicKeySignatureLength == 0){
        LogD("ERROR: Failed to read " + String(entryName) + "from keystore! Size was 0", true);
        return nullptr;
    }

    _rootCaPublicKeySignature = (uint8_t*) malloc(sizeof(uint8_t) * _rootCaPublicKeySignatureLength);
    size_t read = _prefs.getBytes(entryName, _rootCaPublicKeySignature, _rootCaPublicKeySignatureLength);        

#if HAP_DEBUG_KEYSTORE
    HAPHelper::array_print(entryName, _rootCaPublicKeySignature, _rootCaPublicKeySignatureLength);    
#endif

    if ( read != _rootCaPublicKeySignatureLength){
        LogE("ERROR: Failed to read " + String(entryName) + "from keystore! Size mismatch", true );
        return nullptr;
    }
    
    return _rootCaPublicKeySignature;
}


uint8_t* HAPKeystore::getDevicePrivateKey(){

    if (!_isStarted) begin();

    if (isValid() == false) return nullptr;

    const char* entryName = "0x11";

    if (_devicePrivateKeyLength > 0) {
        return _devicePrivateKey;
    }

    _devicePrivateKeyLength = _prefs.getBytesLength(entryName);
    

    if (_devicePrivateKeyLength == 0){
        LogD("ERROR: Failed to read " + String(entryName) + "from keystore! Size was 0", true);
        return nullptr;
    }

    _devicePrivateKey = (uint8_t*) malloc(sizeof(uint8_t) * _devicePrivateKeyLength);
    size_t read = _prefs.getBytes(entryName, _devicePrivateKey, _devicePrivateKeyLength);        

#if HAP_DEBUG_KEYSTORE
    HAPHelper::array_print(entryName, _devicePrivateKey, _devicePrivateKeyLength);    
#endif

    if ( read != _devicePrivateKeyLength){
        LogE("ERROR: Failed to read " + String(entryName) + "from keystore", true );
        return nullptr;
    }
    
    return _devicePrivateKey;
}

bool HAPKeystore::parseRequest(HTTPRequest * req){
    
    // Store "old" values
    char currentPartition[strlen(getCurrentPartition()) + 1];    
    strncpy(currentPartition, getCurrentPartition(), strlen(getCurrentPartition()));
    currentPartition[strlen(getCurrentPartition())] = '\0';
    
    getRootCaPublicKeySignature();  // initial load from partition, all following requests are from memory
    size_t publicKeyLength = getRootCaPublicKeySignatureLength();

    uint8_t publicKey[publicKeyLength];
    memcpy(publicKey, getRootCaPublicKeySignature(), publicKeyLength);
    
    uint8_t currentContainerId = getContainerId();

    // HAPHelper::array_print("publicKey", publicKey, publicKeyLength);


    if (_isStarted) {
        end();
    }

    LogD("Started keystore update to partition " + String(getAlternatePartition()), true);
    begin(getAlternatePartition(), HAP_KEYSTORE_STORAGE_LABEL, false);
    _prefs.clear();
    
    uint8_t signatureBuffer[256];
    size_t signatureOffset = 0;

    uint8_t shaResult[32];
 
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
    mbedtls_md_starts(&ctx);

    while ( !req->requestComplete() ) {
        uint8_t type[1];
        uint8_t length[1];
        
        req->readBytes(type, 1);
        req->readBytes(length, 1);
        
        uint8_t data[*length];
        size_t read = req->readBytes(data, *length);
        if (read != (*length)){
            LogE("ERROR: Could not read data from request!", true);
            _prefs.clear();
            end();
            mbedtls_md_free(&ctx);
            setCurrentPartition(currentPartition);

            return false;
        }

        char typeStr[6];
        sprintf(typeStr, "0x%02X", *type);


#if HAP_DEBUG_KEYSTORE        
        HAPHelper::array_print(typeStr, data, *length);
#endif

        if ( (strncmp(typeStr, "0xFD", 4) == 0) || (strncmp(typeStr, "0xfd", 4) == 0) ) {
            if (data[0] == currentContainerId) {
                LogW("WARNING: Keystore container id is currently used! - Update aborted!", true);
            
                _prefs.clear();
                end();
                mbedtls_md_free(&ctx);
                setCurrentPartition(currentPartition);

                return false;                
            }

            // char containerId[11];
            // sprintf(containerId,"%d", *data);
            LogI("Updating to Container Id " + String(data[0]), true);
            
            // calculate sha256
            mbedtls_md_update(&ctx, (const unsigned char *) type, 1);
            mbedtls_md_update(&ctx, (const unsigned char *) length, 1);
            mbedtls_md_update(&ctx, (const unsigned char *) data, *length);

            _prefs.putUChar("0xFD", data[0]);
            
        } else if ( (strncmp(typeStr, "0xFE", 4) == 0) || (strncmp(typeStr, "0xfe", 4) == 0) ) {
            memcpy(signatureBuffer + signatureOffset, data, *length);
            signatureOffset += (*length);
        } else {

            // calculate sha256
            mbedtls_md_update(&ctx, (const unsigned char *) type, 1);
            mbedtls_md_update(&ctx, (const unsigned char *) length, 1);
            mbedtls_md_update(&ctx, (const unsigned char *) data, *length);

            // check for already stored bytes to append the new data
            size_t storedLength = 0;
            storedLength = _prefs.getBytesLength(typeStr);
            
            if (storedLength > 0){
                uint8_t storedBuffer[storedLength + (*length)];
                _prefs.getBytes(typeStr, storedBuffer, storedLength);
                memcpy(storedBuffer + storedLength, data, *length);

                size_t written = _prefs.putBytes(typeStr, storedBuffer, storedLength + (*length));            
                if (written != (*length) + storedLength){
                    LogE("ERROR: Could not write data to keystore", true);
                    _prefs.clear();
                    end();
                    mbedtls_md_free(&ctx);
                    setCurrentPartition(currentPartition);

                    return false;
                }        
            } else {
                size_t written = _prefs.putBytes(typeStr, data, (*length));            
                if (written != (*length)){
                    LogE("ERROR: Could not write data to keystore", true);
                    _prefs.clear();
                    end();
                    mbedtls_md_free(&ctx);
                    setCurrentPartition(currentPartition);

                    return false;
                }
            }

        }
    }
    
    mbedtls_md_finish(&ctx, shaResult);
    mbedtls_md_free(&ctx);
    

    // HAPHelper::array_print("hash:", shaResult, 32);
    // HAPHelper::array_print("signature:", signatureBuffer, signatureOffset);
    // HAPHelper::array_print("publicKey:", publicKey, publicKeyLength);


    // ToDo: Verify Signature ECDSA 
    LogD("Verifying keystore signature ...", true);
    bool result = verifySignature(publicKey, publicKeyLength, shaResult, signatureBuffer, signatureOffset);

    if (result == false) {
        LogE("ERROR: Signature verification failed! - Aborting Update!", true);
        _prefs.clear();
    } else {
        size_t written = _prefs.putBool("isValid", true);     
        if (written != 1){
            LogE("ERROR: Could not write data to keystore", true);
            _prefs.clear();                                         
        } else {
            LogD( " OK", true);
            // Serial.print("isValid: ");
            // Serial.println(_prefs.getBool("isValid", false)); 
        }
    }
    
    end();

    setCurrentPartition(currentPartition);    

    return result;
}

bool HAPKeystore::verifySignature(const uint8_t* publicKey, size_t publicKeyLength, const uint8_t* hash, const uint8_t* signature, size_t signatureLength){
    
    int errorCode = 0;
    mbedtls_pk_context ctxPublicKey;

    mbedtls_pk_init( &ctxPublicKey );

    // parse public key
    errorCode = mbedtls_pk_parse_public_key(&ctxPublicKey, publicKey, publicKeyLength);
    if (errorCode != 0) {
        LogE("ERROR: Failed to load public key!", true);

        mbedtls_pk_free( &ctxPublicKey );
        return false;
    }

    errorCode = mbedtls_pk_verify( &ctxPublicKey, MBEDTLS_MD_SHA256, hash, 0, signature, signatureLength );
    if (errorCode != 0) {
        LogE("ERROR: Failed to verify signature!", true);

        mbedtls_pk_free( &ctxPublicKey );
        return false;
    }

    mbedtls_pk_free( &ctxPublicKey );

    return true;
}