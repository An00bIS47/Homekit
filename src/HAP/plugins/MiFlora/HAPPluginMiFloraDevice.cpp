//
// HAPPluginMifloraDevice.cpp
// Homekit
//
//  Created on: 22.09.2019
//      Author: michael
//
#include "HAPPluginMifloraDevice.hpp"
#include "HAPPluginMiFlora.hpp"
#include "HAPLogger.hpp"
#include "HAPServer.hpp"

#define VERSION_MAJOR       1
#define VERSION_MINOR       0
#define VERSION_REVISION    3
#define VERSION_BUILD       2

HAPPluginMiFloraDevice::HAPPluginMiFloraDevice(std::string address){
    
    _version.major      = VERSION_MAJOR;
    _version.minor      = VERSION_MINOR;
    _version.revision   = VERSION_REVISION;
    _version.build      = VERSION_BUILD;

    
    _deviceAddress = address;
}

HAPPluginMiFloraDevice::HAPPluginMiFloraDevice(BLEAddress address){
    
    _version.major      = VERSION_MAJOR;
    _version.minor      = VERSION_MINOR;
    _version.revision   = VERSION_REVISION;
    _version.build      = VERSION_BUILD;

    
    _deviceAddress = address.toString();
}


bool HAPPluginMiFloraDevice::begin(){

    return true;
}

std::string HAPPluginMiFloraDevice::address(){
    return _deviceAddress;
}



