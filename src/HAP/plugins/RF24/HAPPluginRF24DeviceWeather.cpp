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
    id 		= 0;
    type    = RemoteDeviceTypeWeather;

    _accessory          = nullptr;
    _eventManager       = nullptr;  
    _fakegatoFactory    = nullptr;

	_batteryLevel       = nullptr;
    _batteryStatus      = nullptr;

	_lastUpdate			= nullptr;
}

HAPPluginRF24DeviceWeather::HAPPluginRF24DeviceWeather(uint16_t id_, String name_, uint8_t measureMode_){    
    name    			= name_;    
    id 					= id_;
    type    			= RemoteDeviceTypeWeather;
	measureMode 		= (enum MeasureMode) measureMode_;

    _accessory          = nullptr;
    _eventManager       = nullptr;      
    _fakegatoFactory    = nullptr;

	_batteryLevel       = nullptr;
    _batteryStatus      = nullptr;

	_lastUpdate			= nullptr;
}




HAPAccessory* HAPPluginRF24DeviceWeather::initAccessory(){
    // Create accessory if not already created
    _accessory = new HAPAccessory();
    //HAPAccessory::addInfoServiceToAccessory(_accessory, "Builtin LED", "ACME", "LED", "123123123", &identify);
    auto callbackIdentify = std::bind(&HAPPluginRF24Device::identify, this, std::placeholders::_1, std::placeholders::_2);
    _accessory->addInfoService(name, "ACME", "RF24", String("Remote Weather ") +String(id, HEX), callbackIdentify, "1.0");


    //
    // Battery service
    // 
    HAPService* batteryService = new HAPService(HAP_SERVICE_BATTERY_SERVICE);
    _accessory->addService(batteryService);

    // 
    // Battery level
    // 
    _batteryLevel = new intCharacteristics(HAP_CHARACTERISTIC_BATTERY_LEVEL, permission_read|permission_notify, 0, 100, 1, unit_percentage);
    _batteryLevel->setValue("0");
    auto callbackChangeBatLevel = std::bind(&HAPPluginRF24DeviceWeather::changeBatteryLevel, this, std::placeholders::_1, std::placeholders::_2);
    _batteryLevel->valueChangeFunctionCall = callbackChangeBatLevel;
    _accessory->addCharacteristics(batteryService, _batteryLevel);

    // 
    // Battery status
    // 
    _batteryStatus = new intCharacteristics(HAP_CHARACTERISTIC_STATUS_LOW_BATTERY, permission_read|permission_notify, 0, 1, 1, unit_none);
    _batteryStatus->setValue("0");
    auto callbackChangeBatStatus = std::bind(&HAPPluginRF24DeviceWeather::changeBatteryStatus, this, std::placeholders::_1, std::placeholders::_2);
    _batteryStatus->valueChangeFunctionCall = callbackChangeBatStatus;
    _accessory->addCharacteristics(batteryService, _batteryStatus);

    // 
    // Charging State
    // 
    intCharacteristics *chargingState = new intCharacteristics(HAP_CHARACTERISTIC_CHARGING_STATE, permission_read, 0, 2, 1, unit_none);
    chargingState->setValue("2");
    _accessory->addCharacteristics(batteryService, chargingState);


	//
	// Temperature
	//
	HAPService* temperatureService = new HAPService(HAP_SERVICE_TEMPERATURE_SENSOR);
	_accessory->addService(temperatureService);

	stringCharacteristics *tempServiceName = new stringCharacteristics(HAP_CHARACTERISTIC_NAME, permission_read, 0);
	tempServiceName->setValue("Remote Temperature Sensor " + String(id));

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
	humServiceName->setValue("Remote Humidity Sensor " + String(id));
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
	pressureServiceName->setValue("Remote Pressure Sensor " + String(id));
	_accessory->addCharacteristics(pressureService, pressureServiceName);
	
	_pressureValue = new uint16Characteristics(HAP_CHARACTERISTIC_FAKEGATO_AIR_PRESSURE, permission_read|permission_notify, 0, 1100, 1, unit_hpa);
	_pressureValue->setValue("320");

	auto callbackChangePressure = std::bind(&HAPPluginRF24DeviceWeather::changePressure, this, std::placeholders::_1, std::placeholders::_2);
	//_humidityValue->valueChangeFunctionCall = std::bind(&changeHum);
	_pressureValue->valueChangeFunctionCall = callbackChangePressure;
	_accessory->addCharacteristics(pressureService, _pressureValue);


    // 
    // Last Update characteristics  (Custom)
    // is bound to temperature service
    // 
    _lastUpdate = new stringCharacteristics("000003EA-6B66-4FFD-88CC-16A60B5C4E03", permission_read|permission_notify, 32);
    _lastUpdate->setDescription("LastUpdate");
    _lastUpdate->setValue("Never");

    auto callbackChangeLastUpdate = std::bind(&HAPPluginRF24DeviceWeather::changeLastUpdate, this, std::placeholders::_1, std::placeholders::_2);
    _lastUpdate->valueChangeFunctionCall = callbackChangeLastUpdate;
    _accessory->addCharacteristics(temperatureService, _lastUpdate);




	//
	// FakeGato
	// 		
	_fakegato.registerFakeGatoService(_accessory, name);    
	auto callbackAddEntry = std::bind(&HAPPluginRF24DeviceWeather::fakeGatoCallback, this);
	_fakegatoFactory->registerFakeGato(&_fakegato,  String("RF24 ") + String(id, HEX), callbackAddEntry);

    return _accessory;
}

