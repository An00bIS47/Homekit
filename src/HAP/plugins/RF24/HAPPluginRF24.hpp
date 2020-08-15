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
#include  <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

#include <vector>
#include <algorithm>

#include "HAPPlugins.hpp"
#include "HAPLogger.hpp"
#include "HAPAccessory.hpp"


#include "HAPPluginRF24Device.hpp"


class HAPPluginRF24: public HAPPlugin {
public:

	HAPPluginRF24();
    ~HAPPluginRF24();

	bool begin();

	HAPAccessory* initAccessory() override;
	
	
	
	// void identify(bool oldValue, bool newValue);
    void handleImpl(bool forced = false);	


	HAPConfigValidationResult validateConfig(JsonObject object);
	JsonObject getConfigImpl();
	void setConfigImpl(JsonObject root);

	void updateSettings(NewSettingsPacket newSettings);
private:	

	int indexOfDevice(uint16_t id);
    void configAccessory(uint8_t devPtr);

	bool readSettingsFromRadio();

	std::vector<HAPPluginRF24Device*>	_devices;
    RF24* _radio;

	bool _awaitSettingsConfirmation;
	bool _isInitialized;

	// HAPPluginRF24Device* _newDevice;

	bool fakeGatoCallback();

	uint8_t _messageCounter;
};

REGISTER_PLUGIN(HAPPluginRF24)

#endif /* HAPPLUGINRF24_HPP_ */ 