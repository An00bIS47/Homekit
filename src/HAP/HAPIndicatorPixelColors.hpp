//
// HAPIndicatorPixelColors.hpp
// Homekit
//
//  Created on: 20.08.2017
//      Author: michael
//

#ifndef HAPINDICATORPIXELCOLORS_HPP_
#define HAPINDICATORPIXELCOLORS_HPP_

#include <Arduino.h>
// #include "NeoPixelBus.h"
#include "FastLED.h"

// struct RGB {
// 	uint8_t r;
// 	uint8_t g;
// 	uint8_t b;

// 	RGB(uint8_t r_, uint8_t g_, uint8_t b_){
// 		r = r_;
// 		g = g_;
// 		b = b_;
// 	}

// 	RGB(){
// 		r = 0;
// 		g = 0;
// 		b = 0;
// 	}
// };

// #define HAPColorBlack 	0x000000 // RGB(0,0,0)
// #define HAPColorWhite 	0xFFFFFF // RGB(255, 255, 255)
// #define HAPColorGreen 	0x00FF00 // RGB(0,255,0)
// #define HAPColorRed 	0xFF0000 // RGB(255,0,0)
// #define HAPColorBlue 	0x00F00F // RGB(0,0,255)
// #define HAPColorMagenta 0xFF00FF // RGB(255,0,255)
// #define HAPColorCyan 	0x00FFFF // RGB(0,255,255)
// #define HAPColorYellow 	0xFFFF00 // RGB(255,255,0)
// #define HAPColorOrange 	0xFFA500 // RGB(255,165,0)
// #define HAPColorPurple 	0x800080 // RGB(128,0,128)

// #define HAPColorBlack 	RgbColor(0,0,0) // RGB(0,0,0)
// #define HAPColorWhite 	RgbColor(255, 255, 255) // RGB(255, 255, 255)
// #define HAPColorGreen 	RgbColor(0,255,0) // RGB(0,255,0)
// #define HAPColorRed 	RgbColor(255,0,0) // RGB(255,0,0)
// #define HAPColorBlue 	RgbColor(0,0,255) // RGB(0,0,255)
// #define HAPColorMagenta RgbColor(255,0,255) // RGB(255,0,255)
// #define HAPColorCyan 	RgbColor(0,255,255) // RGB(0,255,255)
// #define HAPColorYellow 	RgbColor(255,255,0) // RGB(255,255,0)
// #define HAPColorOrange 	RgbColor(255,165,0) // RGB(255,165,0)
// #define HAPColorPurple 	RgbColor(128,0,128) // RGB(128,0,128)

#define HAPColorBlack       CRGB::Black // RGB(0,0,0)
#define HAPColorWhite 	    CRGB::White // RGB(255, 255, 255)
#define HAPColorGreen 	    CRGB::Green // RGB(0,255,0)
#define HAPColorRed 	    CRGB::Red // RGB(255,0,0)
#define HAPColorBlue 	    CRGB::Blue // RGB(0,0,255)
#define HAPColorMagenta     CRGB::Magenta // RGB(255,0,255)
#define HAPColorCyan 	    CRGB::Cyan // RGB(0,255,255)
#define HAPColorYellow 	    CRGB::Yellow // RGB(255,255,0)
#define HAPColorOrange 	    CRGB::Orange // RGB(255,165,0)
#define HAPColorPurple 	    CRGB::Purple // RGB(128,0,128)

#endif /* HAPVERSION_HPP_ */
