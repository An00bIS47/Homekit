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
#define VERSION_REVISION    2
#define VERSION_BUILD       0

#define HAP_PLUGIN_RF24_INTERVAL    250
#define HAP_PLUGIN_RF24_TIMEOUT     200

#ifndef HAP_PLUGIN_RF24_ADDRESS
#define HAP_PLUGIN_RF24_ADDRESS        "HOMEKIT_RF24"
#endif 

#define HAP_PLUGIN_RF24_PA_LEVEL       RF24_PA_MAX
#define HAP_PLUGIN_RF24_DATA_RATE      RF24_1MBPS

#define HAP_PLUGIN_UPDATE_SETTINGS_MESSAGE_COUNT    10  // request settings from remote device every X messages
                                                        // Setting to 0 disable this
                                                        // default 0

// http://www.iotsharing.com/2018/03/esp-and-raspberry-connect-with-nrf24l01.html
// modified Lib: https://github.com/nhatuan84/RF24.git
// RF24          ESP32 (Feather)
// --------------------------
//  NRF24 Pinout:
//        +-+-+
//  GND ->|_| |<- VCC
//   CE ->| | |<- CSN
//  SCK ->| | |<- MOSI
// MISO ->| | |<- IRQ
//        +-+-+   
// 
//               ESP32                Breakout Board
// =====================================================
// CE         -> GPIO 12                -> CE
// CSN        -> GPIO 33                -> CSN
// CLK        -> GPIO 5                 -> SCK
// MISO       -> GPIO 19                -> MI
// MOSI       -> GPIO 18                -> MO
// IRQ        -> not connected          -> -
// 

#define HAP_PLUGIN_RF24_PIN_CE      12
#define HAP_PLUGIN_RF24_PIN_CSN     33
#define HAP_PLUGIN_RF24_PIN_CLK     5
#define HAP_PLUGIN_RF24_PIN_MISO    19
#define HAP_PLUGIN_RF24_PIN_MOSI    18


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
    _awaitSettingsConfirmation = false;
    
    _isInitialized = false;
    _radio = nullptr;


    _messageCounter = 0;
}

HAPPluginRF24::~HAPPluginRF24(){
    if (_radio) {
        delete _radio;
    }    
}


