//
// HAPService.hpp
// Homekit
//
//  Created on: 22.04.2018
//      Author: michael
//

#ifndef HAPSERVICE_HPP_
#define HAPSERVICE_HPP_

#include <Arduino.h>
#include <vector>

#include "HAPCharacteristic.hpp"

class HAPService {
public:

    HAPService();
    HAPService(uint8_t _uuid);
    HAPService(String _uuid);

    String describe();

    uint8_t serviceID;
    uint8_t uuid;
    String uuidString;
    
    bool hidden;
    bool primary;

    std::vector<characteristics *> _characteristics;
    std::vector<uint8_t> _linkedServiceIds;

    inline void setHiddenService(bool mode = true){
        hidden = mode;
    }

    inline void setPrimaryService(bool mode = true){
        primary = mode;
    }

    inline void addLinkedServiceId(uint8_t serviceId_){
        _linkedServiceIds.push_back(serviceId_);
    }

    virtual uint8_t numberOfCharacteristics() { return _characteristics.size(); }
    virtual characteristics *characteristicsAtIndex(uint8_t index) { return _characteristics[index]; }
    
};


#endif /* HAPSERVICE_HPP_ */