//
// HAPPluginFanHoneywell.cpp
// Homekit
//
//  Created on: 01.08.2019
//      Author: michael
//
#include "HAPPluginFanHoneywell.hpp"
#include "HAPServer.hpp"
#include "HAPCustomCharacteristics+Services.hpp"

// https://www.nikolaus-lueneburg.de/2014/10/arduino-infrarot-sende-und-empfangsmodul-teil-1/
// #define HAP_IR_LED          14   // Add IR LED !!!

#define VERSION_MAJOR       0
#define VERSION_MINOR       3	// 2 = FakeGato support
#define VERSION_REVISION    1
#define VERSION_BUILD       1

// 
// Set these values in the HAPGlobals.hpp
// 
#ifndef HAP_PLUGIN_IR_DEVICE_FREQUENCY
#define HAP_PLUGIN_IR_DEVICE_FREQUENCY 38000
#endif

#ifndef HAP_PLUGIN_HONEYWELL_DELAY_SEND
#define HAP_PLUGIN_HONEYWELL_DELAY_SEND 	300  // in m
#endif


// Button 1: Wind Type 
// 3 Types: 
//      - Normal    : 1 Press
//      - Natural   : 2 Press
//      - Sleep     : 3 Press
// Encoding  : UNKNOWN
// Code      : 371A3C86 (32 bits)
// Timing[23]: 
//    +1300, - 350     +1300, - 400     + 450, -1200     +1300, - 350
//    +1300, - 400     + 450, -1200     + 450, -1200     + 500, -1200
//    + 450, -1200     +1300, - 350     + 500, -1200     + 450
uint16_t rawDataWindType[23] = {1300,350, 1300,400, 450,1200, 1300,350, 1300,400, 450,1200, 450,1200, 500,1200, 450,1200, 1300,350, 500,1200, 450};  // UNKNOWN 371A3C86

// Button 2: Timer
// 4 Settings:
//      - 0.5 h : 1 Press
//      -   1 h : 2 Press
//      -   2 h : 3 Press
//      -   4 h : 4 Press
// Encoding  : UNKNOWN
// Code      : E0984BB6 (32 bits)
// Timing[23]: 
//    +1300, - 350     +1300, - 400     + 450, -1200     +1300, - 350
//    +1300, - 400     + 450, -1200     + 450, -1200     + 500, -1200
//    +1300, - 350     + 500, -1200     + 450, -1200     + 450
uint16_t rawDataTimer[23] = {1300,350, 1300,400, 450,1200, 1300,350, 1300,400, 450,1200, 450,1200, 500,1200, 1300,350, 500,1200, 450,1200, 450};  // UNKNOWN E0984BB6

// Button 3: Oscillation
//      - On  : 1 Press
//      - Off : 2 Press
// Encoding  : UNKNOWN
// Code      : 39D41DC6 (32 bits)
// Timing[23]: 
//     +1300, - 400     +1250, - 400     + 450, -1200     +1300, - 400
//     +1250, - 400     + 450, -1200     + 450, -1250     +1250, - 400
//     + 450, -1200     + 500, -1200     + 450, -1200     + 450
uint16_t rawDataOscillation[23] = {1300,400, 1250,400, 450,1200, 1300,400, 1250,400, 450,1200, 450,1250, 1250,400, 450,1200, 500,1200, 450,1200, 450};  // UNKNOWN 39D41DC6

// Button 4: Speed Setting
// 3 Speeds:
//      - Low    : 1 Press  -  0 -  33 %
//      - Medium : 2 Press  - 34 -  66 % 
//      - High   : 3 Press  - 67 - 100 %
// Encoding  : UNKNOWN
// Code      : 143226DB (32 bits)
// Timing[23]: 
//     +1250, - 400     +1300, - 400     + 400, -1250     +1250, - 400
//     +1300, - 400     + 400, -1250     + 450, -1200     + 450, -1250
//     + 400, -1250     + 450, -1200     +1300, - 400     + 450
uint16_t rawDataSpeed[23] = {1250,400, 1300,400, 400,1250, 1250,400, 1300,400, 400,1250, 450,1200, 450,1250, 400,1250, 450,1200, 1300,400, 450};  // UNKNOWN 143226DB

