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
#include "HAPFakeGatoSwitch.hpp"


class HAPPluginRCSwitchDevice {
public:

    HAPPluginRCSwitchDevice();
    HAPPluginRCSwitchDevice(uint8_t houseAddress_, uint8_t deviceAddress_, String name_);
                            
    HAPAccessory* initAccessory();    

    void setState(String pwrState);    

    void identify(bool oldValue, bool newValue);
    void setEventManager(EventManager* eventManager);
    void setFakeGatoFactory(HAPFakeGatoFactory* fakegatoFactory);

    void setRCSwitchSendCallback(std::function<void(uint8_t, uint8_t, bool)> callback){
        _callbackRCSwitchSend = callback;
    }

    
    uint8_t             houseAddress;
    uint8_t             deviceAddress;
    String              name;

private:    

    std::function<void(uint8_t, uint8_t, uint8_t)> _callbackRCSwitchSend = NULL;  

    HAPFakeGatoSwitch       _fakegato;
    
    HAPAccessory*           _accessory;
    EventManager*			_eventManager;
    HAPFakeGatoFactory*     _fakegatoFactory;

    boolCharacteristics*    _stateValue;

    void changedState(bool oldValue, bool newValue);
   
    bool fakeGatoCallback();  

    MD5Builder 				_md5;

    inline String md5(String str) {
		_md5.begin();
		_md5.add(String(str));
		_md5.calculate();
		return _md5.toString();
	}
};

#endif /* HAPPLUGINRCSWITCHDEVICE_HPP_ */

