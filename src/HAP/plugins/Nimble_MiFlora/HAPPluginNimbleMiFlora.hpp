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
#include <NimBLEDevice.h>

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
	static bool containsDevice(std::string address);
	static HAPPluginNimbleMiFloraDevice* getDevice(std::string address);

	HAPAccessory*		_accessory;
    static BLEClient*   _floraClient;

    static BLEUUID      _serviceUUID;
    static BLEUUID      _uuid_version_battery;
    static BLEUUID      _uuid_sensor_data;
    static BLEUUID      _uuid_write_mode;
    
#if HAP_PLUGIN_MIFLORA2_FETCH_HISTORY    
    static BLEUUID      _serviceHistoryUUID;

    static BLEUUID      _uuid_write_history_mode;
    static BLEUUID      _uuid_history_read;
    static BLEUUID      _uuid_device_time;
#endif


    BLEClient* getFloraClient(BLEAddress floraAddress);    
	BLERemoteService* getFloraService(BLEClient* floraClient, BLEUUID uuid);
    bool forceFloraServiceDataMode(BLERemoteService* floraService, BLEUUID uuid, uint8_t* data, size_t dataLength);    
    bool readFloraDataCharacteristic(BLERemoteService* floraService, struct floraData* retData);
    bool readFloraBatteryCharacteristic(BLERemoteService* floraService, struct floraData* retData);


#if HAP_PLUGIN_MIFLORA2_FETCH_HISTORY 
    
    void entryAddress(uint8_t *address, uint16_t entry);
    bool getEntryCount(BLERemoteService* floraService, uint16_t *entryCount);

    bool readFloraDeviceTimeCharacteristic(BLERemoteService* floraService, uint32_t* deviceTime);
    bool readFloraHistoryEntryCountCharacteristic(BLERemoteService* floraService, uint16_t* entryCount);
    bool readFloraHistoryEntryCharacteristic(BLERemoteService* floraService, struct floraHistory* history);

    bool processFloraHistoryService(BLERemoteService* floraService, struct floraHistory* history, uint16_t entryCount);
#endif


    bool processFloraService(BLERemoteService* floraService, struct floraData* retData);
    bool processFloraDevice(BLEAddress floraAddress, int tryCount, struct floraData* retData);
    void processDevices(struct floraData* deviceData);


};


REGISTER_PLUGIN(HAPPluginNimbleMiFlora)
#endif /* HAPPLUGINNIMBLEMIFLORA_HPP_ */