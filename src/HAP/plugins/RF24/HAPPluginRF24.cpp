//
// HAPPluginRF24.cpp
// Homekit
//
//  Created on: 20.05.2020
//      Author: michael
//
#include "HAPPluginRF24.hpp"
#include "HAPServer.hpp"
#include "HAPPluginRF24DeviceWeather.hpp"
#include "HAPPluginRF24DeviceDHT.hpp"
#include "EventManager.h"


#define VERSION_MAJOR       0
#define VERSION_MINOR       3
#define VERSION_REVISION    1
#define VERSION_BUILD       0

#define HAP_PLUGIN_RF24_INTERVAL    250

#ifndef RF24_ADDRESS
#define RF24_ADDRESS        "HOMEKIT_RF24"
#endif 

// http://www.iotsharing.com/2018/03/esp-and-raspberry-connect-with-nrf24l01.html
// modified Lib: https://github.com/nhatuan84/RF24.git
// RF24          ESP32 (Feather)
// --------------------------
//  NRF24 Pinout:
// 
//      +-+-+
//  GND |_| | VCC
//   CE | | | CSN
//  SCK | | | MOSI
// MISO +-+-+ IRQ
// 
// 
// CE         -> GPIO 12    -> = LED changed to: 12
// CSN        -> GPIO 33
// CLK        -> GPIO 5
// MISO       -> GPIO 19
// MOSI       -> GPIO 18
// IRQ        -> not connected



HAPPluginRF24::HAPPluginRF24(){
    _type           = HAP_PLUGIN_TYPE_ACCESSORY;
	_name           = "RF24";
	_isEnabled      = HAP_PLUGIN_USE_RF24;
	_interval       = HAP_PLUGIN_RF24_INTERVAL;
	_previousMillis = 0;

    _version.major      = VERSION_MAJOR;
    _version.minor      = VERSION_MINOR;
    _version.revision   = VERSION_REVISION;
    _version.build      = VERSION_BUILD;

    // _newDevice  = nullptr;
    
    _radio = nullptr;

}

HAPPluginRF24::~HAPPluginRF24(){
    if (_radio) {
        delete _radio;
    }    
}


bool HAPPluginRF24::begin() {
    // _radio = new RF24(13, 33);
    _radio = new RF24(12, 33, 5, 19, 18);
    
    if (_radio->begin() ){                          // Start up the radio    

        _radio->setPALevel(RF24_PA_MIN);            //You can set it as minimum or maximum 
                                                    // depending on the distance between the 
                                                    // transmitter and receiver.
        _radio->setAutoAck(1);                      // Ensure autoACK is enabled
        _radio->setRetries(15,15);                  // Max delay between retries & number of retries
        _radio->openReadingPipe(1, (const uint8_t*)RF24_ADDRESS);   // Write to device address 'SimpleNode'
        _radio->startListening();
        _isEnabled = true; 
    } else {        
        LogE("ERROR: RF24 not found! Please check wiring!", true);
        LogE("Disabling RF24 plugin!", true);
        _isEnabled = false;
        _eventManager->removeListener(&_listenerMemberFunctionPlugin);
        return false;
    }                   

    return true;
}


HAPAccessory* HAPPluginRF24::initAccessory() {
	LogD("\nInitializing plugin: HAPPluginRF24 ...", false);
    
    for (auto& dev : _devices){     

        dev->setFakeGatoFactory(_fakeGatoFactory);
        dev->setEventManager(_eventManager);                

        _accessorySet->addAccessory(dev->initAccessory());
    }

    return nullptr;
}


void HAPPluginRF24::handleImpl(bool forced){
    // _radio->stopListening();
    // _radio->startListening();
    LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Handle plguin [" + String(_interval) + "]", true);

    if (_radio->available()){

        while(_radio->available()){
            struct RadioPacket payload;

            _radio->read( &payload, sizeof(struct RadioPacket) );


#if HAP_DEBUG_RF24
            LogD("Got Payload from id: " + String(payload.radioId, HEX), true);        
            LogD("   type:    " + String(payload.type), true);
            LogD("   temp:    " + String(payload.temperature), true);
            LogD("   hum:     " + String(payload.humidity), true);
            LogD("   pres:    " + String(payload.pressure), true);
            LogD("   voltage: " + String(payload.voltage), true);


            LogD("   Size of struct: " + String(sizeof(struct RadioPacket)), true);        
            LogD("Number of devices: " + String(_devices.size()), true);
#endif        


            int index = indexOfDevice(payload.radioId);

            if ( index == -1 ){
                if (payload.type == (uint8_t)RemoteDeviceTypeWeather){
                    
                    HAPPluginRF24DeviceWeather* newDevice = new HAPPluginRF24DeviceWeather(
                        payload.radioId,
                        String(payload.radioId)                                
                    );            
                
                    LogI("RF24: Adding new remote weather device with id " + String(payload.radioId, HEX) + " ...", false);
                    
                    // Serial.printf("event: %p\n", _eventManager);
                    // Serial.printf("fakegato: %p\n", _fakeGatoFactory);

                    newDevice->setEventManager(_eventManager);
                    newDevice->setFakeGatoFactory(_fakeGatoFactory);            

                    _accessorySet->addAccessory(newDevice->initAccessory());
                    
                    _devices.push_back(newDevice);
                    
                                    							
                    _eventManager->queueEvent( EventManager::kEventUpdatedConfig, HAPEvent(), EventManager::kLowPriority);        
                    

                    // index = indexOfDevice(payload.id);    
                    index = _devices.size() - 1;
                    LogI(" OK", true);          
                } else if (payload.type == (uint8_t)RemoteDeviceTypeDHT){
                    
                    HAPPluginRF24DeviceDHT* newDevice = new HAPPluginRF24DeviceDHT(
                        payload.radioId,
                        String(payload.radioId)                                
                    );            
                
                    LogI("RF24: Adding new remote DHT device with id " + String(payload.radioId, HEX) + " ...", false);
                    
                    // Serial.printf("event: %p\n", _eventManager);
                    // Serial.printf("fakegato: %p\n", _fakeGatoFactory);

                    newDevice->setEventManager(_eventManager);
                    newDevice->setFakeGatoFactory(_fakeGatoFactory);            

                    _accessorySet->addAccessory(newDevice->initAccessory());
                    
                    _devices.push_back(newDevice);
                    
                                    							
                    _eventManager->queueEvent( EventManager::kEventUpdatedConfig, HAPEvent(), EventManager::kLowPriority);        
                    

                    // index = indexOfDevice(payload.id);    
                    index = _devices.size() - 1;
                    LogI(" OK", true);          
                }     
            }        

            if (index >= 0){
                _devices[index]->setValuesFromPayload(payload);
            }
        }
    }

}	



