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
    // bool     setRefTime;    

    uint16_t temperature;
    uint16_t humidity;
    uint16_t pressure;
};


class HAPFakeGatoWeather: public HAPFakeGato  {
public:
    HAPFakeGatoWeather(); 
    ~HAPFakeGatoWeather();    

    void begin();

    size_t size() {
        return _memoryUsed;
    }

    bool isFull() {
        return _memoryUsed == HAP_FAKEGATO_BUFFER_SIZE;
    }

    void clear(){
        _vectorBuffer->clear();
    }

    int signatureLength();
    void getSignature(uint8_t* signature);

    

    bool addEntry(String stringTemperature, String stringHumidity, String stringPressure = "0");    
    bool addEntry(HAPFakeGatoWeatherData data);
    void getData(const size_t count, uint8_t *data, size_t *length, uint16_t offset);

private:
    std::vector<HAPFakeGatoWeatherData>* _vectorBuffer;    
};

#endif /* HAPFAKEGATOWEATHER_HPP_ */