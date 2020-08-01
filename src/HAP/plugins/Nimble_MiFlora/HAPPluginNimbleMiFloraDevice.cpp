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
#include <NimBLEDevice.h>

#define VERSION_MAJOR       1
#define VERSION_MINOR       0
#define VERSION_REVISION    3
#define VERSION_BUILD       3

HAPPluginNimbleMiFloraDevice::HAPPluginNimbleMiFloraDevice(const std::string& address) : _deviceAddress(address){
    
    _version.major      = VERSION_MAJOR;
    _version.minor      = VERSION_MINOR;
    _version.revision   = VERSION_REVISION;
    _version.build      = VERSION_BUILD;

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
}

HAPPluginNimbleMiFloraDevice::HAPPluginNimbleMiFloraDevice(BLEAddress address) : _deviceAddress(address.toString()){
    
    _version.major      = VERSION_MAJOR;
    _version.minor      = VERSION_MINOR;
    _version.revision   = VERSION_REVISION;
    _version.build      = VERSION_BUILD;

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
}


bool HAPPluginNimbleMiFloraDevice::begin(){

    return true;
}

std::string HAPPluginNimbleMiFloraDevice::address(){
    return _deviceAddress;
}



HAPAccessory* HAPPluginNimbleMiFloraDevice::initAccessory(){
    
    String sn = md5(HAPDeviceID::deviceID() + String(_deviceAddress.c_str()));
    
    _accessory = new HAPAccessory();
    //HAPAccessory::addInfoServiceToAccessory(_accessory, "Builtin LED", "ACME", "LED", "123123123", &identify);
    auto callbackIdentify = std::bind(&HAPPluginNimbleMiFloraDevice::identify, this, std::placeholders::_1, std::placeholders::_2);
    _accessory->addInfoService("MiFlora", "Xioami", "Flower Care", sn, callbackIdentify, version());

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
    _lastUpdate = new stringCharacteristics(HAP_CUSTOM_CHARACTERISTICS_LAST_UPDATE, permission_read|permission_notify, 32);
    _lastUpdate->setDescription("LastUpdate");
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


void HAPPluginNimbleMiFloraDevice::setValue(floraData data){
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

    // ToDo: Add Events
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
}


bool HAPPluginNimbleMiFloraDevice::fakeGatoCallback(){
    // LogD(HAPServer::timeString() + " " + "HAPPluginPCA301Device" + "->" + String(__FUNCTION__) + " [   ] " + "fakeGatoCallback()", true);

    // Serial.println("power: " + _curPowerValue->value());    
    return _fakegato.addEntry(_temperatureValue->value(), _humidityValue->value(), "0");
}


void HAPPluginNimbleMiFloraDevice::changeHeartbeat(uint8_t oldValue, uint8_t newValue){
    Serial.printf("[MiFlora:%s] New Heartbeat: %d\n", _deviceAddress.c_str(), newValue);

    if (oldValue != newValue){

        unsigned long newInterval = (newValue * 60) * 1000;
        _callbackSetInterval(newInterval);
    }

}