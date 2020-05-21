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

#define VERSION_MAJOR       0
#define VERSION_MINOR       3
#define VERSION_REVISION    1
#define VERSION_BUILD       0


#define CE_PIN 7
#define CSN_PIN 8


#ifndef RF24_ADDRESS
#define RF24_ADDRESS        "HOMEKIT_RF24"
#endif 


#define HAP_PLUGIN_RF24_INTERVAL    1000





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
    _radio = new RF24(CE_PIN, CSN_PIN);

    _radio->begin();                            // Start up the radio
    _radio->setAutoAck(1);                      // Ensure autoACK is enabled
    _radio->setRetries(15,15);                  // Max delay between retries & number of retries
    _radio->openReadingPipe(1, (const uint8_t*)RF24_ADDRESS);   // Write to device address 'SimpleNode'
    _radio->startListening();

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

void HAPPluginRF24::setValue(int iid, String oldValue, String newValue){

}

void HAPPluginRF24::identify(bool oldValue, bool newValue){

}

void HAPPluginRF24::handleImpl(bool forced){
    _radio->stopListening();
    _radio->startListening();
    
    HAP_RF24_PAYLOAD payload;
    _radio->read( &payload, sizeof(HAP_RF24_PAYLOAD) );

    if(payload.id != 0){
        Serial.print("Got Payload ");
        Serial.println(payload.id);
        Serial.println(payload.temp);
    }
}	



HAPConfigValidationResult HAPPluginRF24::validateConfig(JsonObject object){

    /*
        {
            "enabled": true,
            "interval": 1000,
            "devices": [
                {
                    "address": 17,
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
        
        // plugin._name.devices.count.address
        if (!value.containsKey("address") ) {
            result.reason = "plugins." + _name + ".devices." + String(count) + ".address is required";
            return result;
        }
        if (value.containsKey("address") && !value["address"].is<uint8_t>()) {
            result.reason = "plugins." + _name + ".devices." + String(count) + ".address is not an integer";
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
        devices_["address"] = dev->address;        
        devices_["name"]    = dev->name;
        devices_["tpye"]    = dev->type;
    }

    return doc.as<JsonObject>();
}

void HAPPluginRF24::setConfigImpl(JsonObject root){

    if (root.containsKey("devices")){        
        for (JsonVariant dev : root["devices"].as<JsonArray>()) {

            HAP_RF24_REMOTE_TYPE type = (HAP_RF24_REMOTE_TYPE)dev["type"].as<uint8_t>();
            
            if (type == HAP_RF24_REMOTE_TYPE_WEATHER) {
                HAPPluginRF24DeviceWeather* newDevice = new HAPPluginRF24DeviceWeather(
                    dev["address"].as<uint8_t>(),
                    dev["name"].as<String>()
                );

                int index = indexOfDevice(newDevice);
                if ( index == -1 ){
                    _devices.push_back(newDevice);
                } else {
                    _devices[index] = newDevice;
                } 

            }



        
        }
    }
}

int HAPPluginRF24::indexOfDevice(HAPPluginRF24Device* device){
    // Check if element 22 exists in vector
	std::vector<HAPPluginRF24Device*>::iterator it = std::find(_devices.begin(), _devices.end(), device);
 
	if (it != _devices.end())
	{		
		// Get index of element from iterator
		return std::distance(_devices.begin(), it);		
	} else {
        return -1;
    }
}