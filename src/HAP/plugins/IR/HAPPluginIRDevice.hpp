//
// HAPPluginIRDevice.hpp
// Homekit
//
//  Created on: 20.12.2019
//      Author: michael
//

#ifndef HAPPLUGINIRDEVICE_HPP_
#define HAPPLUGINIRDEVICE_HPP_


#include <Arduino.h>
#include <ArduinoJson.h>
#include "HAPAccessory.hpp"
#include "HAPService.hpp"
#include "HAPCharacteristic.hpp"
#include "HAPFakeGatoFactory.hpp"
#include "EventManager.h"
#include "HAPFakeGatoSwitch.hpp"

#include <IRrecv.h>

#define HAP_PLUGIN_IR_DEVICE_FREQUENCY 38000

class HAPPluginIRDevice {
public:

    HAPPluginIRDevice(const decode_results capture);
    HAPPluginIRDevice(JsonObject device);
                            
    HAPAccessory* initAccessory();    

    void setState(String pwrState);    

    void identify(bool oldValue, bool newValue);
    void setEventManager(EventManager* eventManager);
    // void setFakeGatoFactory(HAPFakeGatoFactory* fakegatoFactory);

    decode_results* getDecodeResults(){
        return &_capture;
    }

    void toJson(JsonObject root);

private:    
    
    HAPAccessory*           _accessory;
    EventManager*			_eventManager;
    // HAPFakeGatoFactory*     _fakegatoFactory;

    boolCharacteristics*    _stateValue;

    decode_results          _capture;

    void changedState(bool oldValue, bool newValue);
};

#endif /* HAPPLUGINIRDEVICE_HPP_ */

