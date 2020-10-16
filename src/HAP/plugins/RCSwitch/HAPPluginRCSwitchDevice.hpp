//
// HAPPluginRCSwitchDevice.hpp
// Homekit
//
//  Created on: 20.12.2019
//      Author: michael
//

#ifndef HAPPLUGINRCSWITCHDEVICE_HPP_
#define HAPPLUGINRCSWITCHDEVICE_HPP_


#include <Arduino.h>
#include <MD5Builder.h>
#include "HAPAccessory.hpp"
#include "HAPService.hpp"
#include "HAPCharacteristic.hpp"
#include "HAPFakeGato.hpp"
#include "HAPFakeGatoFactory.hpp"
#include "EventManager.h"
#include "HAPFakeGatoEnergy.hpp"


class HAPPluginRCSwitchDevice {
public:

    HAPPluginRCSwitchDevice();
    HAPPluginRCSwitchDevice(uint8_t houseAddress_, uint8_t deviceAddress_, String name_);
                            
    HAPAccessory* initAccessory();    

    void setState(String pwrState);    

    void identify(bool oldValue, bool newValue);
    void setEventManager(EventManager* eventManager);
    void setFakeGatoFactory(HAPFakeGatoFactory* fakegatoFactory);

    // void switchOn();
    // void switchOff();
    void switchCallback(uint16_t state);

    uint32_t getTimestampLastActivity();

    void setRCSwitchSendCallback(std::function<void(uint8_t, uint8_t, bool)> callback){
        _callbackRCSwitchSend = callback;
    }

    JsonObject scheduleToJson();
    void scheduleFromJson(JsonObject &root);

    void saveConfig();

    uint8_t             houseAddress;
    uint8_t             deviceAddress;
    String              name;

private:    

    std::function<void(uint8_t, uint8_t, uint8_t)> _callbackRCSwitchSend = NULL;  

    HAPFakeGatoEnergy       _fakegato;
    
    HAPAccessory*           _accessory;
    EventManager*			_eventManager;
    HAPFakeGatoFactory*     _fakegatoFactory;

    boolCharacteristics*    _stateValue;
    boolCharacteristics*    _inUseState;
    boolCharacteristics*    _parentalLock;
    floatCharacteristics*   _curPowerValue;
    floatCharacteristics*   _ttlPowerValue;

    uint32_t                _timestampLastActivity;

    void changedPowerState(bool oldValue, bool newValue);
	void changedPowerCurrent(float oldValue, float newValue);
	void changedPowerTotal(float oldValue, float newValue);
    void changedState(bool oldValue, bool newValue);
   
    bool fakeGatoCallback();  
};

#endif /* HAPPLUGINRCSWITCHDEVICE_HPP_ */

