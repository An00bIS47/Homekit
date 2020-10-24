//
// HAPPluginRCSwitch.hpp
// Homekit
//
//  Created on: 20.12.2019
//      Author: michael
//
// 

#ifndef HAPPLUGINRCSWITCH_HPP_
#define HAPPLUGINRCSWITCH_HPP_

#include <Arduino.h>
#include <RCSwitch.h>
#include <vector>
#include <algorithm>

#include "HAPPlugins.hpp"
#include "HAPLogger.hpp"
#include "HAPAccessory.hpp"
#include "HAPGlobals.hpp"
#include "HAPFakeGato.hpp"


#include "HAPPluginRCSwitchDevice.hpp"


class HAPPluginRCSwitch: public HAPPlugin {
public:

	HAPPluginRCSwitch();
    // HAPPluginRCSwitch(String name_);

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

	boolCharacteristics*	_stateValue;

	int indexOfDevice(HAPPluginRCSwitchDevice* device);
    void configAccessory(uint8_t devPtr);
	std::vector<HAPPluginRCSwitchDevice*>	_devices;

    uint32_t bitStringToUInt32(char* input_binary_string);
    String uint32ToBitString(uint32_t dec);

	HAPConfigValidationResult validateName(const char* name);
	HAPConfigValidationResult validateDeviceAddress(const char* deviceAddress);
	HAPConfigValidationResult validateHouseAddress(const char* houseAddress);

    RCSwitch _rcSwitch;

	HAPPluginRCSwitchDevice* _newDevice;

	bool fakeGatoCallback();
	// HAPFakeGatoWeather _fakegato;

};

REGISTER_PLUGIN(HAPPluginRCSwitch)

#endif /* HAPPLUGINRCSWITCH_HPP_ */ 