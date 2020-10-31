//
// HAPPluginNimbleMiFloraDevice.cpp
// Homekit
//
//  Created on: 22.09.2019
//      Author: michael
//
#include "HAPPluginNimbleMiFloraDevice.hpp"
#include "HAPPluginNimbleMiFlora.hpp"
#include "HAPLogger.hpp"
#include "HAPServer.hpp"
#include "HAPCustomCharacteristics+Services.hpp"

#ifndef HAP_PLUGIN_MIFLORA_INTERVAL
#define HAP_PLUGIN_MIFLORA_INTERVAL		    120000
#endif

BLEUUID HAPPluginNimbleMiFloraDevice::_serviceUUID                 = BLEUUID::fromString("00001204-0000-1000-8000-00805f9b34fb");

BLEUUID HAPPluginNimbleMiFloraDevice::_uuid_write_mode             = BLEUUID::fromString("00001a00-0000-1000-8000-00805f9b34fb");
BLEUUID HAPPluginNimbleMiFloraDevice::_uuid_sensor_data            = BLEUUID::fromString("00001a01-0000-1000-8000-00805f9b34fb");
BLEUUID HAPPluginNimbleMiFloraDevice::_uuid_version_battery        = BLEUUID::fromString("00001a02-0000-1000-8000-00805f9b34fb");

#if HAP_PLUGIN_MIFLORA_ENABLE_HISTORY    
BLEUUID HAPPluginNimbleMiFloraDevice::_serviceHistoryUUID          = BLEUUID::fromString("00001206-0000-1000-8000-00805f9b34fb");

BLEUUID HAPPluginNimbleMiFloraDevice::_uuid_write_history_mode     = BLEUUID::fromString("00001a10-0000-1000-8000-00805f9b34fb");
BLEUUID HAPPluginNimbleMiFloraDevice::_uuid_history_read           = BLEUUID::fromString("00001a11-0000-1000-8000-00805f9b34fb");
BLEUUID HAPPluginNimbleMiFloraDevice::_uuid_device_time            = BLEUUID::fromString("00001a12-0000-1000-8000-00805f9b34fb");
#endif



HAPPluginNimbleMiFloraDevice::HAPPluginNimbleMiFloraDevice(BLEClient* bleClient, const std::string& address) : _deviceAddress(address){

    _accessory          = nullptr;
    _eventManager       = nullptr;  
    _fakegatoFactory    = nullptr;
		
	_firmwareValue      = nullptr;
	_lastUpdate         = nullptr;

	_humidityValue      = nullptr;
	_temperatureValue   = nullptr;   
	_lightValue         = nullptr;
	_fertilityValue     = nullptr;
	
	_batteryLevel       = nullptr;
	_batteryStatus      = nullptr;


    _floraClient        = bleClient;

    _name               = "MiFlora:" + String(_deviceAddress.c_str());
    
    _previousMillis     = HAP_PLUGIN_MIFLORA_INTERVAL;
    _interval           = HAP_PLUGIN_MIFLORA_INTERVAL;

#if HAP_PLUGIN_MIFLORA_ENABLE_HISTORY 
    _hasFetchedHistory  = false;
#endif  
}

HAPPluginNimbleMiFloraDevice::HAPPluginNimbleMiFloraDevice(BLEClient* bleClient, BLEAddress address) : _deviceAddress(address.toString()){    

    _accessory          = nullptr;
    _eventManager       = nullptr;  
    _fakegatoFactory    = nullptr;    
		
	_firmwareValue      = nullptr;
	_lastUpdate         = nullptr;

	_humidityValue      = nullptr;
	_temperatureValue   = nullptr;   
	_lightValue         = nullptr;
	_fertilityValue     = nullptr;
	
	_batteryLevel       = nullptr;
	_batteryStatus      = nullptr;  

    _floraClient        = bleClient;  
    _name               = "MiFlora:" + String(_deviceAddress.c_str());

    _previousMillis     = HAP_PLUGIN_MIFLORA_INTERVAL - 1000;
    _interval           = HAP_PLUGIN_MIFLORA_INTERVAL;

#if HAP_PLUGIN_MIFLORA_ENABLE_HISTORY 
    _hasFetchedHistory  = false;    
#endif    
}


bool HAPPluginNimbleMiFloraDevice::begin(){

    return true;
}

std::string HAPPluginNimbleMiFloraDevice::address(){
    return _deviceAddress;
}



