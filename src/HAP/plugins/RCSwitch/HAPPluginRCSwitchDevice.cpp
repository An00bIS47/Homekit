//
// HAPPluginRCSwitchDevice.cpp
// Homekit
//
//  Created on: 14.09.2019
//      Author: michael
//

#include "HAPPluginRCSwitchDevice.hpp"
#include "HAPServer.hpp"
#include "HAPLogger.hpp"

HAPPluginRCSwitchDevice::HAPPluginRCSwitchDevice(){   
    name    = "";    
    houseAddress = 0;
    deviceAddress = 0;

    _accessory          = nullptr;
    _eventManager       = nullptr;  
    _fakegatoFactory    = nullptr;

    _stateValue         = nullptr;
    _inUseState         = nullptr;
    _parentalLock       = nullptr;
    _curPowerValue      = nullptr;
    _ttlPowerValue      = nullptr;

    _timestampLastActivity = 0;
}

HAPPluginRCSwitchDevice::HAPPluginRCSwitchDevice(uint8_t houseAddress_, uint8_t deviceAddress_, String name_)
: houseAddress(houseAddress_)
, deviceAddress(deviceAddress_)
, name(name_)
{
    _accessory          = nullptr;
    _eventManager       = nullptr;      
    _fakegatoFactory    = nullptr;

    _stateValue         = nullptr;
    _inUseState         = nullptr;
    _parentalLock       = nullptr;
    _curPowerValue      = nullptr;
    _ttlPowerValue      = nullptr;
}


