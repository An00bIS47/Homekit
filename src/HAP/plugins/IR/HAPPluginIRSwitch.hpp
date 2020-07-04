//
// HAPPluginIRSwitch.hpp
// Homekit
//
//  Created on: 20.12.2019
//      Author: michael
//
// 

#ifndef HAPPLUGINIRSWITCH_HPP_
#define HAPPLUGINIRSWITCH_HPP_

#include <Arduino.h>
#include <vector>
#include <algorithm>
#include "IRremote.h"

#include "HAPPlugins.hpp"
#include "HAPLogger.hpp"
#include "HAPAccessory.hpp"

#include "HAPFakeGato.hpp"

#include "HAPPluginIRSwitchDevice.hpp"


class HAPPluginIRSwitch: public HAPPlugin {
public:

	HAPPluginIRSwitch();
    // HAPPluginIRSwitch(String name_);

	bool begin();

	HAPAccessory* initAccessory() override;
	
	void setValue(int iid, String oldValue, String newValue);
	
	void identify(bool oldValue, bool newValue);
    void handleImpl(bool forced = false);	

	HAPConfigValidationResult validateConfig(JsonObject object);
	JsonObject getConfigImpl();
	void setConfigImpl(JsonObject root);
	
    // void handleRoot(HTTPRequest * req, HTTPResponse * res);

	std::vector<HAPWebServerPluginNode*> getResourceNodes();
	
	void handleHTTPGet(HTTPRequest * req, HTTPResponse * res);	
	void handleHTTPGetKeyProcessor(const String& key, HTTPResponse * res);

	void handleHTTPPost(HTTPRequest * req, HTTPResponse * res);
	void handleHTTPFormField(const std::string& fieldName, const std::string& fieldValue);

    void sendDeviceCallback(uint8_t houseAddress_, uint8_t deviceAddress_, bool on_);    


    static void prependZeros(char *dest, String src, uint8_t width);
private:

	void dumpCode(decode_results *results);
	void dumpRaw(decode_results *results);
	void encoding(decode_results *results);
	void ircode(decode_results *results);

	boolCharacteristics*	_stateValue;

	int indexOfDevice(HAPPluginIRSwitchDevice* device);
    void configAccessory(uint8_t devPtr);
	std::vector<HAPPluginIRSwitchDevice*>	_devices;

    uint32_t bitStringToUInt32(char* input_binary_string);
    String uint32ToBitString(uint32_t dec);

	HAPConfigValidationResult validateName(const char* name);
	HAPConfigValidationResult validateDeviceAddress(const char* deviceAddress);
	HAPConfigValidationResult validateHouseAddress(const char* houseAddress);

    
	IRrecv* _irrecv;
	HAPPluginIRSwitchDevice* _newDevice;

	bool fakeGatoCallback();
	// HAPFakeGatoWeather _fakegato;

};

REGISTER_PLUGIN(HAPPluginIRSwitch)

#endif /* HAPPLUGINIRSWITCH_HPP_ */ 