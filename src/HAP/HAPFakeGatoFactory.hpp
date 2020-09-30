// 
// HAPFakeGatoFactory.hpp
// Homekit
//
//  Created on: 29.08.2019
//      Author: michael
//
#ifndef HAPFAKEGATOFACTORY_HPP_
#define HAPFAKEGATOFACTORY_HPP_

#include <Arduino.h>
#include <vector>
#include <memory>

#include "HAPFakeGato.hpp"

class HAPFakeGatoFactory {
public:
    HAPFakeGatoFactory() : _refTime(0) {}

    void setRefTime(uint32_t refTime);

    void handle(bool forced = false);
    void registerFakeGato(HAPFakeGato* fakegato, String name, std::function<bool()> callback, uint32_t interval = HAP_FAKEGATO_INTERVAL);

private:    
    std::vector<HAPFakeGato*> _fakegatos;    
    uint32_t                  _refTime;    
};

#endif /* HAPFAKEGATOFACTORY_HPP_ */