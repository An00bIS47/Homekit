//
// HAPPluginRF24Device.cpp
// Homekit
//
//  Created on: 20.05.2020
//      Author: michael
//

#include "HAPPluginRF24Device.hpp"
#include "HAPServer.hpp"
#include "HAPLogger.hpp"

HAPPluginRF24Device::HAPPluginRF24Device(){   
    name    = "";    
    id = 0;
    type = 0;
    
    _accessory          = nullptr;
    _eventManager       = nullptr;  
    _fakegatoFactory    = nullptr;

    sleepInterval       = 1;
    measureMode         = (enum MeasureMode)0;

}

HAPPluginRF24Device::HAPPluginRF24Device(uint16_t id_, String name_, uint8_t measureMode_)
: id(id_)
, name(name_)
{
    type = 0;
    _accessory          = nullptr;
    _eventManager       = nullptr;      
    _fakegatoFactory    = nullptr;

    sleepInterval       = 1;
    measureMode         = (enum MeasureMode)measureMode_;
}

HAPPluginRF24Device::~HAPPluginRF24Device(){
}

void HAPPluginRF24Device::setEventManager(EventManager* eventManager){
      
    _eventManager = eventManager;
    // Serial.printf("event: %p\n", _eventManager);  
}


void HAPPluginRF24Device::setFakeGatoFactory(HAPFakeGatoFactory* fakegatoFactory){
    
    _fakegatoFactory = fakegatoFactory;
    // Serial.printf("fakegato: %p\n", _fakegatoFactory);
}   


void HAPPluginRF24Device::identify(bool oldValue, bool newValue) {
    printf("Start Identify rf24: %d\n", id);
}

