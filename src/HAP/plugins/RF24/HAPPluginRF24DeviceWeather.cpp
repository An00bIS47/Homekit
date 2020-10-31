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
#include "HAPCustomCharacteristics+Services.hpp"

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
	_measureMode        = nullptr;
    _heartbeat          = nullptr;

    // _firmwareVersion    = HAPVersion("0.0.0");
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
	_measureMode        = nullptr;
    _heartbeat          = nullptr;
    // _firmwareVersion    = HAPVersion("0.0.0");
}



HAPAccessory* HAPPluginRF24DeviceWeather::initAccessory(){
    // Create accessory if not already created
    _accessory = new HAPAccessory();
    //HAPAccessory::addInfoServiceToAccessory(_accessory, "Builtin LED", "ACME", "LED", "123123123", &identify);
    auto callbackIdentify = std::bind(&HAPPluginRF24Device::identify, this, std::placeholders::_1, std::placeholders::_2);
    _accessory->addInfoService(String("Remote Weather ") + String(name), "ACME", "ATTiny85 Remote Weather", String(id, HEX), callbackIdentify, "unknown");


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
	tempServiceName->setValue("Remote Weather " + String(id));

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
    _lastUpdate = new stringCharacteristics(HAP_CHARACTERISTIC_FAKEGATO_OBSERVATION_TIME, permission_read|permission_notify, 32);
    _lastUpdate->setDescription("Observation Time");
    _lastUpdate->setValue("Never");

    auto callbackChangeLastUpdate = std::bind(&HAPPluginRF24DeviceWeather::changeLastUpdate, this, std::placeholders::_1, std::placeholders::_2);
    _lastUpdate->valueChangeFunctionCall = callbackChangeLastUpdate;
    _accessory->addCharacteristics(temperatureService, _lastUpdate);


    // 
    // Measure Mode
	// is bound to temperature service
    // 
    uint8_t validValues[2] = {0,1};
    _measureMode = new uint8Characteristics(HAP_CUSTOM_CHARACTERISTICS_MEASURE_MODE, permission_read|permission_write, 0, 1, 1, unit_none, 2, validValues);
    _measureMode->setDescription("Measure Mode");
    _measureMode->setValue(String((uint8_t) measureMode));    // 0 indoor, 1 outdoor

    auto callbackChangeMeasureMode = std::bind(&HAPPluginRF24DeviceWeather::changeMeasureMode, this, std::placeholders::_1, std::placeholders::_2);
    _measureMode->valueChangeFunctionCall = callbackChangeMeasureMode;
    _accessory->addCharacteristics(temperatureService, _measureMode);


    // 
    // Heartbeat
	// is bound to temperature service
    // 
    uint8_t validValuesHeartbeat[15] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    _heartbeat = new uint8Characteristics(HAP_CUSTOM_CHARACTERISTICS_HEARTBEAT, permission_read|permission_write, 1, 15, 1, unit_none, 15, validValuesHeartbeat);
    _heartbeat->setDescription("Heartbeat");
    _heartbeat->setValue(String(sleepInterval)); 

    auto callbackChangeHeartbeat = std::bind(&HAPPluginRF24DeviceWeather::changeHeartbeat, this, std::placeholders::_1, std::placeholders::_2);
    _heartbeat->valueChangeFunctionCall = callbackChangeHeartbeat;
    _accessory->addCharacteristics(temperatureService, _heartbeat);


	//
	// FakeGato
	// 		
	_fakegato.registerFakeGatoService(_accessory, name);    
	auto callbackAddEntry = std::bind(&HAPPluginRF24DeviceWeather::fakeGatoCallback, this);
	_fakegatoFactory->registerFakeGato(&_fakegato,  String("Remote Weather ") + String(id, HEX), callbackAddEntry);

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
    Serial.printf("[RF24:%X] New LastUpdate: %s\n", id, newValue.c_str());
}

