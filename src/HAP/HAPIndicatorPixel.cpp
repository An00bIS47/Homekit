//
// HAPIndicatorPixel.cpp
// Homekit
//
//  Created on: 09.10.2020
//      Author: michael
//

#include "HAPIndicatorPixel.hpp"
#include "HAPIndicatorPixelColors.hpp"
#include "HAPLogger.hpp"
#include "HAPServer.hpp"

HAPIndicatorPixel::HAPIndicatorPixel(){
    _previousMillis = 0;  
    // _timesBlinked = 0;  
    _timesBlinkedRemaining = 0;
    _currentColor = HAPColorBlack;
    _currentState = false;
}

void HAPIndicatorPixel::begin(){
    // _pixelIndicator = new Adafruit_NeoPixel(1, HAP_PIXEL_INDICATOR_PIN, (NEO_GRB + NEO_KHZ800));
    // _pixelIndicator->begin();
    // _pixelIndicator->show();            // Turn OFF all pixels ASAP
    // _pixelIndicator->setBrightness(HAP_PIXEL_INIDICATOR_BRIGHTNESS);

    // _pixelIndicator = new NeoPixelBus<NeoGrbFeature, NeoEsp32BitBang800KbpsMethod>(1, HAP_PIXEL_INDICATOR_PIN);
    // // this resets all the neopixels to an off state
    // _pixelIndicator->Begin();
    // _pixelIndicator->Show();

    FastLED.addLeds<NEOPIXEL, HAP_PIXEL_INDICATOR_PIN>(_pixelIndicator, 1);  // GRB ordering is assumed
    _pixelIndicator[0] = CRGB::Black;
    FastLED.show();   
}

// void HAPIndicatorPixel::setColor(uint32_t color){
//     Serial.println(">>>>>>>>>>>>>> <<<<<  SET PIXEL ON");    
//     // _pixelIndicator->setPixelColor(1, _pixelIndicator->gamma32(color));
//     // _pixelIndicator->show();

//     _pixelIndicator->SetPixelColor(0, color);
//     _pixelIndicator->Show();

//     // _pixelIndicator[0] = color;
//     // FastLED.show(); 
//     _currentState = true;
    
// }

// void HAPIndicatorPixel::setColor(RgbColor color){
//     Serial.println(">>>>>>>>>>>>>> <<<<<  SET PIXEL ON");    
//     _pixelIndicator->SetPixelColor(0, color);
//     _pixelIndicator->Show();
//     _currentState = true;    
// }

void HAPIndicatorPixel::setColor(CRGB color){
    
    _pixelIndicator[0] = color;
    FastLED.show();
    _currentState = true;    
}

void HAPIndicatorPixel::off(){
     
    // _pixelIndicator->clear(); // Set all pixel colors to 'off'
    // _pixelIndicator->show();

    // _pixelIndicator->SetPixelColor(0, black);
    // _pixelIndicator->Show();

    _pixelIndicator[0] = CRGB::Black;
    FastLED.show(); 

    _currentState = false;
}

// void HAPIndicatorPixel::confirmWithColor(uint32_t color){
//     blinkWithColor(color, 3);
// }

// void HAPIndicatorPixel::confirmWithColor(RgbColor color){
//     blinkWithColor(color, 3);
// }

void HAPIndicatorPixel::confirmWithColor(CRGB color){
    blinkWithColor(color, 3);
}

void HAPIndicatorPixel::handle(){
    uint32_t currentMillis = millis();
    if (currentMillis - _previousMillis >= HAP_PIXEL_INDICATOR_BLINK_DELAY) {
        // save the last time you blinked the LED
        _previousMillis = currentMillis;

        
        // blinking
        if (_timesBlinkedRemaining > 0){
            if (_currentState == false) {
                
                // _pixelIndicator->setPixelColor(1, _pixelIndicator->gamma32(_currentColor));
                // _pixelIndicator->show();
                
                // _pixelIndicator->SetPixelColor(0, blue);
                // _pixelIndicator->Show();
                
                _pixelIndicator[0] = _currentColor;
                FastLED.show(); 

                _currentState = true;                                  
            } else {
                         
                // _pixelIndicator->clear(); // Set all pixel colors to 'off'
                // // _pixelIndicator->setPixelColor(1, _pixelIndicator->gamma32(HAPColorBlack));
                // _pixelIndicator->show();
                
                // _pixelIndicator->SetPixelColor(0, black);
                // _pixelIndicator->Show();

                _pixelIndicator[0] = CRGB::Black;
                FastLED.show(); 


                _currentState = false;
            }       

            _timesBlinkedRemaining--;                   
        }        
    }
}

// void HAPIndicatorPixel::blinkWithColor(uint32_t color, uint8_t times){
//     _currentColor = color;
//     // _timesBlinked = 0;
//     _timesBlinkedRemaining = times * 2;
//     _previousMillis = 0;
// }

// void HAPIndicatorPixel::blinkWithColor(RgbColor color, uint8_t times){
//     _currentColor = color;
//     // _timesBlinked = 0;
//     _timesBlinkedRemaining = times * 2;
//     _previousMillis = 0;
// }

void HAPIndicatorPixel::blinkWithColor(CRGB color, uint8_t times){
    _currentColor = color;
    // _timesBlinked = 0;
    _timesBlinkedRemaining = times * 2;
    _previousMillis = 0;
}