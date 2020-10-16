// 
// HAPFakeGatoScheduleEnergy.hpp
// Homekit
//
//  Created on: 23.09.2020
//      Author: michael
//

#ifndef HAPFAKEGATOSCHEDULEENERGY_HPP_
#define HAPFAKEGATOSCHEDULEENERGY_HPP_

#include <Arduino.h>
#include <ArduinoJson.h>
#include "HAPGlobals.hpp"
#include "HAPAccessory.hpp"

#include "HAPHelper.hpp"
#include "HAPTLV8.hpp"
#include <vector>

#include "HAPDailyTimerFactory.hpp"

// ToDo: define or enum ?
#define HAP_FAKEGATO_SCHEDULE_TYPE_SERIALNUMBER             0x04

#define HAP_FAKEGATO_SCHEDULE_TYPE_USED_MEMORY              0x06
#define HAP_FAKEGATO_SCHEDULE_TYPE_ROLLED_OVER_INDEX        0x07

#define HAP_FAKEGATO_SCHEDULE_TYPE_COMMAND_TOGGLE_SCHEDULE  0x44
#define HAP_FAKEGATO_SCHEDULE_TYPE_COMMAND_STATUS_LED       0x20
#define HAP_FAKEGATO_SCHEDULE_TYPE_PROGRAMS                 0x45
#define HAP_FAKEGATO_SCHEDULE_TYPE_DAYS                     0x46
#define HAP_FAKEGATO_SCHEDULE_TYPE_STATUS_LED               0x60

typedef enum {
    TIME = 0,
    SUN  = 1
} HAPFakeGatoScheduleTimerType;


typedef enum {  
    SUNSET  = 0,
    SUNRISE = 1
} HAPFakeGatoScheduleSunriseType;


struct HAPFakeGatoScheduleTimerEvent {
    HAPFakeGatoScheduleTimerType   type;

    uint8_t     hour;
    uint8_t     minute;
    int32_t     offset;

    bool        state;
    HAPFakeGatoScheduleSunriseType sunrise;    
};


struct HAPFakeGatoScheduleProgramEvent {
    uint8_t id;
    std::vector<HAPFakeGatoScheduleTimerEvent> timerEvents; // 15
};


struct HAPFakeGatoScheduleDays {

    uint8_t mon;
    uint8_t tue;
    uint8_t wed;
    uint8_t thu;
    uint8_t fri;
    uint8_t sat;
    uint8_t sun;

    HAPFakeGatoScheduleDays(){
        mon = 0;
        tue = 0;
        wed = 0;
        thu = 0;
        fri = 0;
        sat = 0;
        sun = 0;
    }

    // decode
    HAPFakeGatoScheduleDays(uint32_t daysnumber) {
        mon = daysnumber & 0x07;
        tue = ( daysnumber >>  3 ) & 0x07;
        wed = ( daysnumber >>  6 ) & 0x07;
        thu = ( daysnumber >>  9 ) & 0x07;
        fri = ( daysnumber >> 12 ) & 0x07;
        sat = ( daysnumber >> 15 ) & 0x07;
        sun = ( daysnumber >> 18 ) & 0x07;  
    }

    // encode
    uint32_t daysnumber(){
        uint32_t d = sun << 18 | sat << 15 | fri << 12 | thu << 9 | wed << 6 | tue << 3 | mon;
        d = d << 4;
		d = d + 0x0F;   // 15
		return d;
    }
};



// template <class TFakeGatoData>
class HAPFakeGatoScheduleEnergy {
public:    

    HAPFakeGatoScheduleEnergy();
    HAPFakeGatoScheduleEnergy(String serialNumber, std::function<void(uint16_t)> callbackStartTimer, std::function<void(uint16_t)> callbackEndTimer, std::function<uint32_t(void)> callbackRefTime, std::function<uint32_t(void)> callbackTimestampLastActivity, std::function<uint32_t(void)> callbackTimestampLastEntry);
    ~HAPFakeGatoScheduleEnergy();

    void begin();
   
    bool decodeToggleOnOff(uint8_t* data);
    void decodeDays(uint8_t *data);
    void decodePrograms(uint8_t* data);

    void encodePrograms(uint8_t* data, size_t *dataSize);

    bool isEnabled();
    void enable(bool on);

    inline void setSerialNumber(String serialNumber){
        _serialNumber = serialNumber;
    }

    void setStatusLED(uint8_t mode);

    static uint32_t encodeTimerCount(uint8_t timerCount);
    static uint8_t encodeProgramCount(uint8_t programCount);    

    void fromJson(JsonObject &root);
    JsonObject toJson();

    String buildScheduleString();

    void clear();

    inline void setCallbackTimerStart(std::function<void(uint16_t)> callback){
        _callbackTimerStart = callback;
    }

    inline void setCallbackTimerEnd(std::function<void(uint16_t)> callback){
        _callbackTimerEnd = callback;
    }

    inline void handle() {        
        // _alarms.delay(0);
        _timers.handle();
    }


    void callbackTimerStart(uint16_t state);
    void callbackTimerEnd(uint16_t state);

    inline void setCallbackGetReftime(std::function<uint32_t(void)> callback){
        _callbackGetRefTime = callback;
    }
    
    inline void setCallbackGetTimestampLastActivity(std::function<uint32_t(void)> callback){
        _callbackGetTimestampLastActivity = callback;
    }

    inline void setCallbackGetTimestampLastEntry(std::function<uint32_t(void)> callback){
        _callbackGetTimestampLastEntry = callback;
    }

    inline void setCallbackGetMemoryUsed(std::function<uint16_t(void)> callback){
        _callbackGetMemoryUsed = callback;
    }

    inline void setCallbackGetRolledOverIndex(std::function<uint32_t(void)> callback){
        _callbackGetRolledOverIndex = callback;
    }



protected:
    std::vector<HAPFakeGatoScheduleProgramEvent> _programEvents; // 7
    HAPFakeGatoScheduleDays _days;  
    HAPDailyTimerFactory _timers;

    // bool _isActive;  
    String _serialNumber;
    uint8_t _statusLED;

    void programTimers();
    
    std::function<void(uint16_t)> _callbackTimerStart;
    std::function<void(uint16_t)> _callbackTimerEnd;

    std::function<uint32_t(void)> _callbackGetRefTime;
    std::function<uint32_t(void)> _callbackGetTimestampLastActivity;
    std::function<uint32_t(void)> _callbackGetTimestampLastEntry;
    
    std::function<uint16_t(void)> _callbackGetMemoryUsed;
    std::function<uint32_t(void)> _callbackGetRolledOverIndex;    
};

#endif /* HAPFAKEGATOSCHEDULEENERGY_HPP_ */