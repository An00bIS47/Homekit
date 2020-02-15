//
// HAPPluginMiFloraDevice.hpp
// Homekit
//
//  Created on: 22.09.2019
//      Author: michael
//

#ifndef HAPPLUGINPMIFLORADEVICE_HPP_
#define HAPPLUGINPMIFLORADEVICE_HPP_

#include <Arduino.h>
#include <MD5Builder.h>
#include <BLEAddress.h>

#include "HAPAccessory.hpp"
#include "HAPService.hpp"
#include "HAPCharacteristic.hpp"
#include "HAPFakeGato.hpp"
#include "HAPFakeGatoFactory.hpp"
#include "EventManager.h"
#include "HAPFakeGatoWeather.hpp"
#include "HAPVersion.hpp"
#include "HAPGlobals.hpp"

#define HAP_PLUGIN_MIFLORA2_FETCH_HISTORY       0


typedef struct floraData {
	float 	temperature;
	int 	moisture;
	int 	light;
	int 	conductivity;
	int 	battery;
	char 	firmware[6];
	bool 	success;
} floraData;


#if HAP_PLUGIN_MIFLORA2_FETCH_HISTORY
typedef struct floraHistory {
	uint32_t 	timestamp;
	float 		temperature;	
	int 		moisture;
	int 		light;
	int 		conductivity;
	bool 		success;
} floraHistory;
#endif

class HAPPluginMiFloraDevice {

public:
    HAPPluginMiFloraDevice(std::string address);
    HAPPluginMiFloraDevice(BLEAddress address);


    HAPAccessory* initAccessory();
	bool begin();
	
	void setValue(floraData data);	
	void setEventManager(EventManager* eventManager);
    void setFakeGatoFactory(HAPFakeGatoFactory* fakegatoFactory);	

    std::string address();

	void identify(bool oldValue, bool newValue);

	void changeTemp(float oldValue, float newValue);
	void changeHum(float oldValue, float newValue);
	void changeLight(float oldValue, float newValue);

	void changeLastUpdate(String oldValue, String newValue);

	void changeBatteryLevel( float oldValue, float newValue);
	void changeBatteryStatus(float oldValue, float newValue);

	void changeFertility( float oldValue, float newValue);
	
	inline String version(){
		return _version.toString();
	}
private:

    MD5Builder 				_md5;

    inline String md5(String str) {
		_md5.begin();
		_md5.add(String(str));
		_md5.calculate();
		return _md5.toString();
	}

	HAPVersion 				_version;

	HAPAccessory*		    _accessory;
		
	stringCharacteristics* 	_firmwareValue;
	stringCharacteristics* 	_lastUpdate;

	floatCharacteristics*	_humidityValue;
	floatCharacteristics*	_temperatureValue;    
	floatCharacteristics*	_lightValue;
	floatCharacteristics*	_fertilityValue;
	
	intCharacteristics* 	_batteryLevel;
	intCharacteristics* 	_batteryStatus;	

    std::string             _deviceAddress;  


	HAPFakeGatoWeather      _fakegato;
	HAPFakeGatoFactory*     _fakegatoFactory;
	EventManager*			_eventManager;

	bool fakeGatoCallback();  
    
};

#endif /* HAPPLUGINPMIFLORADEVICE_HPP_ */