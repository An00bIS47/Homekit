//
// HAPPluginIR.cpp
// Homekit
//
//  Created on: 01.08.2019
//      Author: michael
//
#include "HAPPluginIR.hpp"
#include "HAPServer.hpp"



#define VERSION_MAJOR       0
#define VERSION_MINOR       3	// 2 = FakeGato support
#define VERSION_REVISION    1
#define VERSION_BUILD       1


IRsend* HAPPluginIR::_irsend;
uint8_t HAPPluginIR::_gpioIRSend = HAP_IR_LED_PIN;


HAPPluginIR::HAPPluginIR(){
    _type               = HAP_PLUGIN_TYPE_OTHER;
    _name               = "IR";
    _isEnabled          = HAP_PLUGIN_USE_IR;
    _interval           = 0;
    _previousMillis     = 0;    
    
    
    _gpioIRSend         = HAP_IR_LED_PIN;

    _version.major      = VERSION_MAJOR;
    _version.minor      = VERSION_MINOR;
    _version.revision   = VERSION_REVISION;
    _version.build      = VERSION_BUILD;

}




void HAPPluginIR::handleImpl(bool forced){
       
}

bool HAPPluginIR::begin(){
    return true;
}


HAPAccessory* HAPPluginIR::initAccessory(){
    return nullptr;
}




HAPConfigValidationResult HAPPluginIR::validateConfig(JsonObject object){
    HAPConfigValidationResult result;
    
    result = HAPPlugin::validateConfig(object);
    if (result.valid == false) {
        return result;
    }
    result.valid = false;
    
    // plugin._name.gpioIRSend
    if (object.containsKey("gpioIRSend") && !object["gpioIRSend"].is<uint8_t>()) {
        result.reason = "plugins." + _name + ".gpioIRSend is not an integer";
        return result;
    }

    result.valid = true;
    return result;
}

JsonObject HAPPluginIR::getConfigImpl(){
    DynamicJsonDocument doc(128);
    doc["gpioIRSend"] = _gpioIRSend;

	return doc.as<JsonObject>();
}

void HAPPluginIR::setConfigImpl(JsonObject root){
    if (root.containsKey("gpioIRSend")){
        // LogD(" -- password: " + String(root["password"]), true);
        _gpioIRSend = root["gpioIRSend"].as<uint8_t>();
    }
}

void HAPPluginIR::setupIRSend(){
    _irsend = new IRsend(_gpioIRSend);
    _irsend->begin();
}
 