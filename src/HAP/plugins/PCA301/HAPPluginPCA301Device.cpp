//
// HAPPluginPCA301Device.cpp
// Homekit
//
//  Created on: 14.09.2019
//      Author: michael
//

#include "HAPPluginPCA301Device.hpp"
#include "HAPServer.hpp"
#include "HAPLogger.hpp"

HAPPluginPCA301Device::HAPPluginPCA301Device(){
    channel = 1;    
    devId   = 0;      
    pState  = 0;      
    pNow    = 0;         
    pTtl    = 0;      
    nextTX  = 0;          
    retries = 0;         
    name    = "";    

    _accessory          = nullptr;
    _eventManager       = nullptr;  
    _fakegatoFactory    = nullptr;
}

HAPPluginPCA301Device::HAPPluginPCA301Device(uint8_t channel_, uint32_t devId_, bool pState_, String name_)
: channel(channel_)
, devId(devId_)
, pState(pState_)
, name(name_)
{
    nextTX  = 0;
    retries = 0;
    pNow    = 0;
    pTtl    = 0;

    _accessory          = nullptr;
    _eventManager       = nullptr;      
    _fakegatoFactory    = nullptr;
}


HAPAccessory* HAPPluginPCA301Device::initAccessory(){

    String sn = HAPDeviceID::serialNumber("PCA", String(devId));    

    // Create accessory if not already created
    _accessory = new HAPAccessory();
    //HAPAccessory::addInfoServiceToAccessory(_accessory, "Builtin LED", "ACME", "LED", "123123123", &identify);
    auto callbackIdentify = std::bind(&HAPPluginPCA301Device::identify, this, std::placeholders::_1, std::placeholders::_2);
    _accessory->addInfoService("PCA301 " + String(devId), "ACME", "PCA301", sn, callbackIdentify, "1.0");

    // 
    // Outlet Service
    // 
    HAPService* outletService = new HAPService(HAP_SERVICE_OUTLET);
    _accessory->addService(outletService);

    stringCharacteristics *plugServiceName = new stringCharacteristics(HAP_CHARACTERISTIC_NAME, permission_read, 32);
    plugServiceName->setValue("PCA301 " + String(devId));
    _accessory->addCharacteristics(outletService, plugServiceName);

    //
    // Power State 
    // 
    _powerState = new boolCharacteristics(HAP_CHARACTERISTIC_ON, permission_read|permission_write|permission_notify);            
    _powerState->setValue("0");

    auto callbackState = std::bind(&HAPPluginPCA301Device::changedPowerState, this, std::placeholders::_1, std::placeholders::_2);        
    _powerState->valueChangeFunctionCall = callbackState;
    _accessory->addCharacteristics(outletService, _powerState);


    //
    // in use State
    //
    _inUseState = new boolCharacteristics(HAP_CHARACTERISTIC_OUTLET_IN_USE, permission_read|permission_notify);        
    // auto callbackState = std::bind(&HAPPluginPCA301::setValue, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);        
    _inUseState->valueChangeFunctionCall = callbackState;
    _inUseState->setValue("0");
    _accessory->addCharacteristics(outletService, _inUseState);

    //
    // power current (EVE)
    //
    _curPowerValue = new floatCharacteristics(HAP_CHARACTERISTIC_FAKEGATO_ELECTRIC_CURRENT, permission_read|permission_notify, 0.0, 3600, 0.1, unit_none);
    _curPowerValue->setValue("0.0");
    
    auto callbackChangeCurPower = std::bind(&HAPPluginPCA301Device::changedPowerCurrent, this, std::placeholders::_1, std::placeholders::_2);
    _curPowerValue->valueChangeFunctionCall = callbackChangeCurPower;
    _accessory->addCharacteristics(outletService, _curPowerValue);
    

    //
    // power total (EVE)
    //
    _ttlPowerValue = new floatCharacteristics(HAP_CHARACTERISTIC_FAKEGATO_TOTAL_CONSUMPTION, permission_read|permission_notify, 0.0, 3600, 0.1, unit_none);
    _ttlPowerValue->setValue("0.0");
    
    auto callbackChangeTtlPower = std::bind(&HAPPluginPCA301Device::changedPowerTotal, this, std::placeholders::_1, std::placeholders::_2);
    _ttlPowerValue->valueChangeFunctionCall = callbackChangeTtlPower;
    _accessory->addCharacteristics(outletService, _ttlPowerValue);

    
    // 
    // FakeGato History
    // 
    _fakegato.registerFakeGatoService(_accessory, "PCA301 " + String(devId));
	auto callbackAddEntry = std::bind(&HAPPluginPCA301Device::fakeGatoCallback, this);
	// _fakegato.registerCallback(callbackAddEntry);
	_fakegatoFactory->registerFakeGato(&_fakegato,  "PCA301 " + String(devId), callbackAddEntry);

    return _accessory;
}

void HAPPluginPCA301Device::setEventManager(EventManager* eventManager){
    _eventManager = eventManager;
}


void HAPPluginPCA301Device::setFakeGatoFactory(HAPFakeGatoFactory* fakegatoFactory){
    _fakegatoFactory = fakegatoFactory;
}   


void HAPPluginPCA301Device::identify(bool oldValue, bool newValue) {
    printf("Start Identify pca301\n");
}


void HAPPluginPCA301Device::setPowerState(String pwrState){
    if (_powerState) 
        _powerState->setValue(pwrState);
}

void HAPPluginPCA301Device::setCurrentPower(String pwrCur){
    if (_curPowerValue)
        _curPowerValue->setValue(pwrCur);
}

void HAPPluginPCA301Device::setTotalPower(String pwrTtl){
    if (_ttlPowerValue)
        _ttlPowerValue->setValue(pwrTtl);
}



void HAPPluginPCA301Device::changedPowerState(bool oldValue, bool newValue){

    LogD(HAPServer::timeString() + " " + "HAPPluginPCA301Device" + "->" + String(__FUNCTION__) + " [   ] " + "Setting new value to " + String(newValue), true);

    if (oldValue != newValue) {
        _callbackPCA301Send(devId, newValue == true ? 'e' : 'd');
        
        struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _powerState->iid, newValue ? "1" : "0");							
        _eventManager->queueEvent( EventManager::kEventNotifyController, event);
    }

    // ToDo: implement send to device ...
}


void HAPPluginPCA301Device::changedPowerCurrent(float oldValue, float newValue){
    
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

void HAPPluginPCA301Device::changedPowerTotal(float oldValue, float newValue){
    if (oldValue != newValue) {
        struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _ttlPowerValue->iid, String(newValue));							
	    _eventManager->queueEvent( EventManager::kEventNotifyController, event); 
    }
}

bool HAPPluginPCA301Device::fakeGatoCallback(){
    // LogD(HAPServer::timeString() + " " + "HAPPluginPCA301Device" + "->" + String(__FUNCTION__) + " [   ] " + "fakeGatoCallback()", true);

    // Serial.println("power: " + _curPowerValue->value());    
    return _fakegato.addEntry(0x02, "0", "0", "0", _curPowerValue->value(), "0");
}