//
// HAPPluginPCA301Device.hpp
// Homekit
//
//  Created on: 14.09.2019
//      Author: michael
//

#ifndef HAPPLUGINPCA301DEVICE_HPP_
#define HAPPLUGINPCA301DEVICE_HPP_


#include <Arduino.h>
#include <MD5Builder.h>
#include "HAPAccessory.hpp"
#include "HAPService.hpp"
#include "HAPCharacteristic.hpp"
#include "HAPFakeGato.hpp"
#include "HAPFakeGatoFactory.hpp"
#include "EventManager.h"
#include "HAPFakeGatoEnergy.hpp"


class HAPPluginPCA301Device {
public:

    HAPPluginPCA301Device();
    HAPPluginPCA301Device(uint8_t channel_, uint32_t devId_, bool pState_, String name_);
                            
    HAPAccessory* initAccessory();    

    void setPowerState(String pwrState);    
    void setCurrentPower(String pwrCur);
    void setTotalPower(String pwrTtl);

    void identify(bool oldValue, bool newValue);
    void setEventManager(EventManager* eventManager);
    void setFakeGatoFactory(HAPFakeGatoFactory* fakegatoFactory);

    void setPCA301SendCallback(std::function<void(uint32_t, char)> callback){
        _callbackPCA301Send = callback;
    }

    uint8_t             channel;                    // associated device channel
    uint32_t            devId;                      // device ID
    bool                pState;                     // device powered on/off
    uint16_t            pNow;                       // actual power consumption (W)
    uint16_t            pTtl;                       // total power consumption (KWh)
    uint32_t            nextTX;                     // last packet submitted  
    uint16_t            retries;                    // outstanding answers
    String              name;

private:    

    std::function<void(uint32_t, char)> _callbackPCA301Send = NULL;  

    HAPFakeGatoEnergy      _fakegato;
    
    HAPAccessory*           _accessory;
    EventManager*			_eventManager;
    HAPFakeGatoFactory*     _fakegatoFactory;

    boolCharacteristics*    _powerState;
    boolCharacteristics*    _inUseState;
    floatCharacteristics*   _curPowerValue;
    floatCharacteristics*   _ttlPowerValue;

    void changedPowerState(bool oldValue, bool newValue);
	void changedPowerCurrent(float oldValue, float newValue);
	void changedPowerTotal(float oldValue, float newValue);
   
    bool fakeGatoCallback();  

    MD5Builder 				_md5;

    inline String md5(String str) {
		_md5.begin();
		_md5.add(String(str));
		_md5.calculate();
		return _md5.toString();
	}
};

#endif /* HAPPLUGINPCA301DEVICE_HPP_ */