void HAPPluginRF24DeviceWeather::setEventManager(EventManager* eventManager){
      
    _eventManager = eventManager;
    // Serial.printf("w event: %p\n", _eventManager);  
}


void HAPPluginRF24DeviceWeather::setFakeGatoFactory(HAPFakeGatoFactory* fakegatoFactory){
    
    _fakegatoFactory = fakegatoFactory;
    // Serial.printf("w fakegato: %p\n", _fakegatoFactory);
}   


void HAPPluginRF24DeviceWeather::changeTemp(float oldValue, float newValue) {
	Serial.printf("[RF24:%X] New temperature: %f\n", id, newValue);
}

void HAPPluginRF24DeviceWeather::changeHum(float oldValue, float newValue) {
	Serial.printf("[RF24:%X] New humidity: %f\n", id, newValue);
}

void HAPPluginRF24DeviceWeather::changePressure(uint16_t oldValue, uint16_t newValue) {
	Serial.printf("[RF24:%X] New pressure: %d\n", id, newValue);
}

void HAPPluginRF24DeviceWeather::changeBatteryLevel(float oldValue, float newValue) {
	Serial.printf("[RF24:%X] New battery Level: %f\n", id, newValue);
}

void HAPPluginRF24DeviceWeather::changeBatteryStatus(float oldValue, float newValue) {
	Serial.printf("[RF24:%X] New battery status: %f\n", id, newValue);
}

void HAPPluginRF24DeviceWeather::changeLastUpdate(String oldValue, String newValue){
    Serial.printf("[RF24:%d] New LastUpdate: %s\n", id, newValue.c_str());
}

// void HAPPluginRF24DeviceWeather::identify(bool oldValue, bool newValue) {
//     printf("Start Identify rf24: %d\n", id);
// }

bool HAPPluginRF24DeviceWeather::fakeGatoCallback(){	
	// return _fakegato.addEntry(_temperatureValue->value(), _humidityValue->value(), _pressureValue->value());
	return _fakegato.addEntry(_temperatureValue->value(), _humidityValue->value(), _pressureValue->value());
}



void HAPPluginRF24DeviceWeather::setValuesFromPayload(struct RadioPacket payload){

	LogD("Setting values for remote weather device ...", false);
	_humidityValue->setValue(String(payload.humidity * 1.0));
	_temperatureValue->setValue(String(payload.temperature * 1.0));
	_pressureValue->setValue(String(payload.pressure));


	uint8_t percentage = map(payload.voltage * 10, REMOTE_DEVICE_VMIN, REMOTE_DEVICE_VMAX, 0, 100);

	_batteryLevel->setValue(String(percentage));	
	_batteryStatus->setValue(payload.voltage < HAP_BATTERY_LEVEL_LOW_THRESHOLD ? "1" : "0" );

	_lastUpdate->setValue(HAPServer::timeString());

	{
		struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _humidityValue->iid, _humidityValue->value());		 					
		_eventManager->queueEvent( EventManager::kEventNotifyController, event);
	}

	{
		struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _temperatureValue->iid, _temperatureValue->value());							
		_eventManager->queueEvent( EventManager::kEventNotifyController, event);
	}

	{
		struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _pressureValue->iid, _pressureValue->value());							
		_eventManager->queueEvent( EventManager::kEventNotifyController, event);
	}	


	{        
        struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _batteryLevel->iid, String(payload.voltage));
        _eventManager->queueEvent( EventManager::kEventNotifyController, event);        
    }
    
    {        
        struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _batteryStatus->iid, payload.voltage < HAP_BATTERY_LEVEL_LOW_THRESHOLD ? "1" : "0");
        _eventManager->queueEvent( EventManager::kEventNotifyController, event);        
    }

	{        
        struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _lastUpdate->iid, _lastUpdate->value());
        _eventManager->queueEvent( EventManager::kEventNotifyController, event);        
    }

	LogD(" OK", true);
}

void HAPPluginRF24DeviceWeather::setSettingsFromPayload(struct RemoteDeviceSettings settings){
    measureMode 	= (enum MeasureMode) settings.measureMode;
    sleepInterval 	= settings.sleepInterval;
}