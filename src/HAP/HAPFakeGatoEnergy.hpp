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
    uint8_t  bitmask;
    uint16_t powerWatt;
    uint16_t powerVoltage;
    uint16_t powerCurrent;    
    uint16_t power10th;
    uint8_t  status;    
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
    
    bool addEntry(uint8_t bitmask, String powerWatt, String powerVoltage, String powerCurrent, String stringPower10th, String status);    
    bool addEntry(HAPFakeGatoEnergyData data);
    void getData(const size_t count, uint8_t *data, size_t *length, uint16_t offset) override;

    void beginSchedule() override;

    inline void setCallbackTimerStart(std::function<void(uint16_t)> callback){
        // _callbackTimerStart = callback;
        _schedule->setCallbackTimerStart(callback);
    }

    inline void setCallbackTimerEnd(std::function<void(uint16_t)> callback){
        // _callbackTimerEnd = callback;
        _schedule->setCallbackTimerEnd(callback);
    }

    void handle(bool forced = false) override;

    inline void setCallbackGetTimestampLastActivity(std::function<uint32_t(void)> callback){
        _schedule->setCallbackGetTimestampLastActivity(callback);
    }

    JsonObject scheduleToJson();
    void scheduleFromJson(JsonObject &root);

    
    inline void setCallbackSaveConfig(std::function<void(void)> callback){
        _callbackSaveConfig = callback;
    }


protected:
    // Schedules
    void scheduleRead(String oldValue, String newValue) override;
    void scheduleWrite(String oldValue, String newValue) override;

private:
    bool _shouldSave;

    std::vector<HAPFakeGatoEnergyData>* _vectorBuffer; 
    HAPFakeGatoScheduleEnergy* _schedule;

    std::function<void(void)> _callbackSaveConfig;    
};

#endif /* HAPFAKEGATOENERGY_HPP_ */