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

#define USE_IR32 0

#if USE_IR32
	#include <IRSend32.h>
#else
	#include <IRremoteESP8266.h>
	#include <IRsend.h>
#endif


#define HAP_PLUGIN_IR_ENABLE_RECV 	0		// Not yet working :(

#if HAP_PLUGIN_IR_ENABLE_RECV 

#if USE_IR32
	#include <IRRecv32.h>

#else
	#include <IRrecv.h>
	#include <IRutils.h>
	#include <IRac.h>
	#include <IRtext.h>
#endif
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

#if USE_IR32
	static IRSend32* _irsend;
#else
	static IRsend* _irsend;
#endif


#if HAP_PLUGIN_IR_ENABLE_RECV   

#if USE_IR32
	IRrecv32* _irrecv;
#else
	decode_results _results;  // Somewhere to store the results
	IRrecv* _irrecv;
#endif
	
	uint8_t _gpioIRRecv;		
	bool _isOn;
	boolCharacteristics* 	_powerState;
#endif
};

REGISTER_PLUGIN(HAPPluginIR)

#endif /* HAPPLUGINIR_HPP_ */ 