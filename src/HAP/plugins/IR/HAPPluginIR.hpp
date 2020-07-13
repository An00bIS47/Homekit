//
// HAPPluginIR.hpp
// Homekit
//
//  Created on: 13.07.2020
//      Author: michael
//

#ifndef HAPPLUGINIR_HPP_
#define HAPPLUGINIR_HPP_

#include <Arduino.h>
#include "HAPPlugins.hpp"
#include "HAPLogger.hpp"
#include "HAPAccessory.hpp"
#include <IRremoteESP8266.h>
#include <IRsend.h>

#ifndef HAP_IR_LED_PIN
#define HAP_IR_LED_PIN      14
#endif

#define DELAY_BETWEEN_BUTTON_PRESS 150  // in ms


class HAPPluginIR: public HAPPlugin {
public:

	HAPPluginIR();
	HAPAccessory* initAccessory() override;
	
	bool begin();


	void changePower(bool oldValue, bool newValue);
	void changeBrightness(int oldValue, int newValue);

	void handleImpl(bool forced=false);
	
	
	// void handleEvents(int eventCode, struct HAPEvent eventParam);
	HAPConfigValidationResult validateConfig(JsonObject object);
	JsonObject getConfigImpl();
	void setConfigImpl(JsonObject root);

	static void setupIRSend();

	static IRsend* getIRSend(){
		return _irsend;
	}

private:	
	static uint8_t _gpioIRSend;
	static IRsend* _irsend;

};

REGISTER_PLUGIN(HAPPluginIR)

#endif /* HAPPLUGINIR_HPP_ */ 