//
// HAPPluginHoneywell.cpp
// Homekit
//
//  Created on: 01.08.2019
//      Author: michael
//
#include "HAPPluginHoneywell.hpp"
#include "HAPServer.hpp"


// https://www.nikolaus-lueneburg.de/2014/10/arduino-infrarot-sende-und-empfangsmodul-teil-1/
#define HAP_IR_LED          0   // Add IR LED !!!

#define VERSION_MAJOR       0
#define VERSION_MINOR       3	// 2 = FakeGato support
#define VERSION_REVISION    1
#define VERSION_BUILD       1


// Button 1: Wind Type 
// 3 Types: 
//      - Normal    : 1 Press
//      - Nattural  : 2 Press
//      - Sleep     : 3 Press
uint16_t rawDataWindType[23] = {1300,350, 1300,400, 450,1200, 1300,350, 1300,400, 450,1200, 450,1200, 500,1200, 450,1200, 1300,350, 500,1200, 450};  // UNKNOWN 371A3C86

// Button 2: Timer
// 4 Settings:
//      - 0.5 h : 1 Press
//      -   1 h : 2 Press
//      -   2 h : 3 Press
//      -   4 h : 4 Press
uint16_t rawDataTimer[23] = {1300,350, 1300,400, 450,1200, 1300,350, 1300,400, 450,1200, 450,1200, 500,1200, 1300,350, 500,1200, 450,1200, 450};  // UNKNOWN E0984BB6

// Button 3: Oscillation
//      - On  : 1 Press
//      - Off : 2 Press
uint16_t rawDataOscillation[23] = {1300,400, 1250,400, 450,1200, 1300,400, 1250,400, 450,1200, 450,1250, 1250,400, 450,1200, 500,1200, 450,1200, 450};  // UNKNOWN 39D41DC6

// Button 4: Speed Setting
// 3 Speeds:
//      - Low    : 1 Press   
//      - Medium : 2 Press
//      - High   : 3 Press
uint16_t rawDataSpeed[23] = {1250,400, 1300,400, 400,1250, 1250,400, 1300,400, 400,1250, 450,1200, 450,1250, 400,1250, 450,1200, 1300,400, 450};  // UNKNOWN 143226DB

// Button 5: On/Off
//      - On  : 1 Press
//      - Off : 2 Press
uint16_t rawDataOnOff[23] = {1300,350, 1300,400, 450,1200, 1300,350, 1300,400, 450,1200, 450,1250, 450,1200, 450,1200, 500,1200, 450,1200, 1300};  // UNKNOWN A32AB931



HAPPluginHoneywell::HAPPluginHoneywell(){
    _type               = HAP_PLUGIN_TYPE_ACCESSORY;
    _name               = "Honeywell";
    _isEnabled          = HAP_PLUGIN_USE_HONEYWELL;
    _interval           = 0;
    _previousMillis     = 0;

    _isOn               = false;
    _fanState           = 0;
    _swingMode          = false;

    _gpio               = HAP_IR_LED;

    _version.major      = VERSION_MAJOR;
    _version.minor      = VERSION_MINOR;
    _version.revision   = VERSION_REVISION;
    _version.build      = VERSION_BUILD;

    _activeState        = nullptr;
	_currentFanState    = nullptr;
    _swingModeState     = nullptr;

    _irsend             = nullptr;
}

void HAPPluginHoneywell::changeActive(uint8_t oldValue, uint8_t newValue) {
    LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Setting Active State " +  " oldValue: " + String(oldValue) + " -> newValue: " + String(newValue), true);    
    _irsend->sendGC(rawDataOnOff, 23);   

    _isOn = (bool)newValue;
}


void HAPPluginHoneywell::changeFanState(uint8_t oldValue, uint8_t newValue){
    LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Setting Fan State " +  " oldValue: " + String(oldValue) + " -> newValue: " + String(newValue), true);    
    _irsend->sendGC(rawDataOnOff, 23);   

    _fanState = newValue;    
}

void HAPPluginHoneywell::changeSwingMode(uint8_t oldValue, uint8_t newValue){
    LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Setting Swing Mode " +  " oldValue: " + String(oldValue) + " -> newValue: " + String(newValue), true);    
    _irsend->sendGC(rawDataOscillation, 23);  
    _swingMode = (bool)newValue; 
}


void HAPPluginHoneywell::handleImpl(bool forced){
    
    // LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Handle plguin [" + String(_interval) + "]", true);

    // Add event
    // struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _powerState->iid, _powerState->value());							
    // _eventManager->queueEvent( EventManager::kEventNotifyController, event);
}

bool HAPPluginHoneywell::begin(){

    _irsend = new IRsend(_gpio);
    _irsend->begin();  

    return true;
}


