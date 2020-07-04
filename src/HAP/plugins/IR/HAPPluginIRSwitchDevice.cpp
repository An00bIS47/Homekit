//
// HAPPluginIRSwitchDevice.cpp
// Homekit
//
//  Created on: 14.09.2019
//      Author: michael
//

#include "HAPPluginIRSwitchDevice.hpp"
#include "HAPServer.hpp"
#include "HAPLogger.hpp"

HAPPluginIRSwitchDevice::HAPPluginIRSwitchDevice(){   
    name    = "";    
    houseAddress = 0;
    deviceAddress = 0;

    _accessory          = nullptr;
    _eventManager       = nullptr;  
    _fakegatoFactory    = nullptr;

    _stateValue         = nullptr;
}

HAPPluginIRSwitchDevice::HAPPluginIRSwitchDevice(uint8_t houseAddress_, uint8_t deviceAddress_, String name_)
: houseAddress(houseAddress_)
, deviceAddress(deviceAddress_)
, name(name_)
{
    _accessory          = nullptr;
    _eventManager       = nullptr;      
    _fakegatoFactory    = nullptr;

    _stateValue         = nullptr;
}


HAPAccessory* HAPPluginIRSwitchDevice::initAccessory(){

    String sn = md5(HAPDeviceID::deviceID() + name + String(houseAddress) + String(deviceAddress));

    // Create accessory if not already created
    _accessory = new HAPAccessory();
    //HAPAccessory::addInfoServiceToAccessory(_accessory, "Builtin LED", "ACME", "LED", "123123123", &identify);
    auto callbackIdentify = std::bind(&HAPPluginIRSwitchDevice::identify, this, std::placeholders::_1, std::placeholders::_2);
    _accessory->addInfoService(name, "ACME", "RCSwitch", sn, callbackIdentify, "1.0");

    // 
    // Outlet Service / Switch Service
    // !!! Fakegato history is only with switch service !!!
    // 
    HAPService* outletService = new HAPService(HAP_SERVICE_SWITCH);
    _accessory->addService(outletService);

    stringCharacteristics *plugServiceName = new stringCharacteristics(HAP_CHARACTERISTIC_NAME, permission_read, 32);
    plugServiceName->setValue(name);
    _accessory->addCharacteristics(outletService, plugServiceName);

    //
    // Power State 
    // 
    _stateValue = new boolCharacteristics(HAP_CHARACTERISTIC_ON, permission_read|permission_write|permission_notify);            
    _stateValue->setValue("0");

    auto callbackState = std::bind(&HAPPluginIRSwitchDevice::changedState, this, std::placeholders::_1, std::placeholders::_2);        
    _stateValue->valueChangeFunctionCall = callbackState;
    _accessory->addCharacteristics(outletService, _stateValue);

  
    // 
    // FakeGato History
    // 
    _fakegato.registerFakeGatoService(_accessory, "RCSwitch " + String(houseAddress) + String(deviceAddress));
	auto callbackAddEntry = std::bind(&HAPPluginIRSwitchDevice::fakeGatoCallback, this);
	_fakegatoFactory->registerFakeGato(&_fakegato,  "RCSwitch " + String(houseAddress) + String(deviceAddress), callbackAddEntry);

    return _accessory;
}

void HAPPluginIRSwitchDevice::setEventManager(EventManager* eventManager){
    _eventManager = eventManager;
}


void HAPPluginIRSwitchDevice::setFakeGatoFactory(HAPFakeGatoFactory* fakegatoFactory){
    _fakegatoFactory = fakegatoFactory;
}   


void HAPPluginIRSwitchDevice::identify(bool oldValue, bool newValue) {
    printf("Start Identify rcswitch:%d-%d\n", houseAddress, deviceAddress);
}


void HAPPluginIRSwitchDevice::setState(String pwrState){
    if (_stateValue) 
        _stateValue->setValue(pwrState);
}


void HAPPluginIRSwitchDevice::changedState(bool oldValue, bool newValue){

    LogD(HAPServer::timeString() + " " + "HAPPluginIRSwitchDevice" + "->" + String(__FUNCTION__) + " [   ] " + "Setting new value to " + String(newValue), true);

    if (oldValue != newValue) {
        _callbackRCSwitchSend(houseAddress, deviceAddress, newValue);
        
        // Add status change entry to fakegato
        _fakegato.addEntry(newValue ? "1" : "0");

        struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _stateValue->iid, newValue ? "1" : "0");							
        _eventManager->queueEvent( EventManager::kEventNotifyController, event);        
    }    
}

bool HAPPluginIRSwitchDevice::fakeGatoCallback(){
    // LogD(HAPServer::timeString() + " " + "HAPPluginPCA301Device" + "->" + String(__FUNCTION__) + " [   ] " + "fakeGatoCallback()", true);

    // Serial.println("power: " + _curPowerValue->value());    
    return _fakegato.addEntry(_stateValue->value());
}