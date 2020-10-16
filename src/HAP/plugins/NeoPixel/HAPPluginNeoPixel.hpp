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

// #include <Adafruit_NeoPixel.h>
#include <FastLED.h>

// #define HAP_PLUGIN_NEOPIXEL_FORMAT NEO_GRB + NEO_KHZ800

// How many leds in your strip?
#define NUM_LEDS 1
#define NEOPIXEL_DATA_PIN A5	


class HAPPluginNeoPixel: public HAPPlugin {
public:

	HAPPluginNeoPixel();
	HAPAccessory* initAccessory() override;
	
	bool begin();

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
	
private:		
	boolCharacteristics* 	_powerState;

	floatCharacteristics*	_hue;
	floatCharacteristics*	_saturation;
	intCharacteristics*	 	_brightnessState;

	void setPixelColor(uint16_t hueDegree, uint8_t satPercent, uint8_t briPercent);

	CRGB _pixels[NUM_LEDS];

	uint8_t _gpio;
	bool 	_isOn;
};

REGISTER_PLUGIN(HAPPluginNeoPixel)

#endif /* HAPPLUGINNEOPIXEL_HPP_ */ 