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
#include "HAPFakeGatoScheduleEnergy.hpp"


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

    
    inline size_t size() override {
        return _memoryUsed;
    }

    inline bool isFull() override {
        return _memoryUsed == HAP_FAKEGATO_BUFFER_SIZE;
    }

    inline void clear() override{
        _vectorBuffer->clear();
    }

    void setSerialNumber(String serialNumber);
    
    bool addEntry(String stringPower);    
    bool addEntry(HAPFakeGatoEnergyData data);
    void getData(const size_t count, uint8_t *data, size_t *length, uint16_t offset) override;

    void initSchedule() override;

    virtual inline void setCallbackTimerStart(std::function<void(uint16_t)> callback){
        // _callbackTimerStart = callback;
        _schedule->setCallbackTimerStart(callback);
    }

    virtual inline void setCallbackTimerEnd(std::function<void(uint16_t)> callback){
        // _callbackTimerEnd = callback;
        _schedule->setCallbackTimerEnd(callback);
    }

    void handle(bool forced = false) override;

protected:
    // Schedules
    void scheduleRead(String oldValue, String newValue) override;
    void scheduleWrite(String oldValue, String newValue) override;

    
private:
    
    std::vector<HAPFakeGatoEnergyData>* _vectorBuffer; 
    HAPFakeGatoScheduleEnergy* _schedule;

    // std::function<void(uint16_t)> _callbackTimerEnd;
    // std::function<void(uint16_t)> _callbackTimerStart;
};

#endif /* HAPFAKEGATOENERGY_HPP_ */