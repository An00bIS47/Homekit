//
// HAPIndicatorPixel.hpp
// Homekit
//
//  Created on: 09.10.2020
//      Author: michael
//

#ifndef HAPINDICATORPIXEL_HPP_
#define HAPINDICATORPIXEL_HPP_

#include <Arduino.h>
#include "HAPGlobals.hpp"

#include "FastLED.h"
// #include "NeoPixelBus.h"
// #include <Adafruit_NeoPixel.h>


#define HAP_PIXEL_INDICATOR_BLINK_CONFIRM 3
#define HAP_PIXEL_INDICATOR_BLINK_DELAY   250

class HAPIndicatorPixel {
public:
	HAPIndicatorPixel();

	void begin();
	void handle();
	// void setColor(uint32_t color);
	// void setColor(RgbColor color);
	void setColor(CRGB color);
	void off();
	// void confirmWithColor(uint32_t color);
	// void confirmWithColor(RgbColor color);
	void confirmWithColor(CRGB color);
	// void blinkWithColor(uint32_t color, uint8_t times);
	// void blinkWithColor(RgbColor color, uint8_t times);
	void blinkWithColor(CRGB color, uint8_t times);

protected:	
	uint32_t _previousMillis;
	uint8_t _timesBlinkedRemaining;
	// uint32_t _currentColor;
	// RgbColor _currentColor;
	CRGB _currentColor;
	// Adafruit_NeoPixel *_pixelIndicator;
	// NeoPixelBus<NeoGrbFeature, NeoEsp32BitBang800KbpsMethod>* _pixelIndicator; //(1, HAP_PIXEL_INDICATOR_PIN);
	CRGB _pixelIndicator[1];
	bool _currentState;
private:

};

#endif