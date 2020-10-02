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
#define VERSION_MINOR       5	// 2 = FakeGato support
#define VERSION_REVISION    1
#define VERSION_BUILD       1



IRsend* HAPPluginIR::_irsend;


uint8_t HAPPluginIR::_gpioIRSend = HAP_PLUGIN_IR_SEND_PIN;

HAPPluginIR::HAPPluginIR(){
    _type               = HAP_PLUGIN_TYPE_OTHER;
    _name               = "IR";
    _isEnabled          = HAP_PLUGIN_USE_IR;
    _interval           = 250;
    _previousMillis     = 0;    
    
    _gpioIRSend         = HAP_PLUGIN_IR_SEND_PIN;

#if HAP_PLUGIN_IR_ENABLE_RECV     
    _gpioIRRecv         = HAP_PLUGIN_IR_RECV_PIN;
    _isOn               = false;
    _powerState         = nullptr;
    _irrecv             = nullptr;
#endif


    _version.major      = VERSION_MAJOR;
    _version.minor      = VERSION_MINOR;
    _version.revision   = VERSION_REVISION;
    _version.build      = VERSION_BUILD;

}


// ToDo: Get it to working!
#if HAP_PLUGIN_IR_ENABLE_RECV 
bool HAPPluginIR::receiveIRSignal(){    

    // Check if an IR message has been received.
    if (_irrecv->decode(&_results)) {  // We have captured something.
        
        LogD(HAPServer::timeString() + " " + "IR" + "->" + String(__FUNCTION__) + " [   ] " + "Received IR Signal ...", true);
#if HAP_DEBUG_IR        
        Serial.print(resultToHumanReadableBasic(&_results));
        Serial.println(resultToSourceCode(&_results));
        Serial.println();    // Blank line between entries
#endif

        if (indexOfDecodeResult(&_results) == -1){
            HAPPluginIRDevice* newDevice = new HAPPluginIRDevice(_results);
            
            newDevice->setEventManager(_eventManager);            
            
            _accessorySet->addAccessory(newDevice->initAccessory());

            _devices.push_back(newDevice);
        }

        // Resume capturing IR messages. It was not restarted until after we sent
        // the message so we didn't capture our own message.
        _irrecv->resume();
        
        _powerState->setValue("0");
        
        return true;
    }   

    return false;
}
#endif

void HAPPluginIR::handleImpl(bool forced){
    
#if HAP_PLUGIN_IR_ENABLE_RECV     
    
    if (_isOn || forced) {
        LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Handle plguin [" + String(_interval) + "]", true);
        receiveIRSignal();   
    }    
#endif     

}

#if HAP_PLUGIN_IR_ENABLE_RECV 
void HAPPluginIR::changePower(bool oldValue, bool newValue) {
    LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Enable IR Recv - " +  " oldValue: " + oldValue + " -> newValue: " + newValue, true);

    if (_isOn != newValue) {
        if (!_isOn) {    
            LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Enable IR Receiver", true);
            _irrecv->enableIRIn();  // Start up the IR receiver.  
        } else {
            LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Disable IR Receiver", true);
            _irrecv->disableIRIn(); // Stop the IR receiver.  
        }  
        _isOn = newValue;
    }
    
}


#endif

bool HAPPluginIR::begin(){
#if HAP_PLUGIN_IR_ENABLE_RECV     
    LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Begin IR Receiver", true);
    pinMode(_gpioIRRecv, INPUT);
    _irrecv = new IRrecv(_gpioIRRecv, HAP_PLUGIN_IR_RECEIVE_BUFFER_SIZE, HAP_PLUGIN_IR_RECEIVE_TIMEOUT, false, HAP_PLUGIN_IR_RECEIVE_TIMER);
    _isOn = false;
#endif
    return true;
}