// Button 5: On/Off
//      - On  : 1 Press
//      - Off : 2 Press
// Encoding  : UNKNOWN
// Code      : A32AB931 (32 bits)
// Timing[23]: 
//     +1300, - 350     +1300, - 400     + 450, -1200     +1300, - 350
//     +1300, - 400     + 450, -1200     + 450, -1250     + 450, -1200
//     + 450, -1200     + 500, -1200     + 450, -1200     +1300
uint16_t rawDataOnOff[23] = {1300,350, 1300,400, 450,1200, 1300,350, 1300,400, 450,1200, 450,1250, 450,1200, 450,1200, 500,1200, 450,1200, 1300};  // UNKNOWN A32AB931



HAPPluginFanHoneywell::HAPPluginFanHoneywell(){
    _type               = HAP_PLUGIN_TYPE_ACCESSORY;
    _name               = "FanHoneywell";
    _isEnabled          = HAP_PLUGIN_USE_FAN_HONEYWELL;
    _interval           = 0;
    _previousMillis     = 0;

    // _gpio               = HAP_IR_LED;

    _version.major      = VERSION_MAJOR;
    _version.minor      = VERSION_MINOR;
    _version.revision   = VERSION_REVISION;
    _version.build      = VERSION_BUILD;


    _isOn               = false;
    _activeState        = nullptr;
    
    _swingMode          = false;
    _swingModeState     = nullptr;

    _rotationSpeed      = 0;
    _rotationSpeedState = nullptr;

#if USE_CURRNT_FAN_STATE  
    _fanState           = 0;
	_currentFanState    = nullptr;
#endif        

    // _irsend             = nullptr;
}

void HAPPluginFanHoneywell::changeActive(uint8_t oldValue, uint8_t newValue) {
    LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Setting Active State " +  " oldValue: " + String(oldValue) + " -> newValue: " + String(newValue), true);    
    
    // _irsend->sendRaw(rawDataOnOff, 23, 38);   
    HAPPluginIR::getIRSend()->sendRaw(rawDataOnOff, 23, HAP_PLUGIN_IR_DEVICE_FREQUENCY);   

    _isOn = (bool)newValue;

    // Add event
    struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _activeState->iid, _activeState->value());							
    _eventManager->queueEvent( EventManager::kEventNotifyController, event); 
}

#if USE_CURRNT_FAN_STATE
void HAPPluginFanHoneywell::changeFanState(uint8_t oldValue, uint8_t newValue){
    LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Setting Fan State " +  " oldValue: " + String(oldValue) + " -> newValue: " + String(newValue), true);    
    
    //_irsend->sendRaw(rawDataOnOff, 23, 38);       
    HAPPluginIR::getIRSend()->sendRaw(rawDataOnOff, 23, HAP_PLUGIN_IR_DEVICE_FREQUENCY);   
    
    _fanState = newValue;   

    // Add event
    struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _currentFanState->iid, _currentFanState->value());							
    _eventManager->queueEvent( EventManager::kEventNotifyController, event); 
}
#endif

void HAPPluginFanHoneywell::changeSwingMode(uint8_t oldValue, uint8_t newValue){
    LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Setting Swing Mode " +  " oldValue: " + String(oldValue) + " -> newValue: " + String(newValue), true);    
    // _irsend->sendRaw(rawDataOscillation, 23, 38);  

    HAPPluginIR::getIRSend()->sendRaw(rawDataOscillation, 23, HAP_PLUGIN_IR_DEVICE_FREQUENCY);  
    _swingMode = (bool)newValue; 

    // Add event
    struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _swingModeState->iid, _swingModeState->value());							
    _eventManager->queueEvent( EventManager::kEventNotifyController, event); 
}

