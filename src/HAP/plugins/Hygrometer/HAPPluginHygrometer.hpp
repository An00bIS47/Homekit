//
// HAPPluginHygrometer.hpp
// Homekit
//
//  Created on: 22.04.2018
//      Author: michael
//

#ifndef HAPPLUGINHYGROMETER_HPP_
#define HAPPLUGINHYGROMETER_HPP_

#include <Arduino.h>
#include "HAPPlugins.hpp"
#include "HAPLogger.hpp"
#include "HAPAccessory.hpp"

#include "HAPFakeGato.hpp"
#include "HAPFakeGatoWeather.hpp"

// Pinout:
// The YL-39 module has 4 pins: 
// - VCC: 3.3-5V
// - GND
// - D0 : digital pin that goes LOW or HIGH depending on a preset value
// - A0 : analog output that can be easily read by Arduino
// 
// On ESP32s when using WiFi only ADC1 pins can be used! ADC2 is disabled when using WIFI
// 
// The issue with such sensors is that the probe itself work by trying to measure the current 
// that goes from one side of it to the other. Because of this electrolysis occurs so it can destroy 
// the probe (YL-69) pretty fast in high-moisture soils. To bypass this, 
// instead of directly linking the VCC to the Arduino's VCC/5V we simply link it to a digital pin 
// and power it (digital pin goes HIGH) only before we do a readout (see the code for this).
// 
// 
// Huzzah32 Pinout:
// 14 -> VCC
// A0 -> GPIO 32
// 
// 

#ifndef HAP_PLUGIN_HYGROMETER_USE_DUMMY
#define HAP_PLUGIN_HYGROMETER_USE_DUMMY 	0
#endif


#define HAP_PLUGIN_HYGROMETER_PIN_VCC       14
#define HAP_PLUGIN_HYGROMETER_PIN_ADC       32

#define HAP_PLUGIN_HYGROMTER_REFERENCE      2500    // value if put in a glass of water

class HAPPluginHygrometer: public HAPPlugin {
public:

	HAPPluginHygrometer();

	bool begin();
	HAPAccessory* initAccessory() override;
	
	void setValue(int iid, String oldValue, String newValue);	
	

	void changeHum(float oldValue, float newValue);




	void identify(bool oldValue, bool newValue);
    void handleImpl(bool forced = false);	
	
	HAPConfigValidationResult validateConfig(JsonObject object);
	JsonObject getConfigImpl();
	void setConfigImpl(JsonObject root);
	// void handleEvents(int eventCode, struct HAPEvent eventParam);
private:

	floatCharacteristics*	_humidityValue;

	bool fakeGatoCallback();

	HAPFakeGatoWeather _fakegato;

    uint16_t readSensor();

};

REGISTER_PLUGIN(HAPPluginHygrometer)

#endif /* HAPPLUGINHYGROMETER_HPP_ */ 