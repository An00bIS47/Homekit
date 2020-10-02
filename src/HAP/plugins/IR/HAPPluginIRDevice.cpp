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
#include "HAPPluginIR.hpp"

#include <IRsend.h>
#include <IRutils.h>
#include <IRac.h>
#include <IRtext.h>

HAPPluginIRDevice::HAPPluginIRDevice(const decode_results capture){   

    _accessory          = nullptr;
    _eventManager       = nullptr;  

    _stateValue         = nullptr;

    _capture            = capture;
}

HAPPluginIRDevice::HAPPluginIRDevice(JsonObject root){
    _accessory          = nullptr;
    _eventManager       = nullptr;  

    _stateValue         = nullptr;

    
//   decode_type_t decode_type;  // NEC, SONY, RC5, UNKNOWN
//   // value, address, & command are all mutually exclusive with state.
//   // i.e. They MUST NOT be used at the same time as state, so we can use a union
//   // structure to save us a handful of valuable bytes of memory.
//   union {
//     struct {
//       uint64_t value;    // Decoded value
//       uint32_t address;  // Decoded device address.
//       uint32_t command;  // Decoded command.
//     };
//     uint8_t state[kStateSizeMax];  // Multi-byte results.
//   };
//   uint16_t bits;              // Number of bits in decoded value
//   volatile uint16_t *rawbuf;  // Raw intervals in .5 us ticks
//   uint16_t rawlen;            // Number of records in rawbuf.
//   bool overflow;
//   bool repeat;  // Is the result a repeat code?    
    _capture.decode_type = (decode_type_t)root["protocol"].as<uint8_t>();        
    _capture.bits = root["size"].as<uint16_t>();

    if (_capture.decode_type == decode_type_t::UNKNOWN) {
        
        _capture.rawlen = root["size"].as<uint16_t>();
        
        _capture.rawbuf = (uint16_t*) malloc(sizeof(uint16_t) * _capture.rawlen);        
        for (int i=0; i < _capture.rawlen; i++){
            _capture.rawbuf[i] = root["rawData"][i].as<uint16_t>() / RAWTICK;
        }

    } else if (hasACState(_capture.decode_type)) {
        // ToDo:
        // doc["size"] = size / 8;      
        // doc["rawData"] = _capture.state;        
    } else {                
        _capture.value  = root["rawData"].as<uint64_t>();            
    }

    _capture.bits       = root["bits"].as<uint16_t>();
    _capture.overflow   = root["overflow"].as<bool>();
    _capture.repeat     = root["repeat"].as<bool>();

}

HAPAccessory* HAPPluginIRDevice::initAccessory(){

    String name = "IR ";
    name += typeToString(_capture.decode_type, _capture.repeat);
    name += " ";
    name += resultToHexidecimal(&_capture);

    String sn = HAPDeviceID::serialNumber("IR", resultToHexidecimal(&_capture));    

    // Create accessory if not already created
    _accessory = new HAPAccessory();
    //HAPAccessory::addInfoServiceToAccessory(_accessory, "Builtin LED", "ACME", "LED", "123123123", &identify);
    auto callbackIdentify = std::bind(&HAPPluginIRDevice::identify, this, std::placeholders::_1, std::placeholders::_2);
    _accessory->addInfoService(name, "ACME", "IR Device", sn, callbackIdentify, "1.0");

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


void HAPPluginIRDevice::toJson(JsonObject root){
    
    // The capture has stopped at this point.
    decode_type_t protocol = _capture.decode_type;
    uint16_t size = _capture.bits;        

    LogD("Received IR Signal: ", true);
    LogD("   protocol: " + typeToString(protocol), true);
    //LogD("   size:     " + String(size), true);
    LogD("   code:     " + resultToHexidecimal(&_capture), true);
    LogD("   bits:     " + uint64ToString(_capture.bits), true);

    // Is it a protocol we don't understand?
    if (protocol == decode_type_t::UNKNOWN) {  // Yes.
        // Convert the results into an array suitable for sendRaw().
        // resultToRawArray() allocates the memory we need for the array.
        // uint16_t *raw_array = resultToRawArray(&_capture);
        // Find out how many elements are in the array.
        // size = getCorrectedRawLength(&_capture);

        root["protocol"] = (uint8_t) decode_type_t::UNKNOWN;
        root["size"] = _capture.rawlen;
        
        JsonArray arrayData = root.createNestedArray("rawData");
        for (int i = 0; i < _capture.rawlen; i++){
            uint32_t value = _capture.rawbuf[i] * RAWTICK;
            arrayData.add(value);
        }

        // Deallocate the memory allocated by resultToRawArray().
        // delete [] raw_array;
    } else if (hasACState(protocol)) {
        root["protocol"] = (uint8_t) protocol;
        root["size"] = size / 8;  
        //  ToDo:
        root["rawData"] = _capture.state;        
    } else {
        root["protocol"] = (uint8_t) protocol;
        root["size"] = size;
        root["rawData"] = _capture.value;            
    }

    root["bits"] = _capture.bits;
    root["overflow"] = _capture.overflow;
    root["repeat"] = _capture.repeat;

    // serializeJson(doc, Serial);
}