bool HAPPluginRF24::begin() {

    if (_isInitialized) return true;
    
    
    _radio = new RF24(HAP_PLUGIN_RF24_PIN_CE, HAP_PLUGIN_RF24_PIN_CSN, HAP_PLUGIN_RF24_PIN_CLK, HAP_PLUGIN_RF24_PIN_MISO, HAP_PLUGIN_RF24_PIN_MOSI);
    //_radio = new RF24(HAP_PLUGIN_RF24_PIN_CE, HAP_PLUGIN_RF24_PIN_CSN);
    
    if (_radio->begin() ){                          // Start up the radio    

        _radio->setPALevel(HAP_PLUGIN_RF24_PA_LEVEL);          // You can set it as minimum or maximum 
                                                                // depending on the distance between the 
                                                                // transmitter and receiver.
        _radio->setDataRate(HAP_PLUGIN_RF24_DATA_RATE);                                                    
        // _radio->setAutoAck(1);    
        
        _radio->enableDynamicPayloads();
        _radio->enableAckPayload();                 // Enable Ack Payload
        _radio->setRetries(5, 15);                  // delay, count
                                                    // 5 gives a 1500 µsec delay which is needed for a 32 byte ackPayload

        
        _radio->openReadingPipe(1, (const uint8_t*)HAP_PLUGIN_RF24_ADDRESS);   // Write to device address 'SimpleNode'
        _radio->startListening();
        _isEnabled = true; 

#if HAP_DEBUG_RF24
        _radio->printDetails();
#endif
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

    LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Handle plguin [" + String(_interval) + "]", true);


    // _radio->stopListening();
    // _radio->startListening();

    
    if (_radio->available()){
        
        _messageCounter++;
        bool newDeviceAdded = false;

        while(_radio->available()){            

            struct RadioPacket payload;
            _radio->read( &payload, sizeof(RadioPacket) );


#if HAP_DEBUG_RF24
            LogD(HAPServer::timeString() + " Got Payload from id: " + String(payload.radioId, HEX), true);        
            LogD(HAPServer::timeString() + "   type:    " + String(payload.type), true);
            LogD(HAPServer::timeString() + "   temp:    " + String(payload.temperature), true);
            LogD(HAPServer::timeString() + "   hum:     " + String(payload.humidity), true);
            LogD(HAPServer::timeString() + "   pres:    " + String(payload.pressure), true);
            LogD(HAPServer::timeString() + "   voltage: " + String(payload.voltage), true);


            LogD(HAPServer::timeString() + "   Size of struct: " + String(sizeof(RadioPacket)), true);        
            LogD(HAPServer::timeString() + " Number of devices: " + String(_devices.size()), true);
#endif        

            // Validate values
            if (payload.humidity > 10000) {
                LogW("WARNING: Got invalid payload.humidity from remote device!", true);
                return;
            } else if (!(payload.temperature >= -5000 && payload.temperature <= 15000)) {
                LogW("WARNING: Got invalid payload.temperature from remote device!", true);
                return;
            } else if (payload.type > 2) {
                LogW("WARNING: Got invalid payload.type from remote device!", true);
                return;
            } else if (payload.radioId == 0) {
                LogW("WARNING: Got invalid payload.id from remote device!", true);
                return;
            } else {

                // Process payload
                int index = indexOfDevice(payload.radioId);


                // device not known -> add it
                if ( index == -1 ){
                    if (payload.type == (uint8_t)RemoteDeviceTypeWeather){
                        
                        LogI(HAPServer::timeString() + " RF24: Adding new remote weather device with id " + String(payload.radioId, HEX) + " ...", false);

                        HAPPluginRF24DeviceWeather* newDevice = new HAPPluginRF24DeviceWeather(
                            payload.radioId,
                            String(payload.radioId),
                            0                                
                        );            
                    
                        NewSettingsPacket newSettings;
                        newSettings.forRadioId = payload.radioId;
                        newSettings.changeType = ChangeTypeNone;
                        newSettings.newRadioId = 0;
                        newSettings.newSleepInterval = 0;
                        newSettings.newMeasureMode = 0;
                        

                        LogD("Preload ack data for next transmission", true);
                        _radio->writeAckPayload(1, &newSettings, sizeof(NewSettingsPacket)); // pre-load data                            
                        newDeviceAdded = true;

                        newDevice->setEventManager(_eventManager);
                        newDevice->setFakeGatoFactory(_fakeGatoFactory);            


                        // Set callback for sending updated settings
                        auto callbackSendSettings = std::bind(&HAPPluginRF24::updateSettings, this, std::placeholders::_1);        
                        newDevice->setSendSettingsCallback(callbackSendSettings);

                        _accessorySet->addAccessory(newDevice->initAccessory());
                        
                        _devices.push_back(newDevice);
                        
                        // ToDo: Check if neccessay here because all infos are stored by the remote device                                 
                        // _eventManager->queueEvent( EventManager::kEventUpdatedConfig, HAPEvent(), EventManager::kLowPriority);        
                        

                        // index = indexOfDevice(payload.id);    
                        index = _devices.size() - 1;
                        LogI(" OK", true);          
                    } else if (payload.type == (uint8_t)RemoteDeviceTypeDHT){
                        
                        LogI(HAPServer::timeString() + " RF24: Adding new remote DHT device with id " + String(payload.radioId, HEX) + " ...", false);                    

                        HAPPluginRF24DeviceDHT* newDevice = new HAPPluginRF24DeviceDHT(
                            payload.radioId,
                            String(payload.radioId),
                            0                                
                        );            
                    
                        NewSettingsPacket newSettings;
                        newSettings.forRadioId = payload.radioId;
                        newSettings.changeType = ChangeTypeNone;
                        newSettings.newRadioId = 0;
                        newSettings.newSleepInterval = 0;
                        newSettings.newMeasureMode = 0;
                        
                        _radio->writeAckPayload(newSettings.forRadioId, &newSettings, sizeof(NewSettingsPacket)); // pre-load data                            
                        newDeviceAdded = true;

                        newDevice->setEventManager(_eventManager);
                        newDevice->setFakeGatoFactory(_fakeGatoFactory);            


                        // Set callback for sending updated settings
                        auto callbackSendSettings = std::bind(&HAPPluginRF24::updateSettings, this, std::placeholders::_1);        
                        newDevice->setSendSettingsCallback(callbackSendSettings);

                        _accessorySet->addAccessory(newDevice->initAccessory());
                        
                        _devices.push_back(newDevice);

                        // ToDo: Check if neccessay here because all infos are stored by the remote device                   
                        // _eventManager->queueEvent( EventManager::kEventUpdatedConfig, HAPEvent(), EventManager::kLowPriority);                                
                        
                        index = _devices.size() - 1;
                        LogI(" OK", true);          
                    }     
                    
                }        

                // set values for the device
                if (index >= 0){
                    _devices[index]->setValuesFromPayload(payload);
                }
            }
       
            // awaits settings from the device 
            if (_awaitSettingsConfirmation) {        

                LogD("Awaiting settings confirmation", true);           
                _awaitSettingsConfirmation = false;              

                // read the settings from the payload
                // !! _awaitSettingsConfirmation can be updated during this process !!
                // !! set it to false above this line !!
                Serial.print("readSettings from device: ");
                Serial.println(readSettingsFromRadio());                                                                                                       
            }          
            
            if (newDeviceAdded) {                
                _awaitSettingsConfirmation = true;
            } 


            if (HAP_PLUGIN_UPDATE_SETTINGS_MESSAGE_COUNT > 0){
                if (_messageCounter == HAP_PLUGIN_UPDATE_SETTINGS_MESSAGE_COUNT){
                    
                    _messageCounter = 0;
                    LogD("Pre-Load settings config every 5 messages", true);  

                    NewSettingsPacket newSettings;
                    newSettings.forRadioId = payload.radioId;
                    newSettings.changeType = ChangeTypeNone;
                    newSettings.newRadioId = 0;
                    newSettings.newSleepInterval = 0;
                    newSettings.newMeasureMode = 0;

                    updateSettings(newSettings);
                }
            }

        }
    }
}	

bool HAPPluginRF24::readSettingsFromRadio(){
    uint16_t timeoutCounter = 0;

    while (!_radio->available()){
        Serial.print(".");
        delay(1);
        timeoutCounter++;

        if (timeoutCounter >= HAP_PLUGIN_RF24_TIMEOUT) {       
            Serial.println("Timed out");      
            break;
        }
    }
    Serial.println("");


    if (_radio->available()){

        struct RemoteDeviceSettings remoteSettings;
        _radio->read( &remoteSettings, sizeof(RemoteDeviceSettings) );

#if HAP_DEBUG_RF24
        LogD(HAPServer::timeString() + " Got Settings from id: " + String(remoteSettings.radioId, HEX), true);        
        LogD(HAPServer::timeString() + "   sleepInterval:    " + String(remoteSettings.sleepInterval), true);
        LogD(HAPServer::timeString() + "   measureMode:      " + String(remoteSettings.measureMode), true);
        LogD(HAPServer::timeString() + "   settings version: " + String(remoteSettings.version), true);
        LogD(HAPServer::timeString() + "   firmware version: " + String(remoteSettings.firmware_version), true);
                        
        LogD(HAPServer::timeString() + "   Size of struct: " + String(sizeof(RemoteDeviceSettings)), true);                        
#endif                  

        int indexSettings = indexOfDevice(remoteSettings.radioId);
        if (indexSettings >= 0){
            _devices[indexSettings]->setSettingsFromPayload(remoteSettings);
            return true;
        }
        

    }

    return false;
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
                    "name": "test1",
                    "measureMode": 1,
                    "sleepInterval": 1
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

        // plugin._name.devices.count.measureMode
        if (!value.containsKey("measureMode") ) {
            result.reason = "plugins." + _name + ".devices." + String(count) + ".measureMode is required";
            return result;
        }

        if (value.containsKey("measureMode") && !value["measureMode"].is<uint8_t>()) {
            result.reason = "plugins." + _name + ".devices." + String(count) + ".measureMode is not an integer";
            return result;
        }    

        if (value["measureMode"].as<int>() < 0 || value["measureMode"].as<int>() > 1) {
            result.reason = "plugins." + _name + ".devices." + String(count) + ".measureMode is not a valid value";
            return result;
        }

        // plugin._name.devices.count.sleepInterval
        if (!value.containsKey("sleepInterval") ) {
            result.reason = "plugins." + _name + ".devices." + String(count) + ".sleepInterval is required";
            return result;
        }

        if (value.containsKey("sleepInterval") && !value["sleepInterval"].is<uint8_t>()) {
            result.reason = "plugins." + _name + ".devices." + String(count) + ".sleepInterval is not an integer";
            return result;
        }    

        if (value["sleepInterval"].as<int>() < 0 || value["sleepInterval"].as<int>() > 15) {
            result.reason = "plugins." + _name + ".devices." + String(count) + ".sleepInterval is not a valid value";
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

    LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Get config implementation", true);
    DynamicJsonDocument doc(1);    

//     DynamicJsonDocument doc(2048);
//     JsonArray devices = doc.createNestedArray("devices");

//     for (auto& dev : _devices){
//         JsonObject devices_ = devices.createNestedObject();
//         devices_["id"]              = dev->id;        
//         devices_["name"]            = dev->name;
//         devices_["type"]            = dev->type;
//         devices_["measureMode"]     = (uint8_t)dev->measureMode;
//         devices_["sleepInterval"]   = (uint8_t)dev->sleepInterval;
//     }

// #if HAP_DEBUG_CONFIG
//     serializeJson(doc, Serial);
//     Serial.println();
// #endif

//     doc.shrinkToFit();
    return doc.as<JsonObject>();
}

void HAPPluginRF24::setConfigImpl(JsonObject root){

    // ToDo: A duplicated accessory is made, when loaded from config??
    /*
    if (root.containsKey("devices")){    

        begin();

        for (JsonVariant dev : root["devices"].as<JsonArray>()) {

            int index = indexOfDevice( dev["id"].as<uint16_t>());
            if ( index == -1 ){
                RemoteDeviceType type = (RemoteDeviceType)dev["type"].as<uint8_t>();
                
                
                if (type == RemoteDeviceTypeWeather){
                    HAPPluginRF24DeviceWeather* newDevice = new HAPPluginRF24DeviceWeather(
                        dev["id"].as<uint16_t>(),
                        dev["name"].as<String>(),
                        dev["measureMode"].as<uint8_t>()
                    );

                    NewSettingsPacket newSettings;
                    newSettings.forRadioId = dev["id"].as<uint16_t>();
                    newSettings.changeType = ChangeTypeNone;
                    newSettings.newRadioId = 0;
                    newSettings.newSleepInterval = 0;
                    newSettings.newMeasureMode = 0;
                    
                    _radio->writeAckPayload(newSettings.forRadioId, &newSettings, sizeof(NewSettingsPacket)); // pre-load data                            
                    _awaitSettingsConfirmation = true;

                    newDevice->setEventManager(_eventManager);
                    newDevice->setFakeGatoFactory(_fakeGatoFactory);            


                    // Set callback for sending updated settings
                    auto callbackSendSettings = std::bind(&HAPPluginRF24::updateSettings, this, std::placeholders::_1);        
                    newDevice->setSendSettingsCallback(callbackSendSettings);


                    _accessorySet->addAccessory(newDevice->initAccessory());
                    
                    _devices.push_back(newDevice);
                } else if (type == RemoteDeviceTypeDHT) {                                    
                    HAPPluginRF24DeviceDHT* newDevice = new HAPPluginRF24DeviceDHT(
                        dev["id"].as<uint16_t>(),
                        dev["name"].as<String>(),
                        dev["measureMode"].as<uint8_t>()                                
                    );            
                
                    NewSettingsPacket newSettings;
                    newSettings.forRadioId = dev["id"].as<uint16_t>();
                    newSettings.changeType = ChangeTypeNone;
                    newSettings.newRadioId = 0;
                    newSettings.newSleepInterval = 0;
                    newSettings.newMeasureMode = 0;
                    
                    _radio->writeAckPayload(newSettings.forRadioId, &newSettings, sizeof(NewSettingsPacket)); // pre-load data                            
                    _awaitSettingsConfirmation = true;

                    newDevice->setEventManager(_eventManager);
                    newDevice->setFakeGatoFactory(_fakeGatoFactory);            


                    // Set callback for sending updated settings
                    auto callbackSendSettings = std::bind(&HAPPluginRF24::updateSettings, this, std::placeholders::_1);        
                    newDevice->setSendSettingsCallback(callbackSendSettings);


                    _accessorySet->addAccessory(newDevice->initAccessory());
                    
                    _devices.push_back(newDevice);                                                                                                       
                }
            }     
        }
    }
    */
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


void HAPPluginRF24::updateSettings(NewSettingsPacket newSettings){
    _radio->writeAckPayload(1, &newSettings, sizeof(NewSettingsPacket)); // pre-load data    

    _awaitSettingsConfirmation = true;
}