//
// HAPPluginRF24.hpp
// Homekit
//
//  Created on: 20.05.2020
//      Author: michael
//

#ifndef HAPPLUGINRF24_HPP_
#define HAPPLUGINRF24_HPP_

#include <Arduino.h>
#include <vector>
#include <algorithm>

#include "HAPPlugins.hpp"
#include "HAPLogger.hpp"
#include "HAPAccessory.hpp"

#include "RF24.h"
#include "HAPPluginRF24Device.hpp"
	

enum HAP_RF24_REMOTE_TYPE {
    HAP_RF24_REMOTE_TYPE_NONE       = 0x00,
    HAP_RF24_REMOTE_TYPE_WEATHER    = 0x01
};

struct HAP_RF24_PAYLOAD {
    uint8_t     id;
    HAP_RF24_REMOTE_TYPE     type;

    union {
        uint32_t    temp;
        uint32_t    hum;
        uint32_t    pres;
    };
};

class HAPPluginRF24: public HAPPlugin {
public:

	HAPPluginRF24();
    ~HAPPluginRF24();

	bool begin();

	HAPAccessory* initAccessory() override;
	
	void setValue(int iid, String oldValue, String newValue);
	
	void identify(bool oldValue, bool newValue);
    void handleImpl(bool forced = false);	


	HAPConfigValidationResult validateConfig(JsonObject object);
	JsonObject getConfigImpl();
	void setConfigImpl(JsonObject root);

private:	

	int indexOfDevice(HAPPluginRF24Device* device);
    void configAccessory(uint8_t devPtr);

	HAPConfigValidationResult validateName(const char* name);
	HAPConfigValidationResult validateAddress(const char* address);
	HAPConfigValidationResult validateType(const char* type);


	std::vector<HAPPluginRF24Device*>	_devices;
    RF24* _radio;

	// HAPPluginRF24Device* _newDevice;

	bool fakeGatoCallback();
};

REGISTER_PLUGIN(HAPPluginRF24)

#endif /* HAPPLUGINRF24_HPP_ */ 