HAPAccessory* HAPPluginRCSwitchDevice::initAccessory(){

    String sn = HAPDeviceID::serialNumber("RC", String(houseAddress) + String(deviceAddress));    

    // Create accessory if not already created
    _accessory = new HAPAccessory();
    //HAPAccessory::addInfoServiceToAccessory(_accessory, "Builtin LED", "ACME", "LED", "123123123", &identify);
    auto callbackIdentify = std::bind(&HAPPluginRCSwitchDevice::identify, this, std::placeholders::_1, std::placeholders::_2);
    _accessory->addInfoService(name, "ACME", "RCSwitch", sn, callbackIdentify, "1.0");

    // // 
    // // Outlet Service / Switch Service
    // // !!! Fakegato history is only working with switch service !!!
    // // 
    // HAPService* outletService = new HAPService(HAP_SERVICE_SWITCH);
    // _accessory->addService(outletService);

    // stringCharacteristics *plugServiceName = new stringCharacteristics(HAP_CHARACTERISTIC_NAME, permission_read, HAP_STRING_LENGTH_MAX);
    // plugServiceName->setValue(name);
    // _accessory->addCharacteristics(outletService, plugServiceName);

    // //
    // // Power State 
    // // 
    // _stateValue = new boolCharacteristics(HAP_CHARACTERISTIC_ON, permission_read|permission_write|permission_notify);            
    // _stateValue->setValue("0");

    // auto callbackState = std::bind(&HAPPluginRCSwitchDevice::changedState, this, std::placeholders::_1, std::placeholders::_2);        
    // _stateValue->valueChangeFunctionCall = callbackState;
    // _accessory->addCharacteristics(outletService, _stateValue);

  
    // // 
    // // FakeGato History
    // // 
    // _fakegato.registerFakeGatoService(_accessory, "RCSwitch " + String(houseAddress) + String(deviceAddress));
	// auto callbackAddEntry = std::bind(&HAPPluginRCSwitchDevice::fakeGatoCallback, this);
	// _fakegatoFactory->registerFakeGato(&_fakegato,  "RCSwitch " + String(houseAddress) + String(deviceAddress), callbackAddEntry);

    // 
    // Outlet Service
    // 
    HAPService* outletService = new HAPService(HAP_SERVICE_OUTLET);
    _accessory->addService(outletService);

    stringCharacteristics *plugServiceName = new stringCharacteristics(HAP_CHARACTERISTIC_NAME, permission_read, 32);
    plugServiceName->setValue("RCSwitch " + String(houseAddress) + String(deviceAddress));
    _accessory->addCharacteristics(outletService, plugServiceName);

    //
    // Power State 
    // 
    _stateValue = new boolCharacteristics(HAP_CHARACTERISTIC_ON, permission_read|permission_write|permission_notify);            
    _stateValue->setValue("0");

    auto callbackState = std::bind(&HAPPluginRCSwitchDevice::changedState, this, std::placeholders::_1, std::placeholders::_2);        
    _stateValue->valueChangeFunctionCall = callbackState;
    _accessory->addCharacteristics(outletService, _stateValue);


    //
    // in use State
    //
    _inUseState = new boolCharacteristics(HAP_CHARACTERISTIC_OUTLET_IN_USE, permission_read|permission_notify);        
    // auto callbackState = std::bind(&HAPPluginRCSwitchDevice::setValue, this, std::placeholders::_1, std::placeholders::_2);        
    // _inUseState->valueChangeFunctionCall = callbackState;
    _inUseState->setValue("1");
    _accessory->addCharacteristics(outletService, _inUseState);

    //
    // power current (EVE)
    //
    _curPowerValue = new floatCharacteristics(HAP_CHARACTERISTIC_FAKEGATO_ELECTRIC_CURRENT, permission_read|permission_notify, 0.0, 3600, 0.1, unit_none);
    _curPowerValue->setValue("0.0");
    
    auto callbackChangeCurPower = std::bind(&HAPPluginRCSwitchDevice::changedPowerCurrent, this, std::placeholders::_1, std::placeholders::_2);
    _curPowerValue->valueChangeFunctionCall = callbackChangeCurPower;
    _accessory->addCharacteristics(outletService, _curPowerValue);
    

    //
    // power total (EVE)
    //
    _ttlPowerValue = new floatCharacteristics(HAP_CHARACTERISTIC_FAKEGATO_TOTAL_CONSUMPTION, permission_read|permission_notify, 0.0, 3600, 0.1, unit_none);
    _ttlPowerValue->setValue("0.0");
    
    auto callbackChangeTtlPower = std::bind(&HAPPluginRCSwitchDevice::changedPowerTotal, this, std::placeholders::_1, std::placeholders::_2);
    _ttlPowerValue->valueChangeFunctionCall = callbackChangeTtlPower;
    _accessory->addCharacteristics(outletService, _ttlPowerValue);


    //
    // parental Lock
    //
    _parentalLock = new boolCharacteristics(HAP_CHARACTERISTIC_LOCK_PHYSICAL_CONTROLS, permission_read|permission_write);        
    _parentalLock->setValue("0");    
    // auto callbackChangeTtlPower = std::bind(&HAPPluginRCSwitchDevice::changedPowerTotal, this, std::placeholders::_1, std::placeholders::_2);
    // _ttlPowerValue->valueChangeFunctionCall = callbackChangeTtlPower;
    _accessory->addCharacteristics(outletService, _parentalLock);


    
    // 
    // FakeGato History
    // 
    _fakegato.registerFakeGatoService(_accessory, "RCSwitch " + String(houseAddress) + String(deviceAddress), true);
    
    // Fakegato Schedule
    _fakegato.setSerialNumber(sn);        
    _fakegato.setCallbackTimerStart(std::bind(&HAPPluginRCSwitchDevice::switchCallback, this, std::placeholders::_1));
    // _fakegato.setCallbackTimerEnd(std::bind(&HAPPluginRCSwitchDevice::switchCallback, this));
    _fakegato.setCallbackGetTimestampLastActivity(std::bind(&HAPPluginRCSwitchDevice::getTimestampLastActivity, this));
    
    _fakegato.setCallbackSaveConfig(std::bind(&HAPPluginRCSwitchDevice::saveConfig, this));

    _fakegato.beginSchedule();

	auto callbackAddEntry = std::bind(&HAPPluginRCSwitchDevice::fakeGatoCallback, this);
	_fakegatoFactory->registerFakeGato(&_fakegato,  "RCSwitch " + String(houseAddress) + String(deviceAddress), callbackAddEntry);


    return _accessory;
}

