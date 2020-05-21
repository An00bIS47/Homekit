//
// HAPPluginRF24DeviceWeather.cpp
// Homekit
//
//  Created on: 20.05.2020
//      Author: michael
//

#include "HAPPluginRF24DeviceWeather.hpp"
#include "HAPServer.hpp"
#include "HAPLogger.hpp"
#include "HAPPluginRF24.hpp"


HAPPluginRF24DeviceWeather::HAPPluginRF24DeviceWeather(){   
    name    = "";    
    address = 0;
    type    = HAP_RF24_REMOTE_TYPE_WEATHER;

    _accessory          = nullptr;
    _eventManager       = nullptr;  
    _fakegatoFactory    = nullptr;
}

HAPPluginRF24DeviceWeather::HAPPluginRF24DeviceWeather(uint8_t address_, String name_){
    
    name    = name_;    
    address = address_;
    type    = HAP_RF24_REMOTE_TYPE_WEATHER;

    _accessory          = nullptr;
    _eventManager       = nullptr;      
    _fakegatoFactory    = nullptr;
}


HAPAccessory* HAPPluginRF24DeviceWeather::initAccessory(){

    // Create accessory if not already created
    _accessory = new HAPAccessory();
    //HAPAccessory::addInfoServiceToAccessory(_accessory, "Builtin LED", "ACME", "LED", "123123123", &identify);
    auto callbackIdentify = std::bind(&HAPPluginRF24Device::identify, this, std::placeholders::_1, std::placeholders::_2);
    _accessory->addInfoService(name, "ACME", "RF Weather", "0x01", callbackIdentify, "1.0");


	//
	// Temperature
	//
	HAPService* temperatureService = new HAPService(HAP_SERVICE_TEMPERATURE_SENSOR);
	_accessory->addService(temperatureService);

	stringCharacteristics *tempServiceName = new stringCharacteristics(HAP_CHARACTERISTIC_NAME, permission_read, 0);
	tempServiceName->setValue("Temperature Sensor");

	_accessory->addCharacteristics(temperatureService, tempServiceName);

	//floatCharacteristics(uint8_t _type, int _permission, float minVal, float maxVal, float step, unit charUnit): characteristics(_type, _permission), _minVal(minVal), _maxVal(maxVal), _step(step), _unit(charUnit)
	_temperatureValue = new floatCharacteristics(HAP_CHARACTERISTIC_CURRENT_TEMPERATURE, permission_read|permission_notify, -50, 100, 0.1, unit_celsius);
	_temperatureValue->setValue("0.0");
	auto callbackChangeTemp = std::bind(&HAPPluginRF24DeviceWeather::changeTemp, this, std::placeholders::_1, std::placeholders::_2);
	//_temperatureValue->valueChangeFunctionCall = std::bind(&changeTemp);
	_temperatureValue->valueChangeFunctionCall = callbackChangeTemp;
	_accessory->addCharacteristics(temperatureService, _temperatureValue);

	//
	// Humidity
	//
	HAPService* humidityService = new HAPService(HAP_SERVICE_HUMIDITY_SENSOR);
	_accessory->addService(humidityService);

	stringCharacteristics *humServiceName = new stringCharacteristics(HAP_CHARACTERISTIC_NAME, permission_read, 0);
	humServiceName->setValue("Humidity Sensor");
	_accessory->addCharacteristics(humidityService, humServiceName);

	_humidityValue = new floatCharacteristics(HAP_CHARACTERISTIC_CURRENT_RELATIVE_HUMIDITY, permission_read|permission_notify, 0, 100, 0.1, unit_percentage);
	_humidityValue->setValue("0.0");

	auto callbackChangeHum = std::bind(&HAPPluginRF24DeviceWeather::changeHum, this, std::placeholders::_1, std::placeholders::_2);
	//_humidityValue->valueChangeFunctionCall = std::bind(&changeHum);
	_humidityValue->valueChangeFunctionCall = callbackChangeHum;
	_accessory->addCharacteristics(humidityService, _humidityValue);


	//
	// AirPressure
	//
	HAPService* pressureService = new HAPService(HAP_SERVICE_FAKEGATO_AIR_PRESSURE_SENSOR);
	_accessory->addService(pressureService);

	stringCharacteristics *pressureServiceName = new stringCharacteristics(HAP_CHARACTERISTIC_NAME, permission_read, 0);
	pressureServiceName->setValue("Pressure Sensor");
	_accessory->addCharacteristics(pressureService, pressureServiceName);
	
	_pressureValue = new uint16Characteristics(HAP_CHARACTERISTIC_FAKEGATO_AIR_PRESSURE, permission_read|permission_notify, 0, 1100, 1, unit_hpa);
	_pressureValue->setValue("320");

	auto callbackChangePressure = std::bind(&HAPPluginRF24DeviceWeather::changePressure, this, std::placeholders::_1, std::placeholders::_2);
	//_humidityValue->valueChangeFunctionCall = std::bind(&changeHum);
	_pressureValue->valueChangeFunctionCall = callbackChangePressure;
	_accessory->addCharacteristics(pressureService, _pressureValue);


	//
	// FakeGato
	// 		
	_fakegato.registerFakeGatoService(_accessory, name);    
	auto callbackAddEntry = std::bind(&HAPPluginRF24DeviceWeather::fakeGatoCallback, this);
	_fakegatoFactory->registerFakeGato(&_fakegato,  "RF24 " + String(address), callbackAddEntry);


    return _accessory;
}

void HAPPluginRF24DeviceWeather::setValue(int iid, String oldValue, String newValue){
	if (iid == _temperatureValue->iid) {		
		_temperatureValue->setValue(newValue);
	} else if (iid == _humidityValue->iid) {
		_humidityValue->setValue(newValue);
	} else if (iid == _pressureValue->iid) {
		_pressureValue->setValue(newValue);
	}

	// Add event
	struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, iid, newValue);							
	_eventManager->queueEvent( EventManager::kEventNotifyController, event);
}


void HAPPluginRF24DeviceWeather::changeTemp(float oldValue, float newValue) {
	Serial.printf("[%s] New temperature: %f\n", name.c_str(), newValue);
}

void HAPPluginRF24DeviceWeather::changeHum(float oldValue, float newValue) {
	Serial.printf("[%s] New humidity: %f\n", name.c_str(), newValue);
}

void HAPPluginRF24DeviceWeather::changePressure(uint16_t oldValue, uint16_t newValue) {
	Serial.printf("[%s] New pressure: %d\n", name.c_str(), newValue);
}



bool HAPPluginRF24DeviceWeather::fakeGatoCallback(){	
	// return _fakegato.addEntry(_temperatureValue->value(), _humidityValue->value(), _pressureValue->value());
	return _fakegato.addEntry(_temperatureValue->value(), _humidityValue->value(), _pressureValue->value());
}



