//
// HAPPluginHoneywell.hpp
// Homekit
//
//  Created on: 22.04.2018
//      Author: michael
//

#ifndef HAPPLUGINHONEYWELL_HPP_
#define HAPPLUGINHONEYWELL_HPP_

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

#include "HAPPlugins.hpp"
#include "HAPLogger.hpp"
#include "HAPAccessory.hpp"
// #include "HAPFakeGatoSwitch.hpp"

class HAPPluginHoneywell: public HAPPlugin {
public:

	HAPPluginHoneywell();
	HAPAccessory* initAccessory() override;
	
	bool begin();

	void setValue(int iid, String oldValue, String newValue);

	String getValue(int iid);

	void changeActive(uint8_t oldValue, uint8_t newValue);
	void changeFanState(uint8_t oldValue, uint8_t newValue);
	void changeSwingMode(uint8_t oldValue, uint8_t newValue);


	void handleImpl(bool forced=false);
	void identify( bool oldValue, bool newValue);
	
	// void handleEvents(int eventCode, struct HAPEvent eventParam);
	HAPConfigValidationResult validateConfig(JsonObject object);
	JsonObject getConfigImpl();
	void setConfigImpl(JsonObject root);	

private:	
	uint8Characteristics* 	_activeState;
	uint8Characteristics* 	_currentFanState;
	uint8Characteristics*	_swingModeState;

	// bool fakeGatoCallback(); 
	// HAPFakeGatoSwitch       _fakegato;

	IRsend* _irsend;

	uint8_t _gpio;
	
	// ToDo: Required ??
	bool _isOn;
	uint8_t _fanState;
	bool _swingMode;
};

REGISTER_PLUGIN(HAPPluginHoneywell)

#endif /* HAPPLUGINHONEYWELL_HPP_ */ 