void HAPPluginFanHoneywell::changeRotationSpeed(float oldValue, float newValue){
    LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Setting rotation speed " +  " oldValue: " + String(oldValue) + " -> newValue: " + String(newValue), true);    


    uint8_t target = 0;
    if ((newValue > 0) && (newValue <= 33)) {
        target = 1;
    } else if ((newValue > 34) && (newValue <= 66)) {
        target = 2;
    } else if ((newValue > 67) && (newValue <= 100)) { 
        target = 3;
    }

    uint8_t numberOfPresses = 0;
    while (_rotationSpeed!= target) {
        numberOfPresses++;
        _rotationSpeed++;
        if (_rotationSpeed> 3){
            _rotationSpeed= 0;
        }
    }
    
    LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Press rotation speed button " + String(numberOfPresses) + " times", true);
    for (int i = 0; i < numberOfPresses; i++){
        // _irsend->sendRaw(rawDataSpeed, 23, 38); 
        HAPPluginIR::getIRSend()->sendRaw(rawDataSpeed, 23, HAP_PLUGIN_IR_DEVICE_FREQUENCY); 

        if (numberOfPresses > 1) {
            delay(HAP_PLUGIN_HONEYWELL_DELAY_SEND);
        }         
    }
	

    // Add event
    struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _rotationSpeedState->iid, _rotationSpeedState->value());							
    _eventManager->queueEvent( EventManager::kEventNotifyController, event); 
}


void HAPPluginFanHoneywell::handleImpl(bool forced){
    
    // LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Handle plguin [" + String(_interval) + "]", true);

    // Add event
    // struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _powerState->iid, _powerState->value());							
    // _eventManager->queueEvent( EventManager::kEventNotifyController, event);
}

bool HAPPluginFanHoneywell::begin(){
    
    HAPPluginIR::setupIRSend();
    return true;
}


