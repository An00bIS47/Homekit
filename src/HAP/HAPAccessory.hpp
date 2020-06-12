//
// HAPAccessory.hpp
// Homekit
//
//  Created on: 22.04.2018
//      Author: michael
//

#ifndef HAPACCESSORY_HPP_
#define HAPACCESSORY_HPP_

#include <Arduino.h>
#include <vector>

#include "HAPService.hpp"
#include "HAPCharacteristic.hpp"


// typedef void (*identifyFunction)(bool oldValue, bool newValue);
typedef std::function<void(bool, bool)> identifyFunctionCallback;

class HAPAccessory {
public:

    int aid;    
    uint8_t numberOfInstance = 0;
    
    std::vector<HAPService *>_services;

    void addService(HAPService *ser);   
    void addCharacteristics(HAPService *ser, characteristics *cha);
    bool removeService(HAPService *ser);
    bool removeCharacteristics(characteristics *cha);

    HAPAccessory();

    uint8_t numberOfService() const;
    HAPService *serviceAtIndex(uint8_t index);

    characteristics *characteristicsAtIndex(uint8_t index);
    characteristics *characteristicsOfType(int type);

    String describe() const;

    HAPService* addInfoService(String accessoryName, String manufactuerName, String modelName, String serialNumber, identifyFunctionCallback callback, String firmwareRev = "");
    
    void setName(String name);
    String name();
    
    void setFirmware(String firmware);
    String firmware();

    void setIdentifyCallback(identifyFunctionCallback callback);


    String serialNumber();
    void setSerialNumber(String serialNumber);


    String modelName();
    void setModelName(String serialNumber);


    String manufacturer();
    void setManufacturer(String serialNumber);

private:

    void initInfoService();
    void initAccessoryName();
    void initFirmware();
    void initSerialNumber();
    void initIdentify();
    void initModelName();
    void initManufacturer();

    // String _accessoryName;

    HAPService*             _infoService;

    stringCharacteristics*  _accessoryName;
    stringCharacteristics*  _firmware;
    stringCharacteristics*  _manufacturer;
    stringCharacteristics*  _modelName;
    stringCharacteristics*  _serialNumber;
    boolCharacteristics*    _identify;
};


#endif /* HAPACCESSORY_HPP_ */