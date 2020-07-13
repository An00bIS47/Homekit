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
#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS 1
#define DATA_PIN 21

#define BRIGHTNESS 255

struct rgb_t {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t w;
}; 

class HAPPluginNeoPixel: public HAPPlugin {
public:

	static CRGB neopixels[NUM_LEDS];

	HAPPluginNeoPixel();
	HAPAccessory* initAccessory() override;
	
	static bool setupNeopixels();

	bool begin();

	void setValue(int iid, String oldValue, String newValue);

	String getValue(int iid);

	void identify(bool oldValue, bool newValue);

	void changePower(bool oldValue, bool newValue);
	void changeHue(float oldValue, float newValue);
	void changeSaturation(float oldValue, float newValue);
	

	void changeBrightness( int oldValue, int newValue);


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
	intCharacteristics*	 	_brightnessState;

		
	

	uint8_t _gpio;
	bool _isOn;
};

REGISTER_PLUGIN(HAPPluginNeoPixel)

#endif /* HAPPLUGINNEOPIXEL_HPP_ */ 