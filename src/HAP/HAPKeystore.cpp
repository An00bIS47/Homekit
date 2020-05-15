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

HAPKeystore::HAPKeystore(){
    init();
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
}

HAPKeystore::~HAPKeystore(){
    clear();
}

bool HAPKeystore::begin(const char* partitionName, const char* name, bool readOnly){
    bool result = _prefs.begin(partitionName, name, readOnly);

    if (result == false) LogE("ERROR: Failed to open keystore!", true);
    return result;
}


bool HAPKeystore::readFromNVS(const char* entryName, uint8_t* buffer, uint16_t* len) {

    LogD(HAPServer::timeString() + " " + "HAPKeystore" + "->" + String(__FUNCTION__) + " [   ] " + "Reading nvs entry " + String(entryName), true);

    *len = _prefs.getBytesLength(entryName);

    if (*len == 0){
        LogD("Length was 0", true);
        return false;
    }

    buffer = (uint8_t*) malloc(sizeof(uint8_t) * (*len));
    size_t read = _prefs.getBytes(entryName, buffer, *len);        

#if HAP_DEBUG_KEYSTORE
    HAPHelper::array_print(entryName, buffer, *len);    
#endif

    if ( read != *len){
        LogE("ERROR: Failed to read " + String(entryName) + "from keystore", true );
        return false;
    }
    return true;
}


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


uint8_t* HAPKeystore::getDeviceWebserverCert(){

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


uint8_t* HAPKeystore::getDevicePrivateKey(){

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