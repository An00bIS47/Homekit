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
#include "HAPVersion.hpp"

class HAPPluginRF24DeviceWeather : public HAPPluginRF24Device{
public:

    HAPPluginRF24DeviceWeather();
    HAPPluginRF24DeviceWeather(uint16_t id_, String name_, uint8_t measureMode_);
    // ~HAPPluginRF24DeviceWeather();                            

    HAPAccessory* initAccessory() override;    
	
    void changeLastUpdate(String oldValue, String newValue);
    void changeFirmware(String oldValue, String newValue);
    
    void changeMeasureMode(uint8_t oldValue, uint8_t newValue);
    void changeHeartbeat(uint8_t oldValue, uint8_t newValue);

	void changeTemp(float oldValue, float newValue);
	void changeHum(float oldValue, float newValue);
	void changePressure(uint16_t oldValue, uint16_t newValue);
    
    void changeBatteryLevel( float oldValue, float newValue);
	void changeBatteryStatus(float oldValue, float newValue);

    // void identify(bool oldValue, bool newValue);
    void setEventManager(EventManager* eventManager);
    void setFakeGatoFactory(HAPFakeGatoFactory* fakegatoFactory);

    void setValuesFromPayload(struct RadioPacket payload) override;
    void setSettingsFromPayload(struct RemoteDeviceSettings settings) override;

    void setSendSettingsCallback(std::function<void(NewSettingsPacket)> callback) override {
        _callbackSendSettings = callback;
    }


private:    
    
    HAPAccessory*           _accessory;
    EventManager*			_eventManager;
    HAPFakeGatoFactory*     _fakegatoFactory;
   
	floatCharacteristics*	_humidityValue;
	floatCharacteristics*	_temperatureValue;
	uint16Characteristics*	_pressureValue;

    stringCharacteristics* 	_lastUpdate;

    intCharacteristics* 	_batteryLevel;
	intCharacteristics* 	_batteryStatus;	
    
    uint8Characteristics*   _measureMode;
    uint8Characteristics*   _heartbeat;
    stringCharacteristics*  _firmware;

    HAPFakeGatoWeather      _fakegato;
   
    bool fakeGatoCallback() override;  

    std::function<void(NewSettingsPacket)> _callbackSendSettings = NULL;  
};

#endif /* HAPPLUGINF24DEVICEWEATHER_HPP_ */