void HAPPluginRCSwitchDevice::setEventManager(EventManager* eventManager){
    _eventManager = eventManager;
}


void HAPPluginRCSwitchDevice::setFakeGatoFactory(HAPFakeGatoFactory* fakegatoFactory){
    _fakegatoFactory = fakegatoFactory;
}   


void HAPPluginRCSwitchDevice::identify(bool oldValue, bool newValue) {
    printf("Start Identify rcswitch:%d-%d\n", houseAddress, deviceAddress);
}


void HAPPluginRCSwitchDevice::changedPowerCurrent(float oldValue, float newValue){
    
    if (oldValue != newValue) {
        struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _curPowerValue->iid, String(newValue));							
        _eventManager->queueEvent( EventManager::kEventNotifyController, event);


        String inUse;
        newValue > 0.1 ? inUse = "1" : inUse = "0";    
        if (_inUseState->value() != inUse){
            _inUseState->setValue(inUse);

            struct HAPEvent eventInUse = HAPEvent(nullptr, _accessory->aid, _inUseState->iid, String(newValue));							
            _eventManager->queueEvent( EventManager::kEventNotifyController, eventInUse);
        }
    }
}

void HAPPluginRCSwitchDevice::changedPowerTotal(float oldValue, float newValue){
    if (oldValue != newValue) {
        struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _ttlPowerValue->iid, String(newValue));							
	    _eventManager->queueEvent( EventManager::kEventNotifyController, event); 
    }
}


void HAPPluginRCSwitchDevice::changedState(bool oldValue, bool newValue){

    LogD(HAPServer::timeString() + " " + "HAPPluginRCSwitchDevice" + "->" + String(__FUNCTION__) + " [   ] " + "Setting new value to " + String(newValue), true);

    if (oldValue != newValue) {
        _callbackRCSwitchSend(houseAddress, deviceAddress, newValue);
        
        _timestampLastActivity = HAPServer::timestamp();

        // Add entry to fakegato
        _fakegato.addEntry(0x01, "0", "0", "0", "0", newValue == true ? "1" : "0");

        struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _stateValue->iid, newValue ? "1" : "0");							
        _eventManager->queueEvent( EventManager::kEventNotifyController, event);        
    }    
}

bool HAPPluginRCSwitchDevice::fakeGatoCallback(){
    // LogD(HAPServer::timeString() + " " + "HAPPluginPCA301Device" + "->" + String(__FUNCTION__) + " [   ] " + "fakeGatoCallback()", true);

    // Serial.println("power: " + _curPowerValue->value());    
    return _fakegato.addEntry(0x1F, "0", "0", "0", "0", _stateValue->value());
}

void HAPPluginRCSwitchDevice::switchCallback(uint16_t state){
    LogD(HAPServer::timeString() + " " + "HAPPluginRCSwitchDevice" + "->" + String(__FUNCTION__) + " [   ] " + "Callback to switch " + String(state == 1 ? "ON" : "OFF"), true);
    // _callbackRCSwitchSend(houseAddress, deviceAddress, state);
    _stateValue->setValue(state == 1 ? "1" : "0");
}

uint32_t HAPPluginRCSwitchDevice::getTimestampLastActivity(){
    return _timestampLastActivity;
}

JsonObject HAPPluginRCSwitchDevice::scheduleToJson(){
    return _fakegato.scheduleToJson();
}

void HAPPluginRCSwitchDevice::scheduleFromJson(JsonObject &root){
    _fakegato.scheduleFromJson(root);
}


void HAPPluginRCSwitchDevice::saveConfig(){ 
    LogE(HAPServer::timeString() + " " + "HAPPluginRCSwitchDevice" + "->" + String(__FUNCTION__) + " [   ] " + "Update config event", true);		
    _eventManager->queueEvent( EventManager::kEventUpdatedConfig, HAPEvent());
}
