//
// HAPPluginIRDevice.cpp
// Homekit
//
//  Created on: 14.09.2019
//      Author: michael
//

#include "HAPPluginIRDevice.hpp"
#include "HAPServer.hpp"
#include "HAPLogger.hpp"
#include <IRsend.h>


HAPPluginIRDevice::HAPPluginIRDevice(const decode_results capture){   

    _accessory          = nullptr;
    _eventManager       = nullptr;  

    _stateValue         = nullptr;

    _capture            = capture;
}

HAPAccessory* HAPPluginIRDevice::initAccessory(){

    String name = "IR ";
    name += typeToString(_capture.decode_type, _capture.repeat);
    name += " ";
    name += resultToHexidecimal(&_capture);

    // Create accessory if not already created
    _accessory = new HAPAccessory();
    //HAPAccessory::addInfoServiceToAccessory(_accessory, "Builtin LED", "ACME", "LED", "123123123", &identify);
    auto callbackIdentify = std::bind(&HAPPluginIRDevice::identify, this, std::placeholders::_1, std::placeholders::_2);
    _accessory->addInfoService(name, "ACME", "IR Device", resultToHexidecimal(&_capture), callbackIdentify, "1.0");

    // 
    // Outlet Service / Switch Service
    // !!! Fakegato history is only working with switch service !!!
    // 
    HAPService* outletService = new HAPService(HAP_SERVICE_SWITCH);
    _accessory->addService(outletService);

    stringCharacteristics *outletServiceName = new stringCharacteristics(HAP_CHARACTERISTIC_NAME, permission_read, HAP_STRING_LENGTH_MAX);
    outletServiceName->setValue(name);
    _accessory->addCharacteristics(outletService, outletServiceName);

    //
    // Power State 
    // 
    _stateValue = new boolCharacteristics(HAP_CHARACTERISTIC_ON, permission_read|permission_write|permission_notify);            
    _stateValue->setValue("0");

    auto callbackState = std::bind(&HAPPluginIRDevice::changedState, this, std::placeholders::_1, std::placeholders::_2);        
    _stateValue->valueChangeFunctionCall = callbackState;
    _accessory->addCharacteristics(outletService, _stateValue);

    return _accessory;
}

void HAPPluginIRDevice::setEventManager(EventManager* eventManager){
    _eventManager = eventManager;
}


// void HAPPluginIRDevice::setFakeGatoFactory(HAPFakeGatoFactory* fakegatoFactory){
//     _fakegatoFactory = fakegatoFactory;
// }   


void HAPPluginIRDevice::identify(bool oldValue, bool newValue) {
    printf("Start Identify ir device\n");
}


void HAPPluginIRDevice::setState(String pwrState){
    if (_stateValue) 
        _stateValue->setValue(pwrState);
}


void HAPPluginIRDevice::changedState(bool oldValue, bool newValue){

    LogD(HAPServer::timeString() + " " + "HAPPluginIRDevice" + "->" + String(__FUNCTION__) + " [   ] " + "Setting new value to " + String(newValue), true);

    if (oldValue != newValue) {        

        // The capture has stopped at this point.
        decode_type_t protocol = _capture.decode_type;
        uint16_t size = _capture.bits;
        bool success = true;
        // Is it a protocol we don't understand?
        if (protocol == decode_type_t::UNKNOWN) {  // Yes.
            // Convert the results into an array suitable for sendRaw().
            // resultToRawArray() allocates the memory we need for the array.
            uint16_t *raw_array = resultToRawArray(&_capture);
            // Find out how many elements are in the array.
            size = getCorrectedRawLength(&_capture);

            // Send it out via the IR LED circuit.
            HAPPluginIR::getIRSend()->sendRaw(raw_array, size, HAP_PLUGIN_IR_DEVICE_FREQUENCY); 

            // Deallocate the memory allocated by resultToRawArray().
            delete [] raw_array;
        } else if (hasACState(protocol)) {  // Does the message require a state[]?
            // It does, so send with bytes instead.
            success = HAPPluginIR::getIRSend()->send(protocol, _capture.state, size / 8);
        } else {  // Anything else must be a simple message protocol. ie. <= 64 bits
            success = HAPPluginIR::getIRSend()->send(protocol, _capture.value, size);
        }


        if (success){
            struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _stateValue->iid, newValue ? "1" : "0");							
            _eventManager->queueEvent( EventManager::kEventNotifyController, event);     
        } else {
            LogW("Failed to send command!", true);
        }   
    }    
}
