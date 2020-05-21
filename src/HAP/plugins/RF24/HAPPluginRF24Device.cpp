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
    address = 0;

    _accessory          = nullptr;
    _eventManager       = nullptr;  
    _fakegatoFactory    = nullptr;
}

HAPPluginRF24Device::HAPPluginRF24Device(uint8_t address_, String name_)
: address(address_)
, name(name_)
{
    _accessory          = nullptr;
    _eventManager       = nullptr;      
    _fakegatoFactory    = nullptr;
}

void HAPPluginRF24Device::setEventManager(EventManager* eventManager){
    _eventManager = eventManager;
}


void HAPPluginRF24Device::setFakeGatoFactory(HAPFakeGatoFactory* fakegatoFactory){
    _fakegatoFactory = fakegatoFactory;
}   


void HAPPluginRF24Device::identify(bool oldValue, bool newValue) {
    printf("Start Identify rf24: %d\n", address);
}