HAPAccessory* HAPPluginNimbleMiFloraDevice::initAccessory(){
    
    String devId = _deviceAddress.c_str();
	devId = devId.substring(8);
	devId.replace(":", "");
	String sn = HAPDeviceID::serialNumber("MiFlora", String(devId));    

    
    _accessory = new HAPAccessory();
    //HAPAccessory::addInfoServiceToAccessory(_accessory, "Builtin LED", "ACME", "LED", "123123123", &identify);
    auto callbackIdentify = std::bind(&HAPPluginNimbleMiFloraDevice::identify, this, std::placeholders::_1, std::placeholders::_2);
    _accessory->addInfoService("MiFlora", "Xioami", "Flower Care", sn, callbackIdentify, "");

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
    auto callbackChangeBatLevel = std::bind(&HAPPluginNimbleMiFloraDevice::changeBatteryLevel, this, std::placeholders::_1, std::placeholders::_2);
    _batteryLevel->valueChangeFunctionCall = callbackChangeBatLevel;
    _accessory->addCharacteristics(batteryService, _batteryLevel);

    // 
    // Battery status
    // 
    _batteryStatus = new intCharacteristics(HAP_CHARACTERISTIC_STATUS_LOW_BATTERY, permission_read|permission_notify, 0, 1, 1, unit_none);
    _batteryStatus->setValue("0");
    auto callbackChangeBatStatus = std::bind(&HAPPluginNimbleMiFloraDevice::changeBatteryStatus, this, std::placeholders::_1, std::placeholders::_2);
    _batteryStatus->valueChangeFunctionCall = callbackChangeBatStatus;
    _accessory->addCharacteristics(batteryService, _batteryStatus);

    // 
    // Charging State
    // 
    intCharacteristics *chargingState = new intCharacteristics(HAP_CHARACTERISTIC_CHARGING_STATE, permission_read, 0, 2, 1, unit_none);
    chargingState->setValue("2");
    _accessory->addCharacteristics(batteryService, chargingState);


    //
    // Temperature service
    //
    HAPService* temperatureService = new HAPService(HAP_SERVICE_TEMPERATURE_SENSOR);
    _accessory->addService(temperatureService);

    stringCharacteristics *tempServiceName = new stringCharacteristics(HAP_CHARACTERISTIC_NAME, permission_read, 32);
    tempServiceName->setValue("Temperature Sensor");
    _accessory->addCharacteristics(temperatureService, tempServiceName);


    // 
    // Temperature value
    // 
    _temperatureValue = new floatCharacteristics(HAP_CHARACTERISTIC_CURRENT_TEMPERATURE, permission_read|permission_notify, -50, 100, 0.1, unit_celsius);
    _temperatureValue->setValue("0.0");
    
    auto callbackChangeTemp = std::bind(&HAPPluginNimbleMiFloraDevice::changeTemp, this, std::placeholders::_1, std::placeholders::_2);
    _temperatureValue->valueChangeFunctionCall = callbackChangeTemp;
    _accessory->addCharacteristics(temperatureService, _temperatureValue);

    // 
    // Last Update characteristics  (Custom)
    // is bound to temperature service
    // 
    _lastUpdate = new stringCharacteristics(HAP_CHARACTERISTIC_FAKEGATO_OBSERVATION_TIME, permission_read|permission_notify, 32);
    _lastUpdate->setDescription("Observation Time");
    _lastUpdate->setValue("Never");

    auto callbackChangeLastUpdate = std::bind(&HAPPluginNimbleMiFloraDevice::changeLastUpdate, this, std::placeholders::_1, std::placeholders::_2);
    _lastUpdate->valueChangeFunctionCall = callbackChangeLastUpdate;
    _accessory->addCharacteristics(temperatureService, _lastUpdate);



    //
    // Humidity service
    //
    // HAPService* humidityService = new HAPService(HAP_SERVICE_HUMIDITY_SENSOR);
    // _accessory->addService(humidityService);

    // stringCharacteristics *humServiceName = new stringCharacteristics(HAP_CHARACTERISTIC_NAME, permission_read, 32);
    // humServiceName->setValue("Moisture Sensor");
    // _accessory->addCharacteristics(humidityService, humServiceName);


    //
    // Humidity value
    //  
    _humidityValue = new floatCharacteristics(HAP_CHARACTERISTIC_CURRENT_RELATIVE_HUMIDITY, permission_read|permission_notify, 0, 100, 0.1, unit_percentage);
    _humidityValue->setValue("0.0");

    auto callbackChangeHum = std::bind(&HAPPluginNimbleMiFloraDevice::changeHum, this, std::placeholders::_1, std::placeholders::_2);
    _humidityValue->valueChangeFunctionCall = callbackChangeHum;
    _accessory->addCharacteristics(temperatureService, _humidityValue);

    //
    // AmbientLight == light
    //
    // HAPService* lightService = new HAPService(HAP_SERVICE_LIGHT_SENSOR);
    // _accessory->addService(lightService);

    // stringCharacteristics *lightServiceName = new stringCharacteristics(HAP_CHARACTERISTIC_NAME, permission_read, 32);
    // lightServiceName->setValue("Light Sensor");
    // _accessory->addCharacteristics(lightService, lightServiceName);

    _lightValue = new floatCharacteristics(HAP_CHARACTERISTIC_CURRENT_AMBIENT_LIGHT_LEVEL, permission_read|permission_notify, 0.0, 100000, 0.1, unit_lux);
    _lightValue->setValue("0.0");

    auto callbackChangeLight = std::bind(&HAPPluginNimbleMiFloraDevice::changeLight, this, std::placeholders::_1, std::placeholders::_2);
    _lightValue->valueChangeFunctionCall = callbackChangeLight;
    _accessory->addCharacteristics(temperatureService, _lightValue);

    //
    // Fertility Sensor (Custom)
    //
    // HAPService* fertilityService = new HAPService(HAP_CUSTOM_SERVICE_FERTILITY);
    // _accessory->addService(fertilityService);

    // stringCharacteristics *fertilityServiceName = new stringCharacteristics(HAP_CHARACTERISTIC_NAME, permission_read, 32);
    // fertilityServiceName->setValue("Fertility Sensor");    
    // _accessory->addCharacteristics(fertilityService, fertilityServiceName);

    _fertilityValue = new floatCharacteristics(HAP_CUSTOM_CHARACTERISTICS_FERTITLITY, permission_read|permission_notify, 0.0, 10000, 0.1, unit_none);
    _fertilityValue->setValue("0.0");
    _fertilityValue->setDescription("Fertility");


    auto callbackChangeFertility = std::bind(&HAPPluginNimbleMiFloraDevice::changeFertility, this, std::placeholders::_1, std::placeholders::_2);
    _fertilityValue->valueChangeFunctionCall = callbackChangeFertility;
    _accessory->addCharacteristics(temperatureService, _fertilityValue);

    // 
    // Heartbeat
	// is bound to temperature service
    // 
    uint8_t validValuesHeartbeat[5] = {1,2,3,4,5};
    _heartbeat = new uint8Characteristics(HAP_CUSTOM_CHARACTERISTICS_HEARTBEAT, permission_read|permission_write, 1, 5, 1, unit_none, 5, validValuesHeartbeat);
    _heartbeat->setDescription("Heartbeat");
    _heartbeat->setValue(String(2)); 

    auto callbackChangeHeartbeat = std::bind(&HAPPluginNimbleMiFloraDevice::changeHeartbeat, this, std::placeholders::_1, std::placeholders::_2);
    _heartbeat->valueChangeFunctionCall = callbackChangeHeartbeat;
    _accessory->addCharacteristics(temperatureService, _heartbeat);


    // 
    // FakeGato History
    // 
    // Get the last 3 bytes of the mac address
    String deviceAdd = _deviceAddress.c_str();
    deviceAdd.replace(":", "");
    deviceAdd = deviceAdd.substring(6);

    _fakegato.registerFakeGatoService(_accessory, "MiFlora " + deviceAdd);
	auto callbackAddEntry = std::bind(&HAPPluginNimbleMiFloraDevice::fakeGatoCallback, this);
	// _fakegato.registerCallback(callbackAddEntry);
	_fakegatoFactory->registerFakeGato(&_fakegato,  "MiFlora " + deviceAdd, callbackAddEntry);


    return _accessory;
}


