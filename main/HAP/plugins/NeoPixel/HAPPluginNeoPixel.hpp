//
// HAPPluginNeoPixel.hpp
// Homekit
//
//  Created on: 11.09.2019
//      Author: michael
//

#ifndef HAPPLUGINNEOPIXEL_HPP_
#define HAPPLUGINNEOPIXEL_HPP_

#include <Arduino.h>
#include "HAPPlugins.hpp"
#include "HAPLogger.hpp"
#include "HAPAccessory.hpp"

#include <Adafruit_NeoPixel.h>

#define HAP_PLUGIN_NEOPIXEL_FORMAT NEO_GRB + NEO_KHZ800

// How many leds in your strip?
#define NUM_LEDS 1
#define DATA_PIN 16	

#define HAP_PLUGIN_NEOPIXEL_ENABLE_BRIGHTNESS   1

struct rgb_t {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t w;
}; 

class HAPPluginNeoPixel: public HAPPlugin {
public:

	HAPPluginNeoPixel();
	HAPAccessory* initAccessory() override;
	
	bool begin();

	void setValue(int iid, String oldValue, String newValue);

	String getValue(int iid);

	void identify(bool oldValue, bool newValue);

	void changePower(bool oldValue, bool newValue);
	void changeHue(float oldValue, float newValue);
	void changeSaturation(float oldValue, float newValue);
	
#if HAP_PLUGIN_NEOPIXEL_ENABLE_BRIGHTNESS	
	void changeBrightness( int oldValue, int newValue);
#endif

	void handleImpl(bool forced=false);
	
	// void handleEvents(int eventCode, struct HAPEvent eventParam);
	HAPConfigValidationResult validateConfig(JsonObject object);
	JsonObject getConfigImpl();
	void setConfigImpl(JsonObject root);


	static void hsi2rgb(float H, float S, float I, rgb_t* rgbw);
private:	
	//HAPAccessory*			_accessory;
	// HAPService*				_service;	
	boolCharacteristics* 	_powerState;

	floatCharacteristics*	_hue;
	floatCharacteristics*	_saturation;


#if HAP_PLUGIN_NEOPIXEL_ENABLE_BRIGHTNESS		
	intCharacteristics*	 	_brightnessState;
#endif

	//intCharacteristics*	 	_brightnessState;

	// unsigned long 		_interval;
	// unsigned long 		_previousMillis;

	// EventManager*	_eventManager;
	// MemberFunctionCallable<HAPPlugin> listenerMemberFunctionPlugin;
		
	Adafruit_NeoPixel *_pixels;

	uint8_t _gpio;
	bool _isOn;
};

REGISTER_PLUGIN(HAPPluginNeoPixel)

#endif /* HAPPLUGINNEOPIXEL_HPP_ */ 