HAPAccessory* HAPPluginMiFloraDevice::initAccessory(){
    
    String sn = md5(HAPDeviceID::deviceID() + String(_deviceAddress.c_str()));
    
    _accessory = new HAPAccessory();
    //HAPAccessory::addInfoServiceToAccessory(_accessory, "Builtin LED", "ACME", "LED", "123123123", &identify);
    auto callbackIdentify = std::bind(&HAPPluginMiFloraDevice::identify, this, std::placeholders::_1, std::placeholders::_2);
    _accessory->addInfoService("MiFlora", "Xioami", "Flower Care", sn, callbackIdentify, version());

    //
    // Battery service
    // 
    HAPService* batteryService = new HAPService(HAP_SERVICE_BATTERY_SERVICE);
    _accessory->addService(batteryService);


    // stringCharacteristics *batteryServiceName = new stringCharacteristics(charType_serviceName, permission_read, 0);
    // batteryServiceName->setValue("Battery");
    // _accessory->addCharacteristics(batteryService, batteryServiceName);

    // stringCharacteristics *batteryStatusServiceName = new stringCharacteristics(charType_serviceName, permission_read, 0);
    // batteryStatusServiceName->setValue("Battery Status");
    // _accessory->addCharacteristics(batteryService, batteryStatusServiceName);

    // 
    // Battery level
    // 
    _batteryLevel = new intCharacteristics(HAP_CHARACTERISTIC_BATTERY_LEVEL, permission_read|permission_notify, 0, 100, 1, unit_percentage);
    _batteryLevel->setValue("0");
    auto callbackChangeBatLevel = std::bind(&HAPPluginMiFloraDevice::changeBatteryLevel, this, std::placeholders::_1, std::placeholders::_2);
    _batteryLevel->valueChangeFunctionCall = callbackChangeBatLevel;
    _accessory->addCharacteristics(batteryService, _batteryLevel);


    // 
    // Battery status
    // 
    _batteryStatus = new intCharacteristics(HAP_CHARACTERISTIC_STATUS_LOW_BATTERY, permission_read|permission_notify, 0, 1, 1, unit_none);
    _batteryStatus->setValue("0");
    auto callbackChangeBatStatus = std::bind(&HAPPluginMiFloraDevice::changeBatteryStatus, this, std::placeholders::_1, std::placeholders::_2);
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
    
    auto callbackChangeTemp = std::bind(&HAPPluginMiFloraDevice::changeTemp, this, std::placeholders::_1, std::placeholders::_2);
    _temperatureValue->valueChangeFunctionCall = callbackChangeTemp;
    _accessory->addCharacteristics(temperatureService, _temperatureValue);

    // 
    // Last Update characteristics  (Custom)
    // is bound to temperature service
    // 
    _lastUpdate = new stringCharacteristics("000003EA-6B66-4FFD-88CC-16A60B5C4E03", permission_read|permission_notify, 32);
    _lastUpdate->setDescription("LastUpdate");
    _lastUpdate->setValue("Sun Dec 29 2019 04:30:53");

    auto callbackChangeLastUpdate = std::bind(&HAPPluginMiFloraDevice::changeLastUpdate, this, std::placeholders::_1, std::placeholders::_2);
    _lastUpdate->valueChangeFunctionCall = callbackChangeLastUpdate;
    _accessory->addCharacteristics(temperatureService, _lastUpdate);



    //
    // Humidity service
    //
    HAPService* humidityService = new HAPService(HAP_SERVICE_HUMIDITY_SENSOR);
    _accessory->addService(humidityService);

    stringCharacteristics *humServiceName = new stringCharacteristics(HAP_CHARACTERISTIC_NAME, permission_read, 32);
    humServiceName->setValue("Moisture Sensor");
    _accessory->addCharacteristics(humidityService, humServiceName);


    //
    // Humidity value
    //  
    _humidityValue = new floatCharacteristics(HAP_CHARACTERISTIC_CURRENT_RELATIVE_HUMIDITY, permission_read|permission_notify, 0, 100, 0.1, unit_percentage);
    _humidityValue->setValue("0.0");

    auto callbackChangeHum = std::bind(&HAPPluginMiFloraDevice::changeHum, this, std::placeholders::_1, std::placeholders::_2);
    _humidityValue->valueChangeFunctionCall = callbackChangeHum;
    _accessory->addCharacteristics(humidityService, _humidityValue);

    //
    // AmbientLight == light
    //
    HAPService* lightService = new HAPService(HAP_SERVICE_LIGHT_SENSOR);
    _accessory->addService(lightService);

    stringCharacteristics *lightServiceName = new stringCharacteristics(HAP_CHARACTERISTIC_NAME, permission_read, 32);
    lightServiceName->setValue("Light Sensor");
    _accessory->addCharacteristics(lightService, lightServiceName);

    _lightValue = new floatCharacteristics(HAP_CHARACTERISTIC_CURRENT_AMBIENT_LIGHT_LEVEL, permission_read|permission_notify, 0.0, 100000, 0.1, unit_lux);
    _lightValue->setValue("0.0");

    auto callbackChangeLight = std::bind(&HAPPluginMiFloraDevice::changeLight, this, std::placeholders::_1, std::placeholders::_2);
    _lightValue->valueChangeFunctionCall = callbackChangeLight;
    _accessory->addCharacteristics(lightService, _lightValue);

    //
    // Fertility Sensor (Custom)
    //
    HAPService* fertilityService = new HAPService("00000001-6B66-4FFD-88CC-16A60B5C4E03");
    _accessory->addService(fertilityService);

    stringCharacteristics *fertilityServiceName = new stringCharacteristics(HAP_CHARACTERISTIC_NAME, permission_read, 32);
    fertilityServiceName->setValue("Fertility Sensor");    
    _accessory->addCharacteristics(fertilityService, fertilityServiceName);

    _fertilityValue = new floatCharacteristics("000003E8-6B66-4FFD-88CC-16A60B5C4E03", permission_read|permission_notify, 0.0, 10000, 0.1, unit_none);
    _fertilityValue->setValue("0.0");
    _fertilityValue->setDescription("Fertility");


    auto callbackChangeFertility = std::bind(&HAPPluginMiFloraDevice::changeFertility, this, std::placeholders::_1, std::placeholders::_2);
    _fertilityValue->valueChangeFunctionCall = callbackChangeFertility;
    _accessory->addCharacteristics(fertilityService, _fertilityValue);



    // 
    // FakeGato History
    // 
    // Get the last 3 bytes of the mac address
    String deviceAdd = _deviceAddress.c_str();
    deviceAdd.replace(":", "");
    deviceAdd = deviceAdd.substring(6);

    _fakegato.registerFakeGatoService(_accessory, "MiFlora " + deviceAdd);
	auto callbackAddEntry = std::bind(&HAPPluginMiFloraDevice::fakeGatoCallback, this);
	// _fakegato.registerCallback(callbackAddEntry);
	_fakegatoFactory->registerFakeGato(&_fakegato,  "MiFlora " + deviceAdd, callbackAddEntry);


    return _accessory;
}


