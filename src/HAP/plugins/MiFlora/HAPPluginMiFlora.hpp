//
// HAPPluginMiFlora.hpp
// Homekit
//
//  Created on: 12.07.2019
//      Author: michael
//
// used code from https://github.com/sidddy/flora

#ifndef HAPPLUGINMIFLORA_HPP_
#define HAPPLUGINMIFLORA_HPP_




#include <Arduino.h>
#include <BLEDevice.h>
#include "esp_system.h"
#include <vector>

#include "HAPPlugins.hpp"
#include "HAPLogger.hpp"
#include "HAPAccessory.hpp"
#include "HAPGlobals.hpp"

#if HAP_PLUGIN_USE_MIFLORA

#include "HAPPluginMiFloraDevice.hpp"
#include "HAPPluginMiFloraScanner.hpp"


#define HAP_MIFLORA_INTERVAL    				30000	// every 300 seconds
#define HAP_MIFLORA_RETRY                   	3
#define HAP_MIFLORA_DISABLE_BT_MODE_CLASSIC		1		// disable BLE classic mode
#define HAP_MIFLORA_USE_BLE_SCANNER				1
#define HAP_MIFLORA_SCANNER_TIMEOUT				10		// timeout in seconds

#define HAP_MIFLORA_ENABLE_WDT					0
#define HAP_MIFLORA_WDT_TIMEOUT					90000 	//time in ms to trigger the watchdog

/*
 * Macro to check the outputs of TWDT functions and trigger an abort if an
 * incorrect code is returned.
 */
#define CHECK_ERROR_CODE(returned, expected) ({                        \
            if(returned != expected){                                  \
                printf("TWDT ERROR\n");                                \
                abort();                                               \
            }                                                          \
})


class HAPPluginMiFlora: public HAPPlugin {
public:

	static int 	scanTimeout;
	static bool isConnected;
	static bool isInitialized;

	HAPPluginMiFlora();
	HAPAccessory* initAccessory();
	bool begin();
	
	void setValue(int iid, String oldValue, String newValue);		
	String getValue(int iid);
	
	

	void handleImpl(bool forced=false);
	void identify(bool oldValue, bool newValue);

	HAPConfigValidationResult validateConfig(JsonObject object);
	JsonObject getConfigImpl();
	void setConfigImpl(JsonObject root);
	// void handleEvents(int eventCode, struct HAPEvent eventParam);
	
	static inline void stopClient(){
		if (_floraClient) {
			if (_floraClient->isConnected()){
				_floraClient->disconnect();
			}
		}
	}
	
	static void Task1code( void * pvParameters );
	static TaskHandle_t 	_task;
	static hw_timer_t* _timer;

#if HAP_MIFLORA_USE_BLE_SCANNER
	static HAPPluginMiFloraDevicesScanner floraScanner;
#else
    int _deviceCount;
#endif

private:		

	static std::vector<HAPPluginMiFloraDevice*>	_devices;
	static bool containsDevice(std::string address);
	static HAPPluginMiFloraDevice* getDevice(std::string address);


	HAPAccessory*		_accessory;
		

    static BLEUUID _serviceUUID;
    static BLEUUID _uuid_version_battery;
    static BLEUUID _uuid_sensor_data;
    static BLEUUID _uuid_write_mode;

	static BLEClient* _floraClient;

    BLEClient* getFloraClient(BLEAddress floraAddress);
    
	BLERemoteService* getFloraService();

    bool forceFloraServiceDataMode(BLERemoteService* floraService);
    bool readFloraDataCharacteristic(BLERemoteService* floraService, struct floraData* retData);
    bool readFloraBatteryCharacteristic(BLERemoteService* floraService, struct floraData* retData);
    bool processFloraService(BLERemoteService* floraService, bool readBattery, struct floraData* retData);
    bool processFloraDevice(BLEAddress floraAddress, bool getBattery, int tryCount, struct floraData* retData);

	bool processFloraDevice2(BLEAddress floraAddress, bool getBattery, int tryCount, struct floraData* retData);
	
	unsigned long 			_previousMillisWDT;

	int _wdtTimeout;  	//time in ms to trigger the watchdog		
};


REGISTER_PLUGIN(HAPPluginMiFlora)

#endif


#endif