HAPConfigValidationResult HAPPluginRF24::validateConfig(JsonObject object){

    /*
        {
            "enabled": true,
            "interval": 1000,
            "devices": [
                {
                    "id": 17,
                    "type": 1,
                    "name": "test1"
                }
            ]
        }
     */


    HAPConfigValidationResult result;

    result = HAPPlugin::validateConfig(object);
    if (result.valid == false) {
        return result;
    }

    result.valid = false;

       // plugin._name.devices
    if (object.containsKey("devices") && !object["devices"].is<JsonArray>()) {
        result.reason = "plugins." + _name + ".devices is not an array";
        return result;
    }

    // plugin._name.devices array
    uint8_t count = 0;
    for( const auto& value : object["devices"].as<JsonArray>() ) {
        
        // plugin._name.devices.count.id
        if (!value.containsKey("id") ) {
            result.reason = "plugins." + _name + ".devices." + String(count) + ".id is required";
            return result;
        }
        if (value.containsKey("id") && !value["id"].is<uint16_t>()) {
            result.reason = "plugins." + _name + ".devices." + String(count) + ".id is not an integer";
            return result;
        }   


        // plugin._name.devices.count.type
        if (!value.containsKey("type") ) {
            result.reason = "plugins." + _name + ".devices." + String(count) + ".type is required";
            return result;
        }
        if (value.containsKey("type") && !value["type"].is<uint8_t>()) {
            result.reason = "plugins." + _name + ".devices." + String(count) + ".type is not an integer";
            return result;
        }          

        // optional
        // plugin._name.devices.count.name
        if (value.containsKey("name") && !value["name"].is<const char*>()) {
            result.reason = "plugins." + _name + ".devices." + String(count) + ".name is not a string";
            return result;
        }   

        // plugin._name.devices.count.name - length
        if (value.containsKey("name")) {
            if (strlen(value["name"]) + 1 > HAP_STRING_LENGTH_MAX) {
                result.reason = "plugins." + _name + ".devices." + String(count) + ".name is too long";
                return result;
            } 
        }        
        count++;
    }

    result.valid = true;

    return result;
}

JsonObject HAPPluginRF24::getConfigImpl(){
    DynamicJsonDocument doc(HAP_ARDUINOJSON_BUFFER_SIZE / 8);
    JsonArray devices = doc.createNestedArray("devices");



    for (auto& dev : _devices){
        JsonObject devices_ = devices.createNestedObject();
        devices_["id"]      = dev->id;        
        devices_["name"]    = dev->name;
        devices_["tpye"]    = dev->type;
    }

    return doc.as<JsonObject>();
}

void HAPPluginRF24::setConfigImpl(JsonObject root){

    if (root.containsKey("devices")){        
        for (JsonVariant dev : root["devices"].as<JsonArray>()) {

            int index = indexOfDevice( dev["id"].as<uint16_t>());
            if ( index == -1 ){
                RemoteDeviceType type = (RemoteDeviceType)dev["type"].as<uint8_t>();
                
                // ToDo: Implement DHT Remote Device without pressure
                if (type == RemoteDeviceTypeWeather){
                    HAPPluginRF24DeviceWeather* newDevice = new HAPPluginRF24DeviceWeather(
                        dev["id"].as<uint16_t>(),
                        dev["name"].as<String>()
                    );

                    newDevice->setEventManager(_eventManager);
                    newDevice->setFakeGatoFactory(_fakeGatoFactory);            

                    _accessorySet->addAccessory(newDevice->initAccessory());
                        
                    _devices.push_back(newDevice);
                } else if (type == RemoteDeviceTypeDHT) {                                    
                    HAPPluginRF24DeviceDHT* newDevice = new HAPPluginRF24DeviceDHT(
                        dev["id"].as<uint16_t>(),
                        dev["name"].as<String>()
                    );

                    newDevice->setEventManager(_eventManager);
                    newDevice->setFakeGatoFactory(_fakeGatoFactory);            

                    _accessorySet->addAccessory(newDevice->initAccessory());
                        
                    _devices.push_back(newDevice);                
                }
            }     
        }
    }
}

int HAPPluginRF24::indexOfDevice(uint16_t id){

    int counter = 0;
    for (auto& dev : _devices){             
        if (dev->id == id){
            return counter;
        }
        counter++; 
    }
    return -1;
}