void HAPPluginNimbleMiFloraDevice::setDeviceData(floraDeviceData data){
    LogD(HAPServer::timeString() + " " + "MiFloraDevice" + "->" + String(__FUNCTION__) + " [   ] " + "Set values ", true);

    _temperatureValue->setValue(String(data.temperature));
    _humidityValue->setValue(String(data.moisture));
    _lightValue->setValue(String(data.light));
    _fertilityValue->setValue(String(data.conductivity));

    _batteryLevel->setValue(String(data.battery));
    _batteryStatus->setValue(data.battery < HAP_BATTERY_LEVEL_LOW_THRESHOLD ? "1" : "0" );

    _lastUpdate->setValue(HAPServer::timeString());


    {    
        struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _temperatureValue->iid, String(data.temperature));							
        _eventManager->queueEvent( EventManager::kEventNotifyController, event);
    }

    {        
        struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _humidityValue->iid, String(data.moisture));
        _eventManager->queueEvent( EventManager::kEventNotifyController, event);
    }

    {        
        struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _lightValue->iid, String(data.light));
        _eventManager->queueEvent( EventManager::kEventNotifyController, event);
    }
    
    {        
        struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _fertilityValue->iid, String(data.conductivity));
        _eventManager->queueEvent( EventManager::kEventNotifyController, event);        
    }
    
    {        
        struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _batteryLevel->iid, String(data.battery));
        _eventManager->queueEvent( EventManager::kEventNotifyController, event);        
    }
    
    {        
        struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _batteryStatus->iid, data.battery < HAP_BATTERY_LEVEL_LOW_THRESHOLD ? "1" : "0");
        _eventManager->queueEvent( EventManager::kEventNotifyController, event);        
    }

    {        
        struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _lastUpdate->iid, _lastUpdate->value());
        _eventManager->queueEvent( EventManager::kEventNotifyController, event);   
    }
    
    _accessory->setFirmware(String(data.firmware));
}