HAPAccessory* HAPPluginIR::initAccessory(){

#if HAP_PLUGIN_IR_ENABLE_RECV    
    _accessory = new HAPAccessory();
	//HAPAccessory::addInfoServiceToAccessory(_accessory, "Builtin LED", "ACME", "LED", "123123123", &identify);
    auto callbackIdentify = std::bind(&HAPPluginIR::identify, this, std::placeholders::_1, std::placeholders::_2);
    _accessory->addInfoService("IR Receiver", "ACME", "IRRecv", "123123123", callbackIdentify, version());

    HAPService* _service = new HAPService(HAP_SERVICE_SWITCH);
    _accessory->addService(_service);

    stringCharacteristics *switchServiceName = new stringCharacteristics(HAP_CHARACTERISTIC_NAME, permission_read, 32);
    switchServiceName->setValue("IR Receiver");
    _accessory->addCharacteristics(_service, switchServiceName);

    _powerState = new boolCharacteristics(HAP_CHARACTERISTIC_ON, permission_read|permission_write|permission_notify);
    if (_isOn)
        _powerState->setValue("1");
    else
        _powerState->setValue("0");
    
    auto callbackPowerState = std::bind(&HAPPluginIR::changePower, this, std::placeholders::_1, std::placeholders::_2);        
    _powerState->valueChangeFunctionCall = callbackPowerState;
    _accessory->addCharacteristics(_service, _powerState);
    
    return _accessory;
#else
    return nullptr;
#endif
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

#if HAP_PLUGIN_IR_ENABLE_RECV 
    // plugin._name.gpioIRRecv
    if (object.containsKey("gpioIRRecv") && !object["gpioIRRecv"].is<uint8_t>()) {
        result.reason = "plugins." + _name + ".gpioIRRecv is not an integer";
        return result;
    }

    // ToDo: Validate devices array
#endif
    result.valid = true;
    return result;
}

JsonObject HAPPluginIR::getConfigImpl(){

    LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Get config implementation", true);

    DynamicJsonDocument doc(512);
    doc["gpioIRSend"] = _gpioIRSend;
#if HAP_PLUGIN_IR_ENABLE_RECV     
    doc["gpioIRRecv"] = _gpioIRRecv;


    JsonArray devices = doc.createNestedArray("devices");
    for (int i=0; i < _devices.size(); i++){   
        JsonObject device = devices.createNestedObject();     
        _devices[i]->toJson(device);
    }

#endif

#if HAP_DEBUG_CONFIG
    serializeJson(doc, Serial);
    Serial.println();
#endif

	return doc.as<JsonObject>();
}

void HAPPluginIR::setConfigImpl(JsonObject root){
    if (root.containsKey("gpioIRSend")){
        _gpioIRSend = root["gpioIRSend"].as<uint8_t>();
    }
#if HAP_PLUGIN_IR_ENABLE_RECV 
    if (root.containsKey("gpioIRRecv")){        
        _gpioIRRecv = root["gpioIRRecv"].as<uint8_t>();
    }

    // using C++11 syntax (preferred):
    for (JsonVariant value : root["devices"].as<JsonArray>()) {
        
        HAPPluginIRDevice* newDevice = new HAPPluginIRDevice(value.as<JsonObject>());
            
        newDevice->setEventManager(_eventManager);                        
        _accessorySet->addAccessory(newDevice->initAccessory());

        _devices.push_back(newDevice);
        
    }


#endif    
}

void HAPPluginIR::setupIRSend(){

    _irsend = new IRsend(_gpioIRSend);
    _irsend->begin();

}


 int HAPPluginIR::indexOfDevice(HAPPluginIRDevice* device){
    // Check if element 22 exists in vector
	std::vector<HAPPluginIRDevice*>::iterator it = std::find(_devices.begin(), _devices.end(), device);
 
	if (it != _devices.end()) {		
		// Get index of element from iterator
		return std::distance(_devices.begin(), it);		
	} else {
        return -1;
    }
}

int HAPPluginIR::indexOfDecodeResult(decode_results* result){
    
    for (uint8_t i=0; i < _devices.size(); i++){  

#if HAP_DEBUG_IR        
        Serial.print(i);
        Serial.println();    // Blank line between entries           
        Serial.print(resultToHumanReadableBasic(&_results));
        Serial.println(resultToSourceCode(&_results));
        Serial.println();    // Blank line between entries    
#endif               
        if (memcmp(_devices[i]->getDecodeResults(), result, sizeof(decode_results)) == 0){
            return i;
#if HAP_DEBUG_IR      
            Serial.println("IDENTICAL");
#endif 
        }
#if HAP_DEBUG_IR              
        Serial.println("=================================================");    // Blank line between entries           
#endif         
    }
#if HAP_DEBUG_IR       
    Serial.println("NO MATCH FOUND");
#endif     
    return -1;
}