// 
// HAPFakeGatoEnergy.hpp
// Homekit
//
//  Created on: 29.08.2019
//      Author: michael
//

#ifndef HAPFAKEGATOENERGY_HPP_
#define HAPFAKEGATOENERGY_HPP_

#include <Arduino.h>
#include "HAPFakeGato.hpp"



struct HAPFakeGatoEnergyData {
    uint32_t timestamp;     // unix
    // bool     setRefTime;    

    // uint16_t unknown1;
    // uint16_t unknown2;
    uint16_t power;
    // uint16_t unknown3;
    // uint16_t unknown4;
};


class HAPFakeGatoEnergy : public HAPFakeGato  {
public:

    HAPFakeGatoEnergy(); 
    ~HAPFakeGatoEnergy();    

    void begin() override;

    int signatureLength() override;
    void getSignature(uint8_t* signature) override;

    
    size_t size() override {
        return _memoryUsed;
    }

    bool isFull() override {
        return _memoryUsed == HAP_FAKEGATO_BUFFER_SIZE;
    }

    void clear() override {
        _vectorBuffer->clear();
    }

    
    bool addEntry(String stringPower);    
    bool addEntry(HAPFakeGatoEnergyData data);
    void getData(const size_t count, uint8_t *data, size_t *length, uint16_t offset) override;

private:
    
    std::vector<HAPFakeGatoEnergyData>* _vectorBuffer;    
};

#endif /* HAPFAKEGATOENERGY_HPP_ */