HAPAccessory* HAPPluginFanHoneywell::initAccessory(){
 
	_accessory = new HAPAccessory();
	//HAPAccessory::addInfoServiceToAccessory(_accessory, "Builtin LED", "ACME", "LED", "123123123", &identify);
    auto callbackIdentify = std::bind(&HAPPlugin::identify, this, std::placeholders::_1, std::placeholders::_2);
    _accessory->addInfoService("Honeywell Fan v2", "ACME", "Fan v2", "123123123", callbackIdentify, version());


    // FAN v2 Service
    HAPService* _service = new HAPService(HAP_SERVICE_FANV2);
    _accessory->addService(_service);

    stringCharacteristics *fanServiceName = new stringCharacteristics(HAP_CHARACTERISTIC_NAME, permission_read, 32);
    fanServiceName->setValue("Fan v2");
    _accessory->addCharacteristics(_service, fanServiceName);


    // Active Characteristic - valid values 0, 1
    uint8_t validValuesActiveState[2] = {0,1};
    _activeState = new uint8Characteristics(HAP_CHARACTERISTIC_ACTIVE, permission_read|permission_write|permission_notify, 0, 1, 1, unit_none, 2, validValuesActiveState);
    if (_isOn)
        _activeState->setValue("1");
    else
        _activeState->setValue("0");
    
    auto callbackActiveState = std::bind(&HAPPluginFanHoneywell::changeActive, this, std::placeholders::_1, std::placeholders::_2);        
    _activeState->valueChangeFunctionCall = callbackActiveState;
    _accessory->addCharacteristics(_service, _activeState);    


#if USE_CURRNT_FAN_STATE
    // Current Fan State Characteristic - valid values 0 (inactive), 1 (idle), 2 (blowing air)
    uint8_t validValuesFanState[3] = {0,1,2};
    _currentFanState = new uint8Characteristics(HAP_CHARACTERISTIC_CURRENT_FAN_STATE, permission_read|permission_write|permission_notify, 0, 2, 1, unit_none, 2, validValuesFanState);
    if (_fanState == 0)
        _currentFanState->setValue("0");
    else if (_fanState == 1)
        _currentFanState->setValue("1");
    else
        _currentFanState->setValue("2");
    
    auto callbackCurrentFanState = std::bind(&HAPPluginFanHoneywell::changeFanState, this, std::placeholders::_1, std::placeholders::_2);        
    _currentFanState->valueChangeFunctionCall = callbackCurrentFanState;
    _accessory->addCharacteristics(_service, _currentFanState); 
#endif


    // Swing Mode Characteristic - valid values 0 (disabled), 1 (enabled)
    uint8_t validValuesSwingModeState[2] = {0,1};
    _swingModeState = new uint8Characteristics(HAP_CHARACTERISTIC_SWING_MODE, permission_read|permission_write|permission_notify, 0, 1, 1, unit_none, 2, validValuesSwingModeState);
    if (_swingMode)
        _swingModeState->setValue("1");
    else
        _swingModeState->setValue("0");
    
    auto callbackSwingModeState = std::bind(&HAPPluginFanHoneywell::changeSwingMode, this, std::placeholders::_1, std::placeholders::_2);        
    _swingModeState->valueChangeFunctionCall = callbackSwingModeState;
    _accessory->addCharacteristics(_service, _swingModeState); 


    // Rotation speed Characteristic     
    _rotationSpeedState = new floatCharacteristics(HAP_CHARACTERISTIC_ROTATION_SPEED, permission_read|permission_write|permission_notify, 0, 100, 1, unit_percentage);
    _rotationSpeedState->setValue("0");
    _rotationSpeed= 0;
    
    auto callbackSpeedState = std::bind(&HAPPluginFanHoneywell::changeRotationSpeed, this, std::placeholders::_1, std::placeholders::_2);        
    _rotationSpeedState->valueChangeFunctionCall = callbackSpeedState;
    _accessory->addCharacteristics(_service, _rotationSpeedState); 

    // 
    // FakeGato History
    // 
    // _fakegato.registerFakeGatoService(_accessory, "Honeywell");
	// auto callbackAddEntry = std::bind(&HAPPluginFanHoneywell::fakeGatoCallback, this);
	// _fakegatoFactory->registerFakeGato(&_fakegato,  "Honeywell", callbackAddEntry);


	LogD("OK", true);
	return _accessory;
}

void HAPPluginFanHoneywell::identify(bool oldValue, bool newValue) {
    printf("Start Identify Fan from member\n");
}

HAPConfigValidationResult HAPPluginFanHoneywell::validateConfig(JsonObject object){
    HAPConfigValidationResult result;
    
    result = HAPPlugin::validateConfig(object);
    if (result.valid == false) {
        return result;
    }
    // result.valid = false;
    
    // // plugin._name.username
    // if (object.containsKey("gpio") && !object["gpio"].is<uint8_t>()) {
    //     result.reason = "plugins." + _name + ".gpio is not an integer";
    //     return result;
    // }

    // result.valid = true;
    return result;
}

JsonObject HAPPluginFanHoneywell::getConfigImpl(){
    LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Get config implementation", true);

    DynamicJsonDocument doc(1);
    // doc["gpio"] = _gpio;

#if HAP_DEBUG_CONFIG
    serializeJson(doc, Serial);
    Serial.println();
#endif

    doc.shrinkToFit();
	return doc.as<JsonObject>();
}

void HAPPluginFanHoneywell::setConfigImpl(JsonObject root){
    // if (root.containsKey("gpio")){
    //     // LogD(" -- password: " + String(root["password"]), true);
    //     _gpio = root["gpio"].as<uint8_t>();
    // }
}

// bool HAPPluginFanHoneywell::fakeGatoCallback(){
//     // LogD(HAPServer::timeString() + " " + "HAPPluginPCA301Device" + "->" + String(__FUNCTION__) + " [   ] " + "fakeGatoCallback()", true);

//     // Serial.println("power: " + _curPowerValue->value());    
//     return _fakegato.addEntry(_powerState->value());
// }