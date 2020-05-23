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


enum HAP_RF24_REMOTE_TYPE {
    HAP_RF24_REMOTE_TYPE_NONE       = 0x00,
    HAP_RF24_REMOTE_TYPE_WEATHER    = 0x01
};


struct __attribute__((__packed__)) HAP_RF24_PAYLOAD {
    uint16_t    id;
    uint8_t     type;

	uint32_t    temp;
	uint32_t    hum;
	uint16_t    pres;
    uint8_t     voltage;
};

class HAPPluginRF24Device {
public:
    HAPPluginRF24Device();
    HAPPluginRF24Device(uint16_t id_, String name_);
    ~HAPPluginRF24Device();
    
                            
    virtual HAPAccessory* initAccessory() = 0;    

    void identify(bool oldValue, bool newValue);
    void setEventManager(EventManager* eventManager);
    void setFakeGatoFactory(HAPFakeGatoFactory* fakegatoFactory);
    
    virtual void setValuesFromPayload(struct HAP_RF24_PAYLOAD payload) = 0;

    uint16_t            id;
    String              name;
    uint8_t             type;

private:    
    
    HAPAccessory*           _accessory;
    EventManager*			_eventManager;
    HAPFakeGatoFactory*     _fakegatoFactory;

    
   
    virtual bool fakeGatoCallback() = 0;  
};

#endif /* HAPPLUGINF24DEVICE_HPP_ */

