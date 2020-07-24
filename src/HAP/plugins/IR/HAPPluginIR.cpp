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
    _interval           = 1;
    _previousMillis     = 0;    
    
    _gpioIRSend         = HAP_IR_LED_PIN;

#if HAP_PLUGIN_IR_ENABLE_RECV     
    _gpioIRRecv         = HAP_IR_RECV_PIN;
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

    LogD(HAPServer::timeString() + " " + "IR" + "->" + String(__FUNCTION__) + " [   ] " + "Received IR Signal ...", true);

    // Check if an IR message has been received.
    if (_irrecv->decode(&_results)) {  // We have captured something.
        

        DynamicJsonDocument doc(512);

        // The capture has stopped at this point.
        decode_type_t protocol = _results.decode_type;
        uint16_t size = _results.bits;
        bool success = true;

        Serial.print(resultToHumanReadableBasic(&_results));
        Serial.println(resultToSourceCode(&_results));
        Serial.println();    // Blank line between entries
        

        // Is it a protocol we don't understand?
        if (protocol == decode_type_t::UNKNOWN) {  // Yes.
            // Convert the results into an array suitable for sendRaw().
            // resultToRawArray() allocates the memory we need for the array.
            uint16_t *raw_array = resultToRawArray(&_results);
            // Find out how many elements are in the array.
            size = getCorrectedRawLength(&_results);

            doc["protocol"] = (uint8_t) decode_type_t::UNKNOWN;
            doc["size"] = size;
            doc["hasACState"] = false;
            
            JsonArray arrayData = doc.createNestedArray("rawData");
            for (int i = 0; i < size; i++){
                arrayData.add(arrayData[i]);
            }

            // Deallocate the memory allocated by resultToRawArray().
            delete [] raw_array;
        } else if (hasACState(protocol)) {
            doc["protocol"] = (uint8_t) protocol;
            doc["size"] = size / 8;
            doc["hasACState"] = true;        
            doc["rawData"] = _results.state;        
        } else {
            // doc["protocol"] = (uint8_t) protocol;
            // doc["size"] = size;
            // doc["hasACState"] = false;
            // doc["rawData"] = _results.value;
            
        }

        // Display a crude timestamp & notification.
        uint32_t now = millis();
        Serial.printf(
            "%06u.%03u: A %d-bit %s message was %ssuccessfully retransmitted.\n",
            now / 1000, now % 1000, size, typeToString(protocol).c_str(),
            success ? "" : "un");

        serializeJson(doc, Serial);

        // Resume capturing IR messages. It was not restarted until after we sent
        // the message so we didn't capture our own message.
        _irrecv->resume();
        
        return true;
    }   

    return false;
}
#endif

void HAPPluginIR::handleImpl(bool forced){
    
#if HAP_PLUGIN_IR_ENABLE_RECV     
    LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Handle plguin [" + String(_interval) + "]", true);
    if (_isOn) {
        receiveIRSignal();   
    }    
#endif     

}

#if HAP_PLUGIN_IR_ENABLE_RECV 
void HAPPluginIR::changePower(bool oldValue, bool newValue) {
    // LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Setting iid " + String(iid) +  " oldValue: " + oldValue + " -> newValue: " + newValue, true);

    _isOn = newValue;

    if (_isOn == true) {    
        LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Enable IR Receiver", true);
        _irrecv->enableIRIn();  // Start up the IR receiver.  
    } else {
        LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Disable IR Receiver", true);
        _irrecv->disableIRIn();
    }      
}


#endif

bool HAPPluginIR::begin(){
#if HAP_PLUGIN_IR_ENABLE_RECV     
    LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Begin IR Receiver", true);
    _irrecv = new IRrecv(_gpioIRRecv, HAP_IR_RECEIVE_BUFFER_SIZE, HAP_IR_RECEIVE_TIMEOUT, true);
#endif
    return true;
}



HAPAccessory* HAPPluginIR::initAccessory(){

#if HAP_PLUGIN_IR_ENABLE_RECV    
    _accessory = new HAPAccessory();
	//HAPAccessory::addInfoServiceToAccessory(_accessory, "Builtin LED", "ACME", "LED", "123123123", &identify);
    auto callbackIdentify = std::bind(&HAPPluginIR::identify, this, std::placeholders::_1, std::placeholders::_2);
    _accessory->addInfoService("IR Receiver", "ACME", "IRR", "123123123", callbackIdentify, version());

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
#endif
    result.valid = true;
    return result;
}

JsonObject HAPPluginIR::getConfigImpl(){
    DynamicJsonDocument doc(128);
    doc["gpioIRSend"] = _gpioIRSend;
#if HAP_PLUGIN_IR_ENABLE_RECV     
    doc["gpioIRRecv"] = _gpioIRRecv;
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
#endif    
}

void HAPPluginIR::setupIRSend(){
    _irsend = new IRsend(_gpioIRSend);
    _irsend->begin();
}


 