// 
// HAPFakeGatoHygrometer.hpp
// Homekit
//
//  Created on: 29.08.2019
//      Author: michael
//

#ifndef HAPFAKEGATOHYGROMETER_HPP_
#define HAPFAKEGATOHYGROMETER_HPP_

#include <Arduino.h>
#include "HAPFakeGato.hpp"

struct HAPFakeGatoHygrometerData {    
    uint32_t timestamp;     // unix
    uint8_t  bitmask;
    uint16_t humidity;
};


class HAPFakeGatoHygrometer: public HAPFakeGato  {
public:
    HAPFakeGatoHygrometer(); 
    ~HAPFakeGatoHygrometer();    

    void begin() override;

    size_t size() override {
        return _memoryUsed;
    }

    bool isFull() override {
        return _memoryUsed == HAP_FAKEGATO_BUFFER_SIZE;
    }

    void clear() override {
        _vectorBuffer->clear();
    }

    int signatureLength() override;
    void getSignature(uint8_t* signature) override;

    
    bool addEntry(uint32_t timestamp, String stringHumidity);
    bool addEntry(String stringHumidity);    
    bool addEntry(HAPFakeGatoHygrometerData data);
    void getData(const size_t count, uint8_t *data, size_t *length, uint16_t offset) override;

private:
    std::vector<HAPFakeGatoHygrometerData>* _vectorBuffer;    
};

#endif /* HAPFAKEGATOHYGROMETER_HPP_ */