// 
// HAPFakeGatoWeather.hpp
// Homekit
//
//  Created on: 29.08.2019
//      Author: michael
//

#ifndef HAPFAKEGATOWEATHER_HPP_
#define HAPFAKEGATOWEATHER_HPP_

#include <Arduino.h>
#include "HAPFakeGato.hpp"

struct HAPFakeGatoWeatherData {    
    uint32_t timestamp;     // unix
    uint8_t  bitmask;
    // bool     setRefTime;    

    uint16_t temperature;
    uint16_t humidity;
    uint16_t pressure;
};


class HAPFakeGatoWeather: public HAPFakeGato  {
public:
    HAPFakeGatoWeather(); 
    ~HAPFakeGatoWeather();    

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

    
    bool addEntry(uint8_t bitmask, uint32_t timestamp, String stringTemperature, String stringHumidity, String stringPressure);
    bool addEntry(uint8_t bitmask, String stringTemperature, String stringHumidity, String stringPressure);    
    bool addEntry(HAPFakeGatoWeatherData data);
    void getData(const size_t count, uint8_t *data, size_t *length, uint16_t offset) override;

private:
    std::vector<HAPFakeGatoWeatherData>* _vectorBuffer;    
};

#endif /* HAPFAKEGATOWEATHER_HPP_ */