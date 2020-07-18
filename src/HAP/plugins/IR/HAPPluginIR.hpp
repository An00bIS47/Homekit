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

#define HAP_PLUGIN_IR_ENABLE_RECV 1

#if HAP_PLUGIN_IR_ENABLE_RECV 
#include <IRrecv.h>
#include <IRutils.h>
#include <IRac.h>
#include <IRtext.h>
#ifndef HAP_IR_RECV_PIN
#define HAP_IR_RECV_PIN		A2
#endif

#define HAP_IR_RECEIVE_BUFFER_SIZE	1024
#define HAP_IR_RECEIVE_TIMEOUT		150 // ms

#endif


#ifndef HAP_IR_LED_PIN
#define HAP_IR_LED_PIN      14
#endif




class HAPPluginIR: public HAPPlugin {
public:

	HAPPluginIR();
	HAPAccessory* initAccessory() override;
	
	bool begin();

#if HAP_PLUGIN_IR_ENABLE_RECV   
	void changePower(bool oldValue, bool newValue);
#endif	

	void handleImpl(bool forced=false);
	
	
	// void handleEvents(int eventCode, struct HAPEvent eventParam);
	HAPConfigValidationResult validateConfig(JsonObject object);
	JsonObject getConfigImpl();
	void setConfigImpl(JsonObject root);

	static void setupIRSend();

	static IRsend* getIRSend(){
		return _irsend;
	}
#if HAP_PLUGIN_IR_ENABLE_RECV   
	bool receiveIRSignal();
#endif
private:	
	
	static uint8_t _gpioIRSend;	
	static IRsend* _irsend;

#if HAP_PLUGIN_IR_ENABLE_RECV   	
	IRrecv* _irrecv;
	uint8_t _gpioIRRecv;	
	decode_results _results;  // Somewhere to store the results
	bool _isOn;
	boolCharacteristics* 	_powerState;
#endif
};

REGISTER_PLUGIN(HAPPluginIR)

#endif /* HAPPLUGINIR_HPP_ */ 