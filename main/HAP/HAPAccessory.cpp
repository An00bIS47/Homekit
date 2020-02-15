
#include "HAPAccessory.hpp"
#include "HAPHelper.hpp"
#include "HAPServer.hpp"
#include "EventManager.h"


#include "HAPCharacteristics.hpp"
#include "HAPServices.hpp"

HAPAccessory::HAPAccessory() {
	aid = 0;
    
    _infoService    = nullptr;
    _accessoryName  = nullptr;
    _manufacturer   = nullptr;
    _modelName      = nullptr; 
    _serialNumber   = nullptr;
    _identify       = nullptr;
    _firmware       = nullptr;
}

void HAPAccessory::addService(HAPService *ser) {
	ser->serviceID = ++numberOfInstance;
	_services.push_back(ser);
}

void HAPAccessory::addCharacteristics(HAPService *ser, characteristics *cha) {
	cha->iid = ++numberOfInstance;
	ser->_characteristics.push_back(cha);

    // ToDo: Add event for update mdns config
    
    struct HAPEvent event = HAPEvent(NULL, aid, cha->iid, "");					
	HAPServer::_eventManager.queueEvent( EventManager::kEventIncrementConfigNumber, event);
}

bool HAPAccessory::removeService(HAPService *ser) {
	bool exist = false;
	for (std::vector<HAPService *>::iterator it = _services.begin(); it != _services.end(); it++) {
		if (*it == ser) {
			_services.erase(it);
			exist = true;
		}
	}
	return exist;
}

bool HAPAccessory::removeCharacteristics(characteristics *cha) {
	bool exist = false;
	for (std::vector<HAPService *>::iterator it = _services.begin(); it != _services.end(); it++) {
		for (std::vector<characteristics *>::iterator jt = (*it)->_characteristics.begin(); jt != (*it)->_characteristics.end(); jt++) {
			if (*jt == cha) {
				(*it)->_characteristics.erase(jt);
				exist = true;

                // ToDo: Add event for update mdns config                                                     
                struct HAPEvent event = HAPEvent(NULL, aid, (*jt)->iid, "");					
	            HAPServer::_eventManager.queueEvent( EventManager::kEventIncrementConfigNumber, event);
			}
		}
	}
	return exist;
}



uint8_t HAPAccessory::numberOfService() const { 
	return _services.size(); 
}


HAPService *HAPAccessory::serviceAtIndex(uint8_t index) {
	return _services[index];
}

characteristics *HAPAccessory::characteristicsAtIndex(uint8_t index) {
	for (std::vector<HAPService *>::iterator it = _services.begin(); it != _services.end(); it++) {
		for (std::vector<characteristics *>::iterator jt = (*it)->_characteristics.begin(); jt != (*it)->_characteristics.end(); jt++) {
			if ((*jt)->iid == index) {
				return *jt;
			}
		}
	}
	return NULL;
}

characteristics *HAPAccessory::characteristicsOfType(int type) {
	for (std::vector<HAPService *>::iterator it = _services.begin(); it != _services.end(); it++) {
		for (std::vector<characteristics *>::iterator jt = (*it)->_characteristics.begin(); jt != (*it)->_characteristics.end(); jt++) {
			if ((*jt)->type == type) {
				return *jt;
			}
		}
	}
	return NULL;
}



String HAPAccessory::describe() const {

    String keys[2];
    String values[2];
    
    {
        keys[0] = "aid";
        char temp[8];
        sprintf(temp, "%d", aid);
        values[0] = temp;
    }
    
    {
        //Form services list
        int noOfService = numberOfService();
        String *services = new String[noOfService];
        for (int i = 0; i < noOfService; i++) {
            services[i] = _services[i]->describe();
        }
        keys[1] = "services";
        values[1] = HAPHelper::arrayWrap(services, noOfService);
        delete [] services;
    }
    
    
    return HAPHelper::dictionaryWrap(keys, values, 2);
}