HAPAccessory* HAPPluginHoneywell::initAccessory(){
 
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
    
    auto callbackActiveState = std::bind(&HAPPluginHoneywell::changeActive, this, std::placeholders::_1, std::placeholders::_2);        
    _activeState->valueChangeFunctionCall = callbackActiveState;
    _accessory->addCharacteristics(_service, _activeState);    


    // Current Fan State Characteristic - valid values 0 (inactive), 1 (idle), 2 (blowing air)
    uint8_t validValuesFanState[3] = {0,1,2};
    _currentFanState = new uint8Characteristics(HAP_CHARACTERISTIC_CURRENT_FAN_STATE, permission_read|permission_write|permission_notify, 0, 2, 1, unit_none, 2, validValuesFanState);
    if (_fanState == 0)
        _currentFanState->setValue("0");
    else if (_fanState == 1)
        _currentFanState->setValue("1");
    else
        _currentFanState->setValue("2");
    
    auto callbackCurrentFanState = std::bind(&HAPPluginHoneywell::changeFanState, this, std::placeholders::_1, std::placeholders::_2);        
    _currentFanState->valueChangeFunctionCall = callbackCurrentFanState;
    _accessory->addCharacteristics(_service, _currentFanState); 



    // Swing Mode Characteristic - valid values 0 (disabled), 1 (enabled)
    uint8_t validValuesSwingModeState[2] = {0,1};
    _swingModeState = new uint8Characteristics(HAP_CHARACTERISTIC_SWING_MODE, permission_read|permission_write|permission_notify, 0, 1, 1, unit_none, 2, validValuesSwingModeState);
    if (_swingMode)
        _currentFanState->setValue("1");
    else
        _currentFanState->setValue("0");
    
    auto callbackSwingModeState = std::bind(&HAPPluginHoneywell::changeSwingMode, this, std::placeholders::_1, std::placeholders::_2);        
    _swingModeState->valueChangeFunctionCall = callbackSwingModeState;
    _accessory->addCharacteristics(_service, _swingModeState); 

    // 
    // FakeGato History
    // 
    // _fakegato.registerFakeGatoService(_accessory, "Honeywell");
	// auto callbackAddEntry = std::bind(&HAPPluginHoneywell::fakeGatoCallback, this);
	// _fakegatoFactory->registerFakeGato(&_fakegato,  "Honeywell", callbackAddEntry);


	LogD("OK", true);
	return _accessory;
}


// void HAPPluginHoneywell::setValue(int iid, String oldValue, String newValue){
//     LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Setting iid " + String(iid) +  " oldValue: " + oldValue + " -> newValue: " + newValue, true);

//      if (iid == _powerState->iid) {
        
//         if (newValue == "1"){
//             _isOn = true;
//         } else {
//             _isOn = false;
//         }    

//         _powerState->setValue(newValue);

//         struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _powerState->iid, _powerState->value());							
// 	    _eventManager->queueEvent( EventManager::kEventNotifyController, event);
 
//     }
//     //  else if (type == charType_brightness) {        
//     //     _brightnessState->setValue(newValue);

//     //     struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _brightnessState->iid, _brightnessState->value());							
// 	// 	_eventManager->queueEvent( EventManager::kEventNotifyController, event);
//     // }

// }



// String HAPPluginHoneywell::getValue(int iid){
//     if (iid == _powerState->iid) {
//         return _powerState->value();
//     } 
//     // else if (type == charType_brightness) {
//     //     return _brightnessState->value();
//     // }
//     return "";
// }

void HAPPluginHoneywell::identify(bool oldValue, bool newValue) {
    printf("Start Identify Fan from member\n");
}

HAPConfigValidationResult HAPPluginHoneywell::validateConfig(JsonObject object){
    HAPConfigValidationResult result;
    
    result = HAPPlugin::validateConfig(object);
    if (result.valid == false) {
        return result;
    }
    result.valid = false;
    
    // plugin._name.username
    if (object.containsKey("gpio") && !object["gpio"].is<uint8_t>()) {
        result.reason = "plugins." + _name + ".gpio is not an integer";
        return result;
    }

    result.valid = true;
    return result;
}

JsonObject HAPPluginHoneywell::getConfigImpl(){
    DynamicJsonDocument doc(128);
    doc["gpio"] = _gpio;

	return doc.as<JsonObject>();
}

void HAPPluginHoneywell::setConfigImpl(JsonObject root){
    if (root.containsKey("gpio")){
        // LogD(" -- password: " + String(root["password"]), true);
        _gpio = root["gpio"].as<uint8_t>();
    }
}

// bool HAPPluginHoneywell::fakeGatoCallback(){
//     // LogD(HAPServer::timeString() + " " + "HAPPluginPCA301Device" + "->" + String(__FUNCTION__) + " [   ] " + "fakeGatoCallback()", true);

//     // Serial.println("power: " + _curPowerValue->value());    
//     return _fakegato.addEntry(_powerState->value());
// }