void HAPPluginNimbleMiFloraDevice::setEventManager(EventManager* eventManager){
    _eventManager = eventManager;
}


void HAPPluginNimbleMiFloraDevice::setFakeGatoFactory(HAPFakeGatoFactory* fakegatoFactory){
    _fakegatoFactory = fakegatoFactory;
}   


void HAPPluginNimbleMiFloraDevice::changeTemp(float oldValue, float newValue) {
	Serial.printf("[MiFlora:%s] New temperature: %f\n", _deviceAddress.c_str(), newValue);
}

void HAPPluginNimbleMiFloraDevice::changeHum(float oldValue, float newValue) {
	Serial.printf("[MiFlora:%s] New humidity: %f\n", _deviceAddress.c_str(), newValue);
}

void HAPPluginNimbleMiFloraDevice::changeLight(float oldValue, float newValue) {
	Serial.printf("[MiFlora:%s] New light: %f\n", _deviceAddress.c_str(), newValue);
}

void HAPPluginNimbleMiFloraDevice::changeBatteryLevel(float oldValue, float newValue) {
	Serial.printf("[MiFlora:%s] New battery Level: %f\n", _deviceAddress.c_str(), newValue);
}

void HAPPluginNimbleMiFloraDevice::changeBatteryStatus(float oldValue, float newValue) {
	Serial.printf("[MiFlora:%s] New battery status: %f\n", _deviceAddress.c_str(), newValue);
}

void HAPPluginNimbleMiFloraDevice::changeFertility(float oldValue, float newValue) {
	Serial.printf("[MiFlora:%s] New fertility: %f\n", _deviceAddress.c_str(), newValue);
}

void HAPPluginNimbleMiFloraDevice::changeLastUpdate(String oldValue, String newValue){
    Serial.printf("[MiFlora:%s] New LastUpdate: %s\n", _deviceAddress.c_str(), newValue.c_str());
}

void HAPPluginNimbleMiFloraDevice::identify( bool oldValue, bool newValue) {
    printf("Start Identify MiFlora with address %s\n", _deviceAddress.c_str());

    blinkLED();
}

void HAPPluginNimbleMiFloraDevice::changeHeartbeat(uint8_t oldValue, uint8_t newValue){
    Serial.printf("[MiFlora:%s] New Heartbeat: %d\n", _deviceAddress.c_str(), newValue);

    if (oldValue != newValue){
        _interval = (uint32_t)((newValue * 60) * 1000);;
    }
}

bool HAPPluginNimbleMiFloraDevice::fakeGatoCallback(){
    // LogD(HAPServer::timeString() + " " + "HAPPluginPCA301Device" + "->" + String(__FUNCTION__) + " [   ] " + "fakeGatoCallback()", true);

    return _fakegato.addEntry(0x03, _temperatureValue->value(), _humidityValue->value(), "0");
	// 0102 0202 0302
	//	|	  |	   +-> Pressure	
	//  |	  +-> Humidity
	//  +-> Temp
	// 
	// 0x07 => all			= 111
	// 0x01 => temp			= 001
	// 0x02 => hum			= 010
	// 0x04 => pressure		= 100
}






/**************************************************************************************************************
 *  Bluetooth implementation
 *   
 */

