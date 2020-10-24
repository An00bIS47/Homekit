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
#include "HAPGlobals.hpp"

#include <IRremoteESP8266.h>
#include <IRsend.h>

// 
// Set these values in the HAPGlobals.hpp
// 
#ifndef HAP_PLUGIN_IR_SEND_PIN
#define HAP_PLUGIN_IR_SEND_PIN      		14
#endif

#ifndef HAP_PLUGIN_IR_SEND_PIN
#define HAP_PLUGIN_IR_ENABLE_RECV 			1		
#endif

#if HAP_PLUGIN_IR_ENABLE_RECV 

#include <vector>
#include <algorithm>
#include "HAPPluginIRDevice.hpp"

#include <IRutils.h>
#include <IRac.h>
#include <IRtext.h>

#ifndef HAP_PLUGIN_IR_RECV_PIN
#define HAP_PLUGIN_IR_RECV_PIN				A2
#endif

#ifndef HAP_PLUGIN_IR_RECEIVE_BUFFER_SIZE
#define HAP_PLUGIN_IR_RECEIVE_BUFFER_SIZE	1024
#endif

#ifndef HAP_PLUGIN_IR_RECEIVE_TIMEOUT
#define HAP_PLUGIN_IR_RECEIVE_TIMEOUT		150 // ms
#endif

#ifndef HAP_PLUGIN_IR_RECEIVE_TIMER
#define HAP_PLUGIN_IR_RECEIVE_TIMER			2
#endif

#endif /* HAP_PLUGIN_IR_ENABLE_RECV */

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

	std::vector<HAPPluginIRDevice*>	_devices;
	int indexOfDevice(HAPPluginIRDevice* device);
	int indexOfDecodeResult(decode_results* result);

	decode_results _results;  // Somewhere to store the results
	IRrecv* _irrecv;
	
	uint8_t _gpioIRRecv;		
	bool _isOn;
	boolCharacteristics* 	_powerState;
#endif
};

REGISTER_PLUGIN(HAPPluginIR)

#endif /* HAPPLUGINIR_HPP_ */ 