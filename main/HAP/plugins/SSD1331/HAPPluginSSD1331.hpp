//
// HAPPluginSSD1331.hpp
// Homekit
//
//  Created on: 29.04.2018
//      Author: michael
//

#ifndef HAPPLUGINSSD1331_HPP_
#define HAPPLUGINSSD1331_HPP_

#include <Arduino.h>
#include "HAPPlugins.hpp"
#include "HAPLogger.hpp"
#include "HAPAccessory.hpp"

#include "SSD_13XX.h"

#include <map>

#define __CS 		2
#define __DC		16
#define __RST		17
#define __MOSI 		23
#define __SCLK 		18

#define HAP_PLUGIN_SSD_INTERVAL 2500

struct screenInfo {
	uint8_t aid;
	uint8_t iid;
	String name;
	String accessoryName;
};

class HAPPluginSSD1331: public HAPPlugin {
public:
	HAPPluginSSD1331();
	HAPAccessory* initAccessory();
	bool begin();

	void handleImpl(bool forced=false);
	void handleEvents(int eventCode, struct HAPEvent eventParam);

	void addEventListener(EventManager* eventManager);

	HAPConfigValidationResult validateConfig(JsonObject object);
	JsonObject getConfigImpl();
	void setConfigImpl(JsonObject root);

private:
    SSD_13XX* _tft;	
	bool _updateDisplay;
	uint8_t _currentScreen;
	uint8_t _numberOfScreens;

	std::map<int, struct screenInfo> _screenMap;
	
	void displayQRCode();
	void setupScreens();

	
};

REGISTER_PLUGIN(HAPPluginSSD1331)

#endif