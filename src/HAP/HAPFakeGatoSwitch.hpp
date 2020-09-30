// 
// HAPFakeGatoSwitch.hpp
// Homekit
//
//  Created on: 21.12.2019
//      Author: michael
//

#ifndef HAPFAKEGATOSWITCH_HPP_
#define HAPFAKEGATOSWITCH_HPP_

#include <Arduino.h>
#include "HAPFakeGato.hpp"


struct HAPFakeGatoSwitchData {
    uint32_t timestamp;     // unix
    // bool     setRefTime;    

    // uint16_t unknown1;
    // uint16_t unknown2;
    bool status;
    // uint16_t unknown3;
    // uint16_t unknown4;
};


class HAPFakeGatoSwitch : public HAPFakeGato  {
public:

    HAPFakeGatoSwitch(); 
    ~HAPFakeGatoSwitch();    

    void begin() override;

    int signatureLength() override;
    void getSignature(uint8_t* signature) override;

    
    size_t size() override {
        return _memoryUsed;
    }

    bool isFull() override {
        return _memoryUsed == HAP_FAKEGATO_BUFFER_SIZE;
    }


    void clear() override{
        _vectorBuffer->clear();
    }

   
    bool addEntry(String stringPower);    
    bool addEntry(HAPFakeGatoSwitchData data);
    void getData(const size_t count, uint8_t *data, size_t *length, uint16_t offset) override;

private:    
    std::vector<HAPFakeGatoSwitchData>* _vectorBuffer;    
};

#endif /* HAPFAKEGATOSWITCH_HPP_ */