void HAPPluginMiFloraDevice::setValue(floraData data){
    LogD(HAPServer::timeString() + " " + "MiFloraDevice" + "->" + String(__FUNCTION__) + " [   ] " + "Set values ", true);

    {
        _temperatureValue->setValue(String(data.temperature));
        struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _temperatureValue->iid, String(data.temperature));							
        _eventManager->queueEvent( EventManager::kEventNotifyController, event);
    }

    {
        _humidityValue->setValue(String(data.moisture));
        struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _humidityValue->iid, String(data.moisture));
        _eventManager->queueEvent( EventManager::kEventNotifyController, event);
    }

    {
        _lightValue->setValue(String(data.light));
        struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _lightValue->iid, String(data.light));
        _eventManager->queueEvent( EventManager::kEventNotifyController, event);
    }
    
    {
        _fertilityValue->setValue(String(data.conductivity));
        struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _fertilityValue->iid, String(data.conductivity));
        _eventManager->queueEvent( EventManager::kEventNotifyController, event);        
    }
    
    {
        _batteryLevel->setValue(String(data.battery));
        struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _batteryLevel->iid, String(data.battery));
        _eventManager->queueEvent( EventManager::kEventNotifyController, event);        
    }
    
    {
        _batteryStatus->setValue(data.battery < 15 ? "1" : "0" );
        struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _batteryStatus->iid, data.battery < 15 ? "1" : "0");
        _eventManager->queueEvent( EventManager::kEventNotifyController, event);        
    }

    {
        _lastUpdate->setValue(HAPServer::timeString());
        struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _lastUpdate->iid, _lastUpdate->value());
        _eventManager->queueEvent( EventManager::kEventNotifyController, event);   
    }
    
    _accessory->setFirmware(String(data.firmware));

    // ToDo: Add Events
}


void HAPPluginMiFloraDevice::setEventManager(EventManager* eventManager){
    _eventManager = eventManager;
}


void HAPPluginMiFloraDevice::setFakeGatoFactory(HAPFakeGatoFactory* fakegatoFactory){
    _fakegatoFactory = fakegatoFactory;
}   


void HAPPluginMiFloraDevice::changeTemp(float oldValue, float newValue) {
	Serial.printf("[MiFlora:%s] New temperature: %f\n", _deviceAddress.c_str(), newValue);
}

void HAPPluginMiFloraDevice::changeHum(float oldValue, float newValue) {
	Serial.printf("[MiFlora:%s] New humidity: %f\n", _deviceAddress.c_str(), newValue);
}

void HAPPluginMiFloraDevice::changeLight(float oldValue, float newValue) {
	Serial.printf("[MiFlora:%s] New light: %f\n", _deviceAddress.c_str(), newValue);
}

void HAPPluginMiFloraDevice::changeBatteryLevel(float oldValue, float newValue) {
	Serial.printf("[MiFlora:%s] New battery Level: %f\n", _deviceAddress.c_str(), newValue);
}

void HAPPluginMiFloraDevice::changeBatteryStatus(float oldValue, float newValue) {
	Serial.printf("[MiFlora:%s] New battery status: %f\n", _deviceAddress.c_str(), newValue);
}

void HAPPluginMiFloraDevice::changeFertility(float oldValue, float newValue) {
	Serial.printf("[MiFlora:%s] New fertility: %f\n", _deviceAddress.c_str(), newValue);
}

void HAPPluginMiFloraDevice::changeLastUpdate(String oldValue, String newValue){
    Serial.printf("[MiFlora:%s] New LastUpdate: %s\n", _deviceAddress.c_str(), newValue.c_str());
}

void HAPPluginMiFloraDevice::identify( bool oldValue, bool newValue) {
    printf("Start Identify MiFlora with address %s\n", _deviceAddress.c_str());
}


bool HAPPluginMiFloraDevice::fakeGatoCallback(){
    // LogD(HAPServer::timeString() + " " + "HAPPluginPCA301Device" + "->" + String(__FUNCTION__) + " [   ] " + "fakeGatoCallback()", true);

    // Serial.println("power: " + _curPowerValue->value());    
    return _fakegato.addEntry(_temperatureValue->value(), _humidityValue->value(), "0");
}