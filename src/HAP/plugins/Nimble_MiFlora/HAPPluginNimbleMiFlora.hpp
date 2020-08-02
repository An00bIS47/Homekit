//
// HAPPluginNimbleMiFlora.hpp
// Homekit
//
//  Created on: 12.07.2019
//      Author: michael
//
// used code from https://github.com/sidddy/flora

#ifndef HAPPLUGINNIMBLEMIFLORA_HPP_
#define HAPPLUGINNIMBLEMIFLORA_HPP_

#include <Arduino.h>
#include "esp_system.h"
#include <vector>

#include "HAPPlugins.hpp"
#include "HAPLogger.hpp"
#include "HAPAccessory.hpp"
#include "HAPGlobals.hpp"


#include "HAPPluginNimbleMiFloraDevice.hpp"




class HAPPluginNimbleMiFlora: public HAPPlugin {
public:

	HAPPluginNimbleMiFlora();
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
	
    int _deviceCount;



private:

    static std::vector<BLEAddress> _supportedDevices;

	static std::vector<HAPPluginNimbleMiFloraDevice*>	_devices;
	static bool containsDevice(const std::string address);
	static HAPPluginNimbleMiFloraDevice* getDevice(std::string address);

	HAPAccessory*		_accessory;
    static BLEClient*   _floraClient;

#if HAP_PLUGIN_MIFLORA_ENABLE_HISTORY 
	uint32_t			_intervalScan;
	uint32_t			_previousMillisScan;
    bool shouldScan();
#endif

    void processDevices();


};


REGISTER_PLUGIN(HAPPluginNimbleMiFlora)
#endif /* HAPPLUGINNIMBLEMIFLORA_HPP_ */