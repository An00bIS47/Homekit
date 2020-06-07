//
// HAPPluginRF24DeviceDHT.hpp
// Homekit
//
//  Created on: 31.05.2020
//      Author: michael
//

#ifndef HAPPLUGINF24DEVICEDHT_HPP_
#define HAPPLUGINF24DEVICEDHT_HPP_


#include <Arduino.h>
#include "HAPAccessory.hpp"
#include "HAPService.hpp"
#include "HAPCharacteristic.hpp"
#include "EventManager.h"
#include "HAPFakeGato.hpp"
#include "HAPFakeGatoFactory.hpp"
#include "HAPFakeGatoWeather.hpp"
#include "HAPPluginRF24Device.hpp"

class HAPPluginRF24DeviceDHT : public HAPPluginRF24Device{
public:

    HAPPluginRF24DeviceDHT();
    HAPPluginRF24DeviceDHT(uint16_t id_, String name_, uint8_t measureMode_);
    // ~HAPPluginRF24DeviceDHT();                            

    HAPAccessory* initAccessory() override;    
	
	void changeTemp(float oldValue, float newValue);
	void changeHum(float oldValue, float newValue);
	
    
    void changeMeasureMode(uint8_t oldValue, uint8_t newValue);
    
    void changeLastUpdate(String oldValue, String newValue);
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
	
    stringCharacteristics* 	_lastUpdate;

    intCharacteristics* 	_batteryLevel;
	intCharacteristics* 	_batteryStatus;	


    uint8Characteristics*   _measureMode;
    
    HAPFakeGatoWeather      _fakegato;
   
    bool fakeGatoCallback() override;  

    std::function<void(NewSettingsPacket)> _callbackSendSettings = NULL;  
};

#endif /* HAPPLUGINF24DEVICEWEATHER_HPP_ */