void HAPPluginRF24DeviceWeather::changeMeasureMode(uint8_t oldValue, uint8_t newValue){
    Serial.printf("[RF24:%X] New Measure Mode: %d\n", id, newValue);

    if (oldValue != newValue){
        NewSettingsPacket newSettings;
        
        newSettings.forRadioId = id;
        newSettings.changeType = ChangeMeasureType;
        newSettings.newRadioId = 0;
        newSettings.newSleepInterval = 0;
        newSettings.newMeasureMode = newValue;    

#if HAP_DEBUG_RF24
        LogD(HAPServer::timeString() + " New Settings for " + String(newSettings.forRadioId, HEX), true);
        LogD(HAPServer::timeString() + "   changeType: " + String(newSettings.changeType, HEX), true);
        LogD(HAPServer::timeString() + "   newRadioId: " + String(newSettings.newRadioId, HEX), true);
        LogD(HAPServer::timeString() + "   newMeasureMode: " + String(newSettings.newMeasureMode, HEX), true);
        LogD(HAPServer::timeString() + "   sleepInterval: " + String(newSettings.newSleepInterval), true);        

        LogD(HAPServer::timeString() + "   Size of struct: " + String(sizeof(NewSettingsPacket)), true);        
#endif

        _callbackSendSettings(newSettings);
		
        // ToDo: Check if required
        // _eventManager->queueEvent( EventManager::kEventUpdatedConfig, HAPEvent());            	
    }
}

void HAPPluginRF24DeviceWeather::changeHeartbeat(uint8_t oldValue, uint8_t newValue){
    Serial.printf("[RF24:%X] New Heartbeat: %d\n", id, newValue);

    if (oldValue != newValue){
        NewSettingsPacket newSettings;
        
        newSettings.forRadioId = id;
        newSettings.changeType = ChangeSleepInterval;
        newSettings.newRadioId = 0;
        newSettings.newSleepInterval = newValue;
        newSettings.newMeasureMode = 0;    

#if HAP_DEBUG_RF24
        LogD(HAPServer::timeString() + " New Settings for " + String(newSettings.forRadioId, HEX), true);
        LogD(HAPServer::timeString() + "   changeType: " + String(newSettings.changeType, HEX), true);
        LogD(HAPServer::timeString() + "   newRadioId: " + String(newSettings.newRadioId, HEX), true);
        LogD(HAPServer::timeString() + "   newMeasureMode: " + String(newSettings.newMeasureMode, HEX), true);
        LogD(HAPServer::timeString() + "   sleepInterval: " + String(newSettings.newSleepInterval), true);        

        LogD(HAPServer::timeString() + "   Size of struct: " + String(sizeof(NewSettingsPacket)), true);        
#endif

        _callbackSendSettings(newSettings);

        // ToDo: Check if required
        // _eventManager->queueEvent( EventManager::kEventUpdatedConfig, HAPEvent());            	
    }

}


void HAPPluginRF24DeviceWeather::changeFirmware(String oldValue, String newValue){
    Serial.printf("[RF24:%d] New Firmware: %s\n", id, newValue.c_str());    
}

// void HAPPluginRF24DeviceWeather::identify(bool oldValue, bool newValue) {
//     printf("Start Identify rf24: %d\n", id);
// }

bool HAPPluginRF24DeviceWeather::fakeGatoCallback(){	
	// return _fakegato.addEntry(_temperatureValue->value(), _humidityValue->value(), _pressureValue->value());
	return _fakegato.addEntry(0x07, _temperatureValue->value(), _humidityValue->value(), _pressureValue->value());
}



void HAPPluginRF24DeviceWeather::setValuesFromPayload(struct RadioPacket payload){

    LogD(HAPServer::timeString() + " Setting values for remote weather device ...", false);
	
	_humidityValue->setValue(String(payload.humidity / 100.0));
	_temperatureValue->setValue(String(payload.temperature / 100.0));
	_pressureValue->setValue(String(payload.pressure / 100.0));


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

    LogD(HAPServer::timeString() + " Setting config for remote weather device ...", false);

    measureMode 	= (enum MeasureMode) settings.measureMode;
    sleepInterval 	= settings.sleepInterval;
    
    _accessory->setFirmware(HAPVersion(settings.firmware_version).toString());

	_measureMode->setValue(String((uint8_t) measureMode ));	
    _heartbeat->setValue(String(sleepInterval));	

    LogD(" OK", true);
}