void HAPAccessory::addInfoService(String accessoryName, String manufactuerName, String modelName, String serialNumber, identifyFunctionCallback callback, String firmwareRev){
    
    setName(accessoryName);
    
    setIdentifyCallback(callback);

    setManufacturer(manufactuerName);

    setModelName(modelName);

    setSerialNumber(serialNumber);
    
    if ( firmwareRev != "" ) {
        setFirmware(firmwareRev);
    }
        
}


void HAPAccessory::initInfoService(){
    if (_infoService == nullptr) {
        _infoService = new HAPService(HAP_SERVICE_ACCESSORY_INFORMATION);
        addService(_infoService);    
    }    
}

void HAPAccessory::initAccessoryName(){
    initInfoService();
    if (_accessoryName == nullptr) {
        _accessoryName = new stringCharacteristics(HAP_CHARACTERISTIC_NAME, permission_read, 32);            
        addCharacteristics(_infoService, _accessoryName);
    }
}

void HAPAccessory::initFirmware(){
    initInfoService();
    if (_firmware == nullptr) {
        _firmware = new stringCharacteristics(HAP_CHARACTERISTIC_FIRMWARE_REVISION, permission_read, 32);            
        addCharacteristics(_infoService, _firmware);
    }
}

void HAPAccessory::initManufacturer(){
    initInfoService();
    if (_manufacturer == nullptr) {
        _manufacturer = new stringCharacteristics(HAP_CHARACTERISTIC_MANUFACTURER, permission_read, 32);            
        addCharacteristics(_infoService, _manufacturer);
    }
}

void HAPAccessory::initModelName(){
    initInfoService();
    if (_modelName == nullptr) {
        _modelName = new stringCharacteristics(HAP_CHARACTERISTIC_MODEL, permission_read, 32);            
        addCharacteristics(_infoService, _modelName);
    }
}

void HAPAccessory::initSerialNumber(){
    initInfoService();
    if (_serialNumber == nullptr) {
        _serialNumber = new stringCharacteristics(HAP_CHARACTERISTIC_SERIAL_NUMBER, permission_read, 32);            
        addCharacteristics(_infoService, _serialNumber);
    }
}

void HAPAccessory::initIdentify(){
    initInfoService();
    if (_identify == nullptr) {
        _identify = new boolCharacteristics(HAP_CHARACTERISTIC_IDENTIFY, permission_write);            
        addCharacteristics(_infoService, _identify);
    }
}


void HAPAccessory::setName(String name){
    initAccessoryName();
	_accessoryName->setValue(name);
}

String HAPAccessory::name(){
    if (_accessoryName == nullptr) {
        return "";    
    }
	return _accessoryName->value();
}

void HAPAccessory::setFirmware(String firmware){
    initFirmware();
	_firmware->setValue(firmware);
}

String HAPAccessory::firmware(){
    if (_firmware == nullptr) {
        return "";    
    }
	return _firmware->value();
}


void HAPAccessory::setManufacturer(String manufacturer){
    initManufacturer();
	_manufacturer->setValue(manufacturer);
}

String HAPAccessory::manufacturer(){
    if (_manufacturer == nullptr) {
        return "";    
    }
	return _manufacturer->value();
}


void HAPAccessory::setModelName(String modelName){
    initModelName();
	_modelName->setValue(modelName);
}

String HAPAccessory::modelName(){
    if (_modelName == nullptr) {
        return "";    
    }
	return _modelName->value();
}

void HAPAccessory::setSerialNumber(String serialNumber){
    initSerialNumber();
	_serialNumber->setValue(serialNumber);
}

String HAPAccessory::serialNumber(){
    if (_serialNumber == nullptr) {
        return "";    
    }
	return _serialNumber->value();
}

void HAPAccessory::setIdentifyCallback(identifyFunctionCallback callback){
    initIdentify();
    _identify->valueChangeFunctionCall = callback;
}