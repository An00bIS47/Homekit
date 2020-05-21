//
// HAPPluginRF24Device.hpp
// Homekit
//
//  Created on: 20.05.2020
//      Author: michael
//

#ifndef HAPPLUGINF24DEVICE_HPP_
#define HAPPLUGINF24DEVICE_HPP_


#include <Arduino.h>
#include "HAPAccessory.hpp"
#include "HAPService.hpp"
#include "HAPCharacteristic.hpp"
#include "HAPFakeGato.hpp"
#include "HAPFakeGatoFactory.hpp"
#include "EventManager.h"
#include "HAPFakeGatoSwitch.hpp"


class HAPPluginRF24Device {
public:
    HAPPluginRF24Device();
    HAPPluginRF24Device(uint8_t address_, String name_);
                            
    virtual HAPAccessory* initAccessory() = 0;    

    void identify(bool oldValue, bool newValue);
    void setEventManager(EventManager* eventManager);
    void setFakeGatoFactory(HAPFakeGatoFactory* fakegatoFactory);

    uint8_t             address;
    String              name;
    uint8_t             type;

private:    
    
    HAPAccessory*           _accessory;
    EventManager*			_eventManager;
    HAPFakeGatoFactory*     _fakegatoFactory;
   
    virtual bool fakeGatoCallback() = 0;  
};

#endif /* HAPPLUGINF24DEVICE_HPP_ */

