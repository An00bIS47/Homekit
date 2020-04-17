//
// HAPPluginLED.cpp
// Homekit
//
//  Created on: 01.08.2019
//      Author: michael
//
#include "HAPPluginLED.hpp"
#include "HAPServer.hpp"

#define HAP_BLINK_INTERVAL 1000	

#if 0
#ifndef BUILTIN_LED
#define BUILTIN_LED 13
#endif
#else
#ifndef BUILTIN_LED
#define BUILTIN_LED 5
#endif
#endif


#ifndef HAP_LED_PIN
#define HAP_LED_PIN BUILTIN_LED
#endif

#define VERSION_MAJOR       0
#define VERSION_MINOR       0
#define VERSION_REVISION    3	// 2 = FakeGato support
#define VERSION_BUILD       1

#ifndef HAP_LED_ENABLE_BRIGHTNESS
#define HAP_LED_ENABLE_BRIGHTNESS   0
#endif


HAPPluginLED::HAPPluginLED(){
    _type = HAP_PLUGIN_TYPE_ACCESSORY;
    _name = "LED";
    _isEnabled = HAP_PLUGIN_USE_LED;
    _interval = HAP_BLINK_INTERVAL;
    _previousMillis = 0;
    _isOn = false;
    _gpio = HAP_LED_PIN;

    _version.major      = VERSION_MAJOR;
    _version.minor      = VERSION_MINOR;
    _version.revision   = VERSION_REVISION;
    _version.build      = VERSION_BUILD;
}

void HAPPluginLED::changePower(bool oldValue, bool newValue) {
    // LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Setting iid " + String(iid) +  " oldValue: " + oldValue + " -> newValue: " + newValue, true);

    if (newValue == true) {
        digitalWrite(_gpio, HIGH);     // dont know why to put low here, maybe because of SPI ?  
    } else {
        digitalWrite(_gpio, LOW);
    }      
}

void HAPPluginLED::changeBrightness(int oldValue, int newValue){
    printf("New brightness state: %d\n", newValue);
}

void HAPPluginLED::handleImpl(bool forced){
    
    LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Handle plguin [" + String(_interval) + "]", true);

    if (_isOn) {            
        setValue(_powerState->iid, "1", "0");
    } else {
        setValue(_powerState->iid, "0", "1");            
    }

    // Add event
    // struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _powerState->iid, _powerState->value());							
    // _eventManager->queueEvent( EventManager::kEventNotifyController, event);

#if HAP_LED_ENABLE_BRIGHTNESS  
    uint32_t freeMem = ESP.getFreeHeap();        
    uint8_t percentage = ( freeMem * 100) / 245000;        
    // _brightnessState->setValue(String(percentage));
    setValue(charType_brightness, _brightnessState->getValue(), percentage);

    // Add event
    // struct HAPEvent eventB = HAPEvent(nullptr, _accessory->aid, _brightnessState->iid, _brightnessState->value());							
    // _eventManager->queueEvent( EventManager::kEventNotifyController, eventB);
#endif        


}

bool HAPPluginLED::begin(){
    pinMode(_gpio, OUTPUT);    
    digitalWrite(_gpio, _isOn);

    return true;
}
HAPAccessory* HAPPluginLED::initAccessory(){
 

	_accessory = new HAPAccessory();
	//HAPAccessory::addInfoServiceToAccessory(_accessory, "Builtin LED", "ACME", "LED", "123123123", &identify);
    auto callbackIdentify = std::bind(&HAPPlugin::identify, this, std::placeholders::_1, std::placeholders::_2);
    _accessory->addInfoService("Builtin LED", "ACME", "LED", "123123123", callbackIdentify, version());

    HAPService* _service = new HAPService(HAP_SERVICE_LIGHTBULB);
    _accessory->addService(_service);

    stringCharacteristics *lightServiceName = new stringCharacteristics(HAP_CHARACTERISTIC_NAME, permission_read, 32);
    lightServiceName->setValue("LED");
    _accessory->addCharacteristics(_service, lightServiceName);

    _powerState = new boolCharacteristics(HAP_CHARACTERISTIC_ON, permission_read|permission_write|permission_notify);
    if (_isOn)
        _powerState->setValue("1");
    else
        _powerState->setValue("0");
    
    auto callbackPowerState = std::bind(&HAPPluginLED::changePower, this, std::placeholders::_1, std::placeholders::_2);        
    _powerState->valueChangeFunctionCall = callbackPowerState;
    _accessory->addCharacteristics(_service, _powerState);

    
    _brightnessState = new intCharacteristics(HAP_CHARACTERISTIC_BRIGHTNESS, permission_read|permission_write|permission_notify, 0, 100, 1, unit_percentage);
        //_brightnessState->valueChangeFunctionCall = &changeBrightness;

#if HAP_LED_ENABLE_BRIGHTNESS   
    _brightnessState->setValue("50");
    auto callbackBrightness = std::bind(&HAPPluginLED::changeBrightness, this, std::placeholders::_1, std::placeholders::_2);        
    _brightnessState->valueChangeFunctionCall = callbackBrightness;
    _accessory->addCharacteristics(_service, _brightnessState);    
#endif

	LogD("OK", true);

	return _accessory;
}


void HAPPluginLED::setValue(int iid, String oldValue, String newValue){
    LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Setting iid " + String(iid) +  " oldValue: " + oldValue + " -> newValue: " + newValue, true);

     if (iid == _powerState->iid) {
        
        if (newValue == "1"){
            _isOn = true;
        } else {
            _isOn = false;
        }    

        _powerState->setValue(newValue);

        struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _powerState->iid, _powerState->value());							
	    _eventManager->queueEvent( EventManager::kEventNotifyController, event);
 
    }
    //  else if (type == charType_brightness) {        
    //     _brightnessState->setValue(newValue);

    //     struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _brightnessState->iid, _brightnessState->value());							
	// 	_eventManager->queueEvent( EventManager::kEventNotifyController, event);
    // }

}



String HAPPluginLED::getValue(int iid){
    if (iid == _powerState->iid) {
        return _powerState->value();
    } 
    // else if (type == charType_brightness) {
    //     return _brightnessState->value();
    // }
    return "";
}

void HAPPluginLED::identify(bool oldValue, bool newValue) {
    printf("Start Identify Light from member\n");
}

HAPConfigValidationResult HAPPluginLED::validateConfig(JsonObject object){
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

JsonObject HAPPluginLED::getConfigImpl(){
    DynamicJsonDocument doc(128);
    doc["gpio"] = _gpio;

	return doc.as<JsonObject>();
}

void HAPPluginLED::setConfigImpl(JsonObject root){
    if (root.containsKey("gpio")){
        // LogD(" -- password: " + String(root["password"]), true);
        _gpio = root["gpio"].as<uint8_t>();
    }
}