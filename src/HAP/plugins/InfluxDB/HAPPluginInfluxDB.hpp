//
// HAPPluginInfluxDB.hpp
// Homekit
//
//  Created on: 29.04.2018
//      Author: michael
//

#ifndef HAPPLUGININFLUXDB_HPP_
#define HAPPLUGININFLUXDB_HPP_

#include <Arduino.h>

#include "HAPPlugins.hpp"
#include "HAPLogger.hpp"
#include "HAPAccessory.hpp"
#include "HAPGlobals.hpp"
#include <InfluxDb.h>

#define HAP_PLUGIN_INFLUXDB_INTERVAL 		(60 * 1000)		// 60 sec
#define HAP_INFLUXDB_TIMEOUT 				(10 * 1000)		// 10 sec -> unused


class HAPPluginInfluxDB: public HAPPlugin {
public:

	HAPPluginInfluxDB();

	HAPAccessory* initAccessory() override;

	bool begin();

	
    void setValue(String oldValue, String newValue){};
    String getValue(int iid) { return ""; };

    void handleImpl(bool forced = false); 
	void handleEvents(int eventCode, struct HAPEvent eventParam);

	
	HAPConfigValidationResult validateConfig(JsonObject object);
	
	JsonObject getConfigImpl();
	void setConfigImpl(JsonObject root);
	void addEventListener(EventManager* eventManager);

	
	
private:
	
	Influxdb* _influxdb;

	String _username;
	String _password;
	String _hostname;
	String _database;
	uint16_t _port;

	uint16_t _usedSize;
	
	// bool shouldHandle();
};

REGISTER_PLUGIN(HAPPluginInfluxDB)

#endif /* HAPPLUGININFLUXDB_HPP_ */ 