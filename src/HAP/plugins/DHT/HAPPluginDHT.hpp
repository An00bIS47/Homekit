//
// HAPPluginDHT.hpp
// Homekit
//
//  Created on: 22.04.2018
//      Author: michael
//
// Pinout:
//  _______
// |#######|
// |#######|
// |#######|
// |_______|
//  | | | |
//  | | | |
// 3v H - GND
//    H | = Not connected
//    H => Resistor: 10k 
//    H 
//    |
//    15 
// 


#ifndef HAPPLUGINDHT_HPP_
#define HAPPLUGINDHT_HPP_

#include <Arduino.h>
#include "HAPPlugins.hpp"
#include "HAPLogger.hpp"
#include "HAPAccessory.hpp"

#include "HAPFakeGato.hpp"
#include "HAPGlobals.hpp"

// 
// Set these values in the HAPGlobals.hpp
// 
#ifndef HAP_PLUGIN_DHT_USE_DUMMY
#define HAP_PLUGIN_DHT_USE_DUMMY 	0
#endif

#ifndef HAP_PLUGIN_DHT_USE_PRESSURE
#define HAP_PLUGIN_DHT_USE_PRESSURE 0
#endif

#ifndef DHTPIN
#define DHTPIN 		A8 	// 15
#endif

#ifndef DHTTYPE
#define DHTTYPE    	DHT22
#endif

#include <Adafruit_Sensor.h>

#if HAP_PLUGIN_DHT_USE_DUMMY
#else
#include <DHT.h>
#include <DHT_U.h>
#endif





// #if HAP_PLUGIN_DHT_USE_PRESSURE
#include "HAPFakeGatoWeather.hpp"
// #else

// #endif


// #include "HAPFakeGatoWeather.hpp"

class HAPPluginDHT: public HAPPlugin {
public:

	HAPPluginDHT();

	bool begin();
	HAPAccessory* initAccessory() override;
	
	void setValue(int iid, String oldValue, String newValue);	
	
	void changeTemp(float oldValue, float newValue);
	void changeHum(float oldValue, float newValue);

#if HAP_PLUGIN_DHT_USE_PRESSURE	
	void changePressure(uint16_t oldValue, uint16_t newValue);
#endif


	void identify(bool oldValue, bool newValue);
    void handleImpl(bool forced = false);	
	
	HAPConfigValidationResult validateConfig(JsonObject object);
	JsonObject getConfigImpl();
	void setConfigImpl(JsonObject root);
	// void handleEvents(int eventCode, struct HAPEvent eventParam);
private:

	
	// HAPAccessory*			_accessory;

	floatCharacteristics*	_humidityValue;
	floatCharacteristics*	_temperatureValue;
#if HAP_PLUGIN_DHT_USE_PRESSURE	
	uint16Characteristics*	_pressureValue;
#endif

#if HAP_PLUGIN_DHT_USE_DUMMY
#else
	DHT_Unified* _dht;
#endif

	bool fakeGatoCallback();

// #if HAP_PLUGIN_DHT_USE_PRESSURE	

// #else
	HAPFakeGatoWeather _fakegato;
// #endif


};

REGISTER_PLUGIN(HAPPluginDHT)

#endif /* HAPPLUGINS_HPP_ */ 