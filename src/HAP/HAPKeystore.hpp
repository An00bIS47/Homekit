//
// HAPKeystore.hpp
// Homekit
//
//  Created on: 15.05.2020
//      Author: michael
//
#ifndef HAPKEYSTORE_HPP_
#define HAPKEYSTORE_HPP_

#include <Arduino.h>

#include "HAPPreferencesExt.hpp"


enum HAP_KEYSTORE_TYPE {
    HAP_KEYSTORE_TYPE_ROOT_CA                       = 0x00,
    HAP_KEYSTORE_TYPE_ROOT_CA_PUBLIC_KEY_SIGNATURE  = 0x01, 

    HAP_KEYSTORE_TYPE_DEVICE_CLIENT_CERT            = 0x10,
    HAP_KEYSTORE_TYPE_DEVICE_PRIVATE_KEY            = 0x11,
    HAP_KEYSTORE_TYPE_DEVICE_PUBLIC_KEY             = 0x12,
    HAP_KEYSTORE_TYPE_DEVICE_WEBSERVER_CERT         = 0x13,       

    HAP_KEYSTORE_TYPE_UPDATE_SERVER_CERT            = 0x20,

    HAP_KEYSTORE_TYPE_PLUGIN_0                      = 0x30,
    HAP_KEYSTORE_TYPE_PLUGIN_1                      = 0x31,
    HAP_KEYSTORE_TYPE_PLUGIN_2                      = 0x32,
    HAP_KEYSTORE_TYPE_PLUGIN_3                      = 0x33,
    HAP_KEYSTORE_TYPE_PLUGIN_4                      = 0x34,
    HAP_KEYSTORE_TYPE_PLUGIN_5                      = 0x35,
    HAP_KEYSTORE_TYPE_PLUGIN_6                      = 0x36,
    HAP_KEYSTORE_TYPE_PLUGIN_7                      = 0x37,
    HAP_KEYSTORE_TYPE_PLUGIN_8                      = 0x38,
    HAP_KEYSTORE_TYPE_PLUGIN_9                      = 0x39,
    
    HAP_KEYSTORE_TYPE_CONTAINER_ID                  = 0xFD,
    HAP_KEYSTORE_TYPE_SIGNATURE                     = 0xFE,
};


class HAPKeystore {
public:
    HAPKeystore();
    ~HAPKeystore();

    bool begin(const char* partitionName, const char* name, bool ReadOnly = true);
    // bool load(const char* partitionName, const char* name);
    void clear();

    
    uint16_t getRootCaLength() {
    	return _rootCaLength;
    }    

    uint16_t getRootCaPublicKeySignatureLength() {
    	return _rootCaPublicKeySignatureLength;
    }

    uint16_t getDeviceClientCertLength() {
    	return _deviceClientCertLength;
    }

    uint16_t getDevicePrivateKeyLength() {
    	return _devicePrivateKeyLength;
    }

    uint16_t getDevicePublicKeyLength() {
    	return _devicePublicKeyLength;
    }

    uint16_t getDeviceWebserverCertLength(){
        return _deviceWebserverCertLength;
    }

    uint16_t getUpdateServerCertLength() {
    	return _updateServerCertLength;
    }

    uint16_t getPluginServerCert_0Length() {
    	return _pluginServerCert_0Length;
    }

    uint16_t getPluginServerCert_1Length() {
    	return _pluginServerCert_1Length;
    }

    uint16_t getPluginServerCert_2Length() {
    	return _pluginServerCert_2Length;
    }

    uint16_t getPluginServerCert_3Length() {
    	return _pluginServerCert_3Length;
    }



    uint8_t* getRootCa(){
        return _rootCa;
    }
    uint8_t* getRootCaPublicKeySignature(){
        return _rootCaPublicKeySignature;
    }

    uint8_t* getDeviceClientCert(){
        return _deviceClientCert;
    }
    


    uint8_t* getDevicePrivateKey();
    uint8_t* getDeviceWebserverCert();


    uint8_t* getDevicePublicKey(){
        return _devicePublicKey;
    }
    
    
    uint8_t* getUpdateServerCert(){
        return _updateServerCert;
    }

    uint8_t* getPluginServerCert_0(){
        return _pluginServerCert_0;
    }
    uint8_t* getPluginServerCert_1(){
        return _pluginServerCert_1;
    }
    uint8_t* getPluginServerCert_2(){
        return _pluginServerCert_2;
    }
    uint8_t* getPluginServerCert_3(){
        return _pluginServerCert_3;
    }  
    

private:
    bool readFromNVS(const char* entryName, uint8_t* buffer, uint16_t* len);
    
    
    void init();
    

    HAPPreferencesExt _prefs;

    uint8_t* _rootCa;
    uint8_t* _rootCaPublicKeySignature;

    uint8_t* _deviceClientCert;
    uint8_t* _devicePrivateKey;
    uint8_t* _devicePublicKey;
    uint8_t* _deviceWebserverCert;
    
    uint8_t* _updateServerCert;

    uint8_t* _pluginServerCert_0;
    uint8_t* _pluginServerCert_1;
    uint8_t* _pluginServerCert_2;
    uint8_t* _pluginServerCert_3;
    // uint8_t* _pluginServerCert_4;
    // uint8_t* _pluginServerCert_5;
    // uint8_t* _pluginServerCert_6;
    // uint8_t* _pluginServerCert_7;
    // uint8_t* _pluginServerCert_8;
    // uint8_t* _pluginServerCert_9;


    uint16_t _rootCaLength;
    uint16_t _rootCaPublicKeySignatureLength;

    uint16_t _deviceClientCertLength;
    uint16_t _devicePrivateKeyLength;    
    uint16_t _devicePublicKeyLength;
    uint16_t _deviceWebserverCertLength;

    uint16_t _updateServerCertLength;

    uint16_t _pluginServerCert_0Length;
    uint16_t _pluginServerCert_1Length;
    uint16_t _pluginServerCert_2Length;
    uint16_t _pluginServerCert_3Length;  

};

#endif /* HAPKEYSTORE_HPP_ */