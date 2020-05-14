//
// HAPPluginSSD1306.hpp
// Homekit
//
//  Created on: 29.04.2018
//      Author: michael
//

#ifndef HAPPLUGINSSD1306_HPP_
#define HAPPLUGINSSD1306_HPP_

#include <Arduino.h>
#include "HAPPlugins.hpp"
#include "HAPLogger.hpp"
#include "HAPAccessory.hpp"

#if HAP_PLUGIN_USE_SSD1306
#include <Wire.h>
#include "SSD1306Wire.h"

#include <map>

#define __CS 		A12			//  2
#define __DC		RST_OLED	// 16
#define __RST		A17			// 17
#define __MOSI 		MOSI		// 23
#define __SCLK 		SCK			// 18

#define HAP_PLUGIN_SSD_INTERVAL 2500

struct screenInfo {
	uint8_t aid;
	uint8_t iid;
	String name;
	String accessoryName;
};

class HAPPluginSSD1306: public HAPPlugin {
public:
	HAPPluginSSD1306();
	~HAPPluginSSD1306();
	HAPAccessory* initAccessory();
	bool begin();

	void handleImpl(bool forced=false);
	void handleEvents(int eventCode, struct HAPEvent eventParam);

	void addEventListener(EventManager* eventManager);

	HAPConfigValidationResult validateConfig(JsonObject object);
	JsonObject getConfigImpl();
	void setConfigImpl(JsonObject root);

private:
	SSD1306Wire *_tft;
	bool _updateDisplay;
	uint8_t _currentScreen;
	uint8_t _numberOfScreens;

	std::map<int, struct screenInfo> _screenMap;
	
	void displayQRCode();
	void setupScreens();

	void updateProgressbar(uint8_t percentage);
	
};

REGISTER_PLUGIN(HAPPluginSSD1306)

#endif

#endif