BLEClient* HAPPluginNimbleMiFloraDevice::getFloraClient(BLEAddress floraAddress) {	

	try {
		if (!_floraClient->connect(floraAddress)) {
			LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Connection failed, skipping [" + String(floraAddress.toString().c_str()) + "]", true);
			return nullptr;
		}
	}
	catch (...) {
		// something went wrong
	}

	LogV(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Connection successful [" + String(floraAddress.toString().c_str()) + "]", true);
	return _floraClient;
}


BLERemoteService* HAPPluginNimbleMiFloraDevice::getFloraService(BLEUUID uuid) {
	BLERemoteService* floraService = nullptr;

	try {
		floraService = _floraClient->getService(uuid);
	}
	catch (...) {
		// something went wrong
	}
	if (floraService == nullptr) {
		LogW(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Failed to find data service [" + String(_floraClient->getPeerAddress().toString().c_str()) + "]", true);
	}
	else {
		LogV(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Found data service [" + String(_floraClient->getPeerAddress().toString().c_str()) + "]", true);
	}

	return floraService;
}

bool HAPPluginNimbleMiFloraDevice::forceFloraServiceDataMode(BLERemoteService* floraService, BLEUUID uuid, uint8_t* data, size_t dataLength) {
	BLERemoteCharacteristic* floraCharacteristic;
	
	// get device mode characteristic, needs to be changed to read data	
	LogV(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Force device in data mode", true);
	floraCharacteristic = nullptr;
	try {
		floraCharacteristic = floraService->getCharacteristic(uuid);
	}
	catch (...) {
		// something went wrong
	}
	if (floraCharacteristic == nullptr) {
		
		LogW(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Failed to force device in data mode", true);
		return false;
	}

	// write the magic data
	//uint8_t buf[2] = {0xA0, 0x1F};
	floraCharacteristic->writeValue(data, dataLength, true);

	delay(100);
	return true;
}

bool HAPPluginNimbleMiFloraDevice::readFloraDataCharacteristic(BLERemoteService* floraService) {
	BLERemoteCharacteristic* floraCharacteristic = nullptr;

	// get the main device data characteristic	
	LogV(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Read data from device", true);
	try {
		floraCharacteristic = floraService->getCharacteristic(_uuid_sensor_data);
	}
	catch (...) {
		// something went wrong
	}
	if (floraCharacteristic == nullptr) {
		LogW(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Failed to read characteristic", true);
		return false;
	}

	// read characteristic value
	//LogD("- Read value from characteristic", true);
	std::string value;
	try{
		value = floraCharacteristic->readValue();
	}
	catch (...) {
		// something went wrong
		LogW(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Failed to read characteristic", true);
		return false;
	}
	const char *val = value.c_str();

#if HAP_DEBUG_MIFLORA
    HAPHelper::array_print("value", (const unsigned char*)val, 16);
#endif

	int16_t* temp_raw = (int16_t*)val;
	float temperature = (*temp_raw) / ((float)10.0);

#if HAP_DEBUG_MIFLORA    
	Serial.print("-- Temperature: ");
	Serial.println(temperature);
#endif


	int moisture = val[7];

#if HAP_DEBUG_MIFLORA
	Serial.print("-- Moisture: ");
	Serial.println(moisture);
#endif

	int light = val[3] + val[4] * 256;
	
#if HAP_DEBUG_MIFLORA    
    Serial.print("-- Light: ");
	Serial.println(light);
#endif

	int conductivity = val[8] + val[9] * 256;

#if HAP_DEBUG_MIFLORA    
	Serial.print("-- Conductivity: ");
	Serial.println(conductivity);
#endif

	if ((temperature > 200) || (temperature < -100)) {
		LogW(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Unreasonable values received", true);
		return false;
	}

	_deviceData.temperature = temperature;
	_deviceData.moisture = moisture;
	_deviceData.light = light;
	_deviceData.conductivity = conductivity;

	return true;
}


bool HAPPluginNimbleMiFloraDevice::readFloraBatteryCharacteristic(BLERemoteService* floraService) {
	BLERemoteCharacteristic* floraCharacteristic = nullptr;

	// get the device battery characteristic
	LogV(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Read device info from device", true);
	try {
		floraCharacteristic = floraService->getCharacteristic(_uuid_version_battery);
	}
	catch (...) {
		// something went wrong
	}
	if (floraCharacteristic == nullptr) {
		LogD("-- Failed, skipping battery level", true);
		return false;
	}

	// read characteristic value
	//LogD("- Read value from characteristic", true);
	std::string value;

	try{
		value = floraCharacteristic->readValue();
	}
	catch (...) {
		// something went wrong
		LogW(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Failed to read characteristic", true);
		return false;
	}
	
    const char *val2 = value.c_str();

#if HAP_DEBUG_MIFLORA
    HAPHelper::array_print("value", (const unsigned char*)val2, strlen(val2));
#endif


	int battery = val2[0];

#if HAP_DEBUG_MIFLORA
	Serial.print("-- Battery: ");
	Serial.println(battery);
#endif

	_deviceData.battery = battery;


    if (strlen(val2) == 7) {
        _deviceData.firmware[0] = val2[2];
        _deviceData.firmware[1] = val2[3];
        _deviceData.firmware[2] = val2[4];
        _deviceData.firmware[3] = val2[5];
        _deviceData.firmware[4] = val2[6];
        _deviceData.firmware[5] = '\0';

#if HAP_DEBUG_MIFLORA
    	Serial.print("-- Firmware: ");
    	Serial.println(_deviceData.firmware);
#endif
    }

	return true;
}

#if HAP_PLUGIN_MIFLORA_ENABLE_HISTORY 
bool HAPPluginNimbleMiFloraDevice::readFloraDeviceTimeCharacteristic(BLERemoteService* floraService) {
	BLERemoteCharacteristic* floraCharacteristic = nullptr;

	// get the device device time characteristic
	LogD("- Access time characteristic from device", true);
	try {
		floraCharacteristic = floraService->getCharacteristic(_uuid_device_time);
	}
	catch (...) {
		// something went wrong
	}
	if (floraCharacteristic == nullptr) {
		LogD("-- Failed - char not found?, skipping device time", true);
		return false;
	}

	// read characteristic value
	LogD("- Read value from characteristic", true);
	std::string value;
	try{
		value = floraCharacteristic->readValue();
	}
	catch (...) {
		// something went wrong
		LogD("-- Failed - reading value, skipping device time", true);
		return false;
	}
	const char *val = value.c_str();
	uint32_t deviceTime = val[0] | ((uint32_t)val[1] << 8) | ((uint32_t)val[2] << 16) | ((uint32_t)val[3] << 24);

    _deviceData.deviceTime = HAPServer::timestamp() - deviceTime;

#if HAP_DEBUG_MIFLORA
	Serial.print("-- DeviceTime: ");
	Serial.println(_deviceData.deviceTime);	
#endif

	return true;
}

bool HAPPluginNimbleMiFloraDevice::readFloraHistoryEntryCountCharacteristic(BLERemoteService* floraService, uint16_t* entryCount) {
	BLERemoteCharacteristic* floraCharacteristic = nullptr;

	// get the device device time characteristic
	LogD("- Access history entry count characteristic from device", true);
	try {
		floraCharacteristic = floraService->getCharacteristic(_uuid_history_read);
	}
	catch (...) {
		// something went wrong
	}
	if (floraCharacteristic == nullptr) {
		LogE("-- Failed - char not found?, skipping device time", true);
		return false;
	}

	// read characteristic value
	LogD("- Read value from characteristic", true);
	std::string value;
	try{
		value = floraCharacteristic->readValue();
	}
	catch (...) {
		// something went wrong
		LogE("-- Failed - reading value, skipping device time", true);
		return false;
	}
	const char *val = value.c_str();
	*entryCount = val[0] + val[1] * 256;

#if HAP_DEBUG_MIFLORA
	Serial.print("-- Entry Count: ");
	Serial.println(*entryCount);
	// retData->deviceTime = deviceTime;
#endif


	return true;
}


void HAPPluginNimbleMiFloraDevice::calculateEntryAddress(uint8_t *address, const uint16_t entry){
	address[0] = 0xA1;
    address[1] = entry;
    address[2] = entry << 8;
}


bool HAPPluginNimbleMiFloraDevice::readFloraHistoryEntryCharacteristic(BLERemoteService* floraService, struct floraHistory* history) {
	BLERemoteCharacteristic* floraCharacteristic = nullptr;

	// get the device device time characteristic

	
	LogD("- Access history entry characteristic from device", true);
	try {
		floraCharacteristic = floraService->getCharacteristic(_uuid_history_read);
	}
	catch (...) {
		// something went wrong
	}
	if (floraCharacteristic == nullptr) {
		LogE("-- Failed - char not found?, skipping entry count", true);
		return false;
	}

	// read characteristic value
	LogD("- Read value from characteristic", true);
	std::string value;
	try{
		value = floraCharacteristic->readValue();
	}
	catch (...) {
		// something went wrong
		LogE("-- Failed - char not found?, skipping entry count", true);
		return false;
	}
	const char *val = value.c_str();

#if HAP_DEBUG_MIFLORA
    HAPHelper::array_print("value", (const uint8_t*)val, 16);
#endif


	uint32_t timestamp = val[0] | ((uint32_t)val[1] << 8) |
                    ((uint32_t)val[2] << 16) | ((uint32_t)val[3] << 24);	

      
    
    if ( (timestamp == 0) ) {
		LogE("-- Unreasonable values for timestamp received, skip publish", true);
		return false;
	}                     

#if HAP_DEBUG_MIFLORA	
	Serial.print("-- Timestamp: ");
	Serial.print(timestamp);
#endif

	
	timestamp = _deviceData.deviceTime + timestamp;	

#if HAP_DEBUG_MIFLORA	
	Serial.print(" -- after calculation: ");
	Serial.println(timestamp);
#endif


	int16_t temp_raw = val[4] | ((int16_t)val[5] << 8);
	float temperature = temp_raw / ((float)10.0);

#if HAP_DEBUG_MIFLORA
	Serial.print("-- Temperature: ");
	Serial.println(temperature);
#endif

	
	if ((temperature > 200) || (temperature < -50)) {
		LogE("-- Unreasonable values received, skip", true);
		return false;
	}

	uint32_t light = val[7] | ((uint32_t)val[8] << 8) |
                    ((uint32_t)val[9] << 16) | ((uint32_t)val[10] << 24);
#if HAP_DEBUG_MIFLORA					
	Serial.print("-- Light: ");
	Serial.println(light);
#endif

	int moisture = val[11];
#if HAP_DEBUG_MIFLORA	
	Serial.print("-- Moisture: ");
	Serial.println(moisture);
#endif	

	int conductivity = val[12] | ((uint32_t)val[13] << 8);
#if HAP_DEBUG_MIFLORA	
	Serial.print("-- Conductivity: ");
	Serial.println(conductivity);
#endif



	history->timestamp = timestamp;
	history->temperature = temperature;
	history->moisture = moisture;
	history->light = light;
	history->conductivity = conductivity;
	// retData->deviceTime = deviceTime;    

	return true;
}


bool HAPPluginNimbleMiFloraDevice::getEntryCount(BLERemoteService* floraService, uint16_t *entryCount){
    // set device in data mode
	// write the magic data
	uint8_t buf[3] = {0xA0, 0x00, 0x00};
	if (!forceFloraServiceDataMode(floraService, _uuid_write_history_mode, buf, 3)) {
		return false;
	}

	bool entryCountSuccess = readFloraHistoryEntryCountCharacteristic(floraService, entryCount);

#if HAP_DEBUG_MIFLORA
	Serial.print(">>> entryCount: ");
	Serial.println(*entryCount);
#endif

    return entryCountSuccess;
}

bool HAPPluginNimbleMiFloraDevice::processFloraHistoryService(BLERemoteService* floraService, struct floraHistory* history, uint16_t entryCount) {
	
	if (entryCount == 0) {
		history->success = true;
		return history->success;
	}

	bool entrySuccess = false;

    bool errorOccured = false;
	
#if HAP_DEBUG_MIFLORA	
	Serial.println("History Data:");
	Serial.println("=============================================================");
#endif
	
	// uint16_t    counter      = 0;
    uint8_t     errorCounter = 0;

	for (int i = 0; i < entryCount; i++){

        if (!_floraClient->isConnected()){
            LogW("floraClient lost connection - Trying to connect ...", true);            
            _floraClient = getFloraClient(_deviceAddress);
            if (_floraClient == nullptr) {
                LogE("Could not connect to device", true);
                return false;
            }                       

            floraService = getFloraService(_serviceHistoryUUID);
            if (floraService == nullptr) {
                _floraClient->disconnect();
                LogE("Could not connect to history service [" + String(_floraClient->getPeerAddress().toString().c_str()) + "]", true);
                return false;
            }

            uint8_t buf[3] = {0xA0, 0x00, 0x00};
            if (!forceFloraServiceDataMode(floraService, _uuid_write_history_mode, buf, 3)) {
				LogE("Could not force data mode to history service [" + String(_floraClient->getPeerAddress().toString().c_str()) + "]", true);
                return false;
            }	 

			errorCounter = 0;
        }

		uint8_t address[3];
		calculateEntryAddress(&(*address), i);

#if HAP_DEBUG_MIFLORA
		Serial.print("Address: ");
		for (int j = 0; j < 3; j++){
			Serial.printf("%02X", address[j]);			
		}
		Serial.println("");
#endif

		if (!forceFloraServiceDataMode(floraService, _uuid_write_history_mode, address, 3)) {
            errorOccured = true;
            LogE("Could not set read mode for history entry [" + String(_floraClient->getPeerAddress().toString().c_str()) + "]", true);
			break;
		}

        entrySuccess = readFloraHistoryEntryCharacteristic(floraService, &(history[i]));
		if (!entrySuccess){
            errorOccured = true;
            LogE("Could not read history entry [" + String(_floraClient->getPeerAddress().toString().c_str()) + "]", true);
		}

		// Add to fakegato 
		errorOccured = !_fakegato.addEntry(0x06, history->timestamp, String(history->temperature), String(history->moisture), "0");

#if HAP_DEBUG_MIFLORA
		Serial.println("=============================================================");
#endif		

        if (errorOccured){
            i--;
            errorCounter++;
            errorOccured = false;
        }

        if (errorCounter >= 5){
            errorOccured = true;
            break;
        }
       
	}

	history->success = !errorOccured;
	return history->success;
}

#endif

bool HAPPluginNimbleMiFloraDevice::processFloraService(BLERemoteService* floraService) {
	
    bool batterySuccess = readFloraBatteryCharacteristic(floraService);
	

	// set device in data mode
	// write the magic data
	uint8_t buf[2] = {0xA0, 0x1F};
	if (!forceFloraServiceDataMode(floraService, _uuid_write_mode, buf, 2)) {
		return false;
	}

	bool dataSuccess = readFloraDataCharacteristic(floraService);    
	
	_deviceData.success = dataSuccess && batterySuccess;
	return _deviceData.success;
}


bool HAPPluginNimbleMiFloraDevice::processFloraDevice() {

    bool success = true;

    if (shouldHandle()) {
        // connect to flora ble server
        _floraClient = getFloraClient(_deviceAddress);
        if (_floraClient == nullptr) {

            LogE("Could not connect to device [", true);
            return false;
        }


        // connect data service
        BLERemoteService* floraService = getFloraService(_serviceUUID);
        if (floraService == nullptr) {
            LogE("Could not connect to service [" + String(_floraClient->getPeerAddress().toString().c_str()) + "]", true);
            _floraClient->disconnect();
            return false;
        }

        // process devices data
        success = processFloraService(floraService);
        // blink(floraService);


        if (success) {
            setDeviceData(_deviceData);
        }

#if HAP_PLUGIN_MIFLORA_ENABLE_HISTORY

        if (_hasFetchedHistory == false) {

            BLERemoteService* floraHistoryService = getFloraService(_serviceHistoryUUID);
            if (floraHistoryService == nullptr) {
                _floraClient->disconnect();
                LogE("Could not connect to history service [" + String(_floraClient->getPeerAddress().toString().c_str()) + "]", true);
                return false;
            }

            bool deviceTimeSuccess = readFloraDeviceTimeCharacteristic(floraHistoryService);
            if (!deviceTimeSuccess) {
                LogE("Could not process device time [" + String(_floraClient->getPeerAddress().toString().c_str()) + "]", true);
            }

            uint16_t entryCount = 0;
            getEntryCount(floraHistoryService, &entryCount);	
            struct floraHistory* history = (struct floraHistory*)malloc(entryCount * sizeof(struct floraHistory));
            

            bool successHistory = processFloraHistoryService(floraHistoryService, history, entryCount);	
            if (!successHistory) {
                LogE("Processing history failed for device [" + String(_floraClient->getPeerAddress().toString().c_str()) + "]", true);
            }
            

            success = success && successHistory && deviceTimeSuccess;
            
            _hasFetchedHistory = true;

            free(history);

        }


#endif

        // disconnect from device
        _floraClient->disconnect();

    }
	
	return success;
}


bool HAPPluginNimbleMiFloraDevice::blinkLED(){

    _floraClient = getFloraClient(_deviceAddress);
    // connect data service
	BLERemoteService* floraService = getFloraService(_serviceUUID);
	if (floraService == nullptr) {
        LogE("Could not connect to service [" + String(_floraClient->getPeerAddress().toString().c_str()) + "]", true);
		_floraClient->disconnect();
		return false;
	}

	// set device in data mode
	// write the magic data
	uint8_t buf[2] = {0xFD, 0xFF};
	if (!forceFloraServiceDataMode(floraService, _uuid_write_mode, buf, 2)) {
        LogE("Could not blink LED for device [" + String(_floraClient->getPeerAddress().toString().c_str()) + "]", true);
		return false;
	}

    _floraClient->disconnect();

    return true;
}


