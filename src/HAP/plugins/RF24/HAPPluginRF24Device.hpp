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

struct __attribute__((__packed__)) RemoteDeviceSettings
{
    uint16_t    radioId;
    uint8_t     sleepInterval;
    uint8_t     measureMode;
    uint8_t     version;
    char        firmware_version[6];
};


struct __attribute__((__packed__)) RadioPacket {
    uint16_t    radioId;
    uint8_t     type;
    
    int32_t     temperature;    // temperature
    uint32_t    humidity;       // humidity
    uint32_t    pressure;       // pressure
    
    uint16_t    voltage;        // voltage
};

enum MeasureMode
{
	MeasureModeWeatherStation   = 0x00,
	MeasureModeIndoor           = 0x01,
};


enum ChangeType
{
    ChangeTypeNone              = 0x00,
    ChangeRadioId               = 0x01,
    ChangeSleepInterval         = 0x02,
    ChangeMeasureType           = 0x03,
};


struct __attribute__((__packed__)) NewSettingsPacket
{
    uint8_t         changeType;     // enum ChangeType
    uint16_t        forRadioId;
    uint16_t        newRadioId;
    uint8_t         newSleepInterval;
    uint8_t         newMeasureMode;
};


class HAPPluginRF24Device {
public:
    HAPPluginRF24Device();
    HAPPluginRF24Device(uint16_t id_, String name_, uint8_t measureMode_);
    ~HAPPluginRF24Device();
    
                            
    virtual HAPAccessory* initAccessory() = 0;    

    void identify(bool oldValue, bool newValue);
    void setEventManager(EventManager* eventManager);
    void setFakeGatoFactory(HAPFakeGatoFactory* fakegatoFactory);
    
    virtual void setValuesFromPayload(struct RadioPacket payload) = 0;
    virtual void setSettingsFromPayload(struct RemoteDeviceSettings settings) = 0;


    virtual void setSendSettingsCallback(std::function<void(NewSettingsPacket)> callback){
        _callbackSendSettings = callback;
    }


    uint16_t            id;
    String              name;
    uint8_t             type;

    enum MeasureMode    measureMode;
    uint32_t            sleepInterval;    

private:    



    std::function<void(NewSettingsPacket)> _callbackSendSettings = NULL;  

    HAPAccessory*           _accessory;
    EventManager*			_eventManager;
    HAPFakeGatoFactory*     _fakegatoFactory;
   
    virtual bool fakeGatoCallback() = 0;  
};

#endif /* HAPPLUGINF24DEVICE_HPP_ */

