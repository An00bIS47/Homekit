//
// HAPPluginNimbleMiFloraDevice.hpp
// Homekit
//
//  Created on: 22.09.2019
//      Author: michael
//

#ifndef HAPPLUGINNIMBLEMIFLORADEVICE_HPP_
#define HAPPLUGINNIMBLEMIFLORADEVICE_HPP_

#include <Arduino.h>
#include <MD5Builder.h>
#include <NimBLEDevice.h>

#include "HAPAccessory.hpp"
#include "HAPService.hpp"
#include "HAPCharacteristic.hpp"
#include "HAPFakeGato.hpp"
#include "HAPFakeGatoFactory.hpp"
#include "EventManager.h"
#include "HAPFakeGatoWeather.hpp"
#include "HAPVersion.hpp"
#include "HAPGlobals.hpp"




struct floraDeviceData {
	float 		temperature;
	int 		moisture;
	int 		light;
	int 		conductivity;
	int 		battery;
	char 		firmware[6];
	uint32_t	deviceTime;	
	bool 		success;
};


#if HAP_PLUGIN_MIFLORA_ENABLE_HISTORY
struct floraHistory {
	uint32_t 	timestamp;
	float 		temperature;	
	int 		moisture;
	int 		light;
	int 		conductivity;
	bool 		success;
};
#endif

class HAPPluginNimbleMiFloraDevice {

public:
    HAPPluginNimbleMiFloraDevice(BLEClient* bleClient, const std::string& address);
    HAPPluginNimbleMiFloraDevice(BLEClient* bleClient, BLEAddress address);


    HAPAccessory* initAccessory();
	bool begin();
	
	void setDeviceData(floraDeviceData data);	
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

	void changeHeartbeat(uint8_t oldValue, uint8_t newValue);

	bool processFloraDevice();
	
private:

	struct floraDeviceData 	_deviceData;

    MD5Builder 				_md5;


    inline String md5(String str) {
		_md5.begin();
		_md5.add(String(str));
		_md5.calculate();
		return _md5.toString();
	}

	HAPAccessory*		    _accessory;
		
	stringCharacteristics* 	_firmwareValue;
	stringCharacteristics* 	_lastUpdate;

	floatCharacteristics*	_humidityValue;
	floatCharacteristics*	_temperatureValue;    
	floatCharacteristics*	_lightValue;
	floatCharacteristics*	_fertilityValue;
	
	intCharacteristics* 	_batteryLevel;
	intCharacteristics* 	_batteryStatus;	

    uint8Characteristics*   _heartbeat;
	
	std::string             _deviceAddress;  


	HAPFakeGatoWeather      _fakegato;
	HAPFakeGatoFactory*     _fakegatoFactory;
	EventManager*			_eventManager;

	bool fakeGatoCallback();  
	String					_name;


	BLEClient*   			_floraClient;

    static BLEUUID      	_serviceUUID;
    static BLEUUID     	 	_uuid_version_battery;
    static BLEUUID      	_uuid_sensor_data;
    static BLEUUID      	_uuid_write_mode;
    

    BLEClient* getFloraClient(BLEAddress floraAddress);    
	BLERemoteService* getFloraService(BLEUUID uuid);
    
	bool forceFloraServiceDataMode(BLERemoteService* floraService, BLEUUID uuid, uint8_t* data, size_t dataLength); 


    bool readFloraDataCharacteristic(BLERemoteService* floraService);
    bool readFloraBatteryCharacteristic(BLERemoteService* floraService);

	uint32_t _previousMillis;
	uint32_t _interval;

#if HAP_PLUGIN_MIFLORA_ENABLE_HISTORY 

    static BLEUUID      _serviceHistoryUUID;

    static BLEUUID      _uuid_write_history_mode;
    static BLEUUID      _uuid_history_read;
    static BLEUUID      _uuid_device_time;

	bool 				_hasFetchedHistory;

    void calculateEntryAddress(uint8_t *address, const uint16_t entry);
    bool getEntryCount(BLERemoteService* floraService, uint16_t *entryCount);
    
	bool readFloraDeviceTimeCharacteristic(BLERemoteService* floraService);
    bool readFloraHistoryEntryCountCharacteristic(BLERemoteService* floraService, uint16_t* entryCount);
    bool readFloraHistoryEntryCharacteristic(BLERemoteService* floraService, struct floraHistory* history);

    bool processFloraHistoryService(BLERemoteService* floraService, struct floraHistory* history, uint16_t entryCount);        
#endif

    bool processFloraService(BLERemoteService* floraService);
    bool blinkLED();

	inline bool shouldHandle(){

		
		unsigned long currentMillis = millis(); // grab current time

		if ((unsigned long)(currentMillis - _previousMillis) >= _interval) {

			// save the last time you blinked the LED
			_previousMillis = currentMillis;

			//LogD("Handle plugin: " + String(_name), true);			
			return true;			
		}
		

		return false;
	}
    
};

#endif /* HAPPLUGINPMIFLORADEVICE_HPP_ */