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

#define HAP_PLUGIN_RF24_INTERVAL    1000

#ifndef RF24_ADDRESS
#define RF24_ADDRESS        "HOMEKIT_RF24"
#endif 

// http://www.iotsharing.com/2018/03/esp-and-raspberry-connect-with-nrf24l01.html
// modified Lib: https://github.com/nhatuan84/RF24.git
// RF24          ESP32 (Feather)
// --------------------------
// CE         -> GPIO 13
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
    _radio = new RF24(13, 33, 5, 19, 18);
    
    if (_radio->begin() ){                          // Start up the radio    

        _radio->setPALevel(RF24_PA_MIN);            //You can set it as minimum or maximum 
                                                    // depending on the distance between the 
                                                    // transmitter and receiver.
        _radio->setAutoAck(1);                      // Ensure autoACK is enabled
        _radio->setRetries(15,15);                  // Max delay between retries & number of retries
        _radio->openReadingPipe(1, (const uint8_t*)RF24_ADDRESS);   // Write to device address 'SimpleNode'
        _radio->startListening();

        // ToDo: check if required!
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

        struct HAP_RF24_PAYLOAD payload;

        _radio->read( &payload, sizeof(struct HAP_RF24_PAYLOAD) );


        Serial.print("Got Payload from ");
        Serial.println(payload.id);

        Serial.print("type: ");
        Serial.println(payload.type);

        Serial.print("temp: ");
        Serial.println(payload.temp);

        Serial.print("hum: ");
        Serial.println(payload.hum);

        Serial.print("pres: ");
        Serial.println(payload.pres);

        Serial.print("Size of struct: ");
        Serial.println( sizeof(struct HAP_RF24_PAYLOAD) );        

        Serial.print("devices size: ");
        Serial.println(_devices.size());


        int index = indexOfDevice(payload.id);

        if ( index == -1 ){
            if (payload.type == (uint8_t)HAP_RF24_REMOTE_TYPE_WEATHER){
                
                HAPPluginRF24DeviceWeather* newDevice = new HAPPluginRF24DeviceWeather(
                    payload.id,
                    String(payload.id)                                
                );            
            
                LogI("RF24: Adding new remote weather device with id " + String(payload.id) + " ...", false);
                
                // Serial.printf("event: %p\n", _eventManager);
                // Serial.printf("fakegato: %p\n", _fakeGatoFactory);

                newDevice->setEventManager(_eventManager);
                newDevice->setFakeGatoFactory(_fakeGatoFactory);            

                _accessorySet->addAccessory(newDevice->initAccessory());
                
                _devices.push_back(newDevice);

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

            int index = indexOfDevice( dev["address"].as<uint8_t>());
            if ( index == -1 ){
                HAP_RF24_REMOTE_TYPE type = (HAP_RF24_REMOTE_TYPE)dev["type"].as<uint8_t>();
                
                if (type == HAP_RF24_REMOTE_TYPE_WEATHER) {
                    HAPPluginRF24DeviceWeather* newDevice = new HAPPluginRF24DeviceWeather(
                        dev["address"].as<uint8_t>(),
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

int HAPPluginRF24::indexOfDevice(uint8_t address){

    int counter = 0;
    for (auto& dev : _devices){             
        if (dev->address == address){
            return counter;
        }
        counter++; 
    }
    return -1;
}