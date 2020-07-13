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

#define USE_CURRNT_FAN_STATE	1

class HAPPluginHoneywell: public HAPPlugin {
public:

	HAPPluginHoneywell();
	HAPAccessory* initAccessory() override;
	
	bool begin();

	// void setValue(int iid, String oldValue, String newValue);
	// String getValue(int iid);

	void changeActive(uint8_t oldValue, uint8_t newValue);
	void changeSwingMode(uint8_t oldValue, uint8_t newValue);
	void changeSpeed(uint8_t oldValue, uint8_t newValue);

#if USE_CURRNT_FAN_STATE	
	void changeFanState(uint8_t oldValue, uint8_t newValue);
#endif	

	void handleImpl(bool forced=false);
	void identify( bool oldValue, bool newValue);
	
	// void handleEvents(int eventCode, struct HAPEvent eventParam);
	HAPConfigValidationResult validateConfig(JsonObject object);
	JsonObject getConfigImpl();
	void setConfigImpl(JsonObject root);	

private:	
	bool _isOn;
	uint8Characteristics* 	_activeState;

	bool _swingMode;
	uint8Characteristics*	_swingModeState;

	uint8_t _speed;
	uint8Characteristics* 	_speedState;

#if USE_CURRNT_FAN_STATE	
	uint8_t _fanState;
	uint8Characteristics* 	_currentFanState;
#endif	
	

	// bool fakeGatoCallback(); 
	// HAPFakeGatoSwitch       _fakegato;

	IRsend* _irsend;

	uint8_t _gpio;
	
};

REGISTER_PLUGIN(HAPPluginHoneywell)

#endif /* HAPPLUGINHONEYWELL_HPP_ */ 