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


#define REMOTE_DEVICE_VMIN 1900
#define REMOTE_DEVICE_VMAX 3450


enum RemoteDeviceType {
    RemoteDeviceTypeWeather    = 0x01,
    RemoteDeviceTypeDHT	       = 0x02,
};


struct __attribute__((__packed__)) RadioPacket {
    uint8_t     radioId;
    uint8_t     type;
    
    uint32_t    temperature;    // temperature
    uint32_t    humidity;       // humidity
    uint16_t    pressure;       // pressure
    
    uint16_t    voltage;        // voltage
};

enum MeasureMode
{
	MeasureModeWeatherStation   = 0x01,
	MeasureModeIndoor           = 0x11,
};


enum ChangeType
{
    ChangeRadioId               = 0x00,
    ChangeSleepInterval         = 0x01,
    ChangeMeasureType           = 0x02,
};


struct __attribute__((__packed__)) NewSettingsPacket
{
    uint8_t         changeType;     // enum ChangeType
    uint8_t         forRadioId;
    uint8_t         newRadioId;
    uint32_t        newSleepIntervalSeconds;
    uint8_t         newMeasureMode;
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
    
    virtual void setValuesFromPayload(struct RadioPacket payload) = 0;

    uint16_t            id;
    String              name;
    uint8_t             type;

    virtual void setSendSettingsCallback(std::function<void(NewSettingsPacket)> callback){
        _callbackSendSettings = callback;
    }

private:    
    
    std::function<void(NewSettingsPacket)> _callbackSendSettings = NULL;  


    HAPAccessory*           _accessory;
    EventManager*			_eventManager;
    HAPFakeGatoFactory*     _fakegatoFactory;
   
    virtual bool fakeGatoCallback() = 0;  
};

#endif /* HAPPLUGINF24DEVICE_HPP_ */

