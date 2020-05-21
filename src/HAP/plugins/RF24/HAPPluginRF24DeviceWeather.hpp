//
// HAPPluginRF24DeviceWeather.hpp
// Homekit
//
//  Created on: 20.05.2020
//      Author: michael
//

#ifndef HAPPLUGINF24DEVICEWEATHER_HPP_
#define HAPPLUGINF24DEVICEWEATHER_HPP_


#include <Arduino.h>
#include "HAPAccessory.hpp"
#include "HAPService.hpp"
#include "HAPCharacteristic.hpp"
#include "EventManager.h"
#include "HAPFakeGato.hpp"
#include "HAPFakeGatoFactory.hpp"
#include "HAPFakeGatoWeather.hpp"
#include "HAPPluginRF24Device.hpp"

class HAPPluginRF24DeviceWeather : public HAPPluginRF24Device{
public:

    HAPPluginRF24DeviceWeather();
    HAPPluginRF24DeviceWeather(uint8_t address_, String name_);
                            
    HAPAccessory* initAccessory() override;    

	void setValue(int iid, String oldValue, String newValue);
	

	void changeTemp(float oldValue, float newValue);
	void changeHum(float oldValue, float newValue);
	void changePressure(uint16_t oldValue, uint16_t newValue);


    void identify(bool oldValue, bool newValue);
    void setEventManager(EventManager* eventManager);
    void setFakeGatoFactory(HAPFakeGatoFactory* fakegatoFactory);



private:    
    
    HAPAccessory*           _accessory;
    EventManager*			_eventManager;
    HAPFakeGatoFactory*     _fakegatoFactory;
   
	floatCharacteristics*	_humidityValue;
	floatCharacteristics*	_temperatureValue;
	uint16Characteristics*	_pressureValue;
    
    HAPFakeGatoWeather _fakegato;
   
    bool fakeGatoCallback() override;  
};

#endif /* HAPPLUGINF24DEVICEWEATHER_HPP_ */

