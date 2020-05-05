// 
// HAPFakeGato.hpp
// Homekit
//
//  Created on: 29.08.2019
//      Author: michael
//

#ifndef HAPFAKEGATO_HPP_
#define HAPFAKEGATO_HPP_

#include <Arduino.h>
#include "HAPGlobals.hpp"
#include "HAPAccessory.hpp"

#include "HAPHelper.hpp"
#include <vector>


#if ESP_IDF_VERSION_MAJOR == 4

#else
extern "C" {
    #include "crypto/base64.h"
}
#endif


#define FAKEGATO_EPOCH_OFFSET           978307200


#ifndef HAP_FAKEGATO_BUFFER_SIZE
#define HAP_FAKEGATO_BUFFER_SIZE	1536    // Number of history entries for each characteristic 
#endif										// default: 768


#ifndef HAP_FAKEGATO_INTERVAL
#define HAP_FAKEGATO_INTERVAL       300000	// Interval to add entry to history in millis
#endif                                      // EVE app requires at least one entry every 10 mins
											// default: 300000

#ifndef HAP_FAKEGATO_CHUNK_SIZE
#define HAP_FAKEGATO_CHUNK_SIZE     16      // Number of entries sent at once from device to EVE app
#endif										// default: 16

// ToDo: Adjust for the other entries
// #define HAP_FAKEGATO_CHUNK_BUFFER_SIZE  HAP_FAKEGATO_CHUNK_SIZE * 16     // 256 => 16 is the length of the weather info
#define HAP_FAKEGATO_CHUNK_BUFFER_SIZE      512     // base64 256 bits = 344


// this.accessoryType117
#define HAP_FAKEGATO_TYPE_REFTIME       0x81
#define HAP_FAKEGATO_TYPE_WEATHER       0x07    // --> not unique
#define HAP_FAKEGATO_TYPE_ENERGY        0x1F    // --> not unique
#define HAP_FAKEGATO_TYPE_ENERGY_18     0x1E
#define HAP_FAKEGATO_TYPE_ROOM          0x0F
#define HAP_FAKEGATO_TYPE_MOTION        0x02
#define HAP_FAKEGATO_TYPE_DOOR          0x01
#define HAP_FAKEGATO_TYPE_THERMO        0x1F    // --> not unique
#define HAP_FAKEGATO_TYPE_AQUA          0x05    // Entry for Eve Aqua, valve on, 13 bytes in total
#define HAP_FAKEGATO_TYPE_AQUA_21       0x07    // --> not unique
#define HAP_FAKEGATO_TYPE_SWITCH        0x01

// 
// Services
// 
#define HAP_SERVICE_FAKEGATO_WEATHER                            "E863F001-079E-48FF-8F27-9C2605A29F52"  // 001
#define HAP_SERVICE_FAKEGATO_HISTORY                            "E863F007-079E-48FF-8F27-9C2605A29F52"  // 007
#define HAP_SERVICE_FAKEGATO_AIR_PRESSURE_SENSOR                "E863F00A-079E-48FF-8F27-9C2605A29F52"  // 00A

// 
// Characteristics
// 
#define HAP_CHARACTERISTIC_FAKEGATO_VOLTAGE                     "E863F10A-079E-48FF-8F27-9C2605A29F52"  // 10A
#define HAP_CHARACTERISTIC_FAKEGATO_AIR_PARTICULATE_DENSITY     "E863F10B-079E-48FF-8F27-9C2605A29F52"  // 10B
#define HAP_CHARACTERISTIC_FAKEGATO_TOTAL_CONSUMPTION           "E863F10C-079E-48FF-8F27-9C2605A29F52"  // 10C
#define HAP_CHARACTERISTIC_FAKEGATO_CURRENT_CONSUMPTION         "E863F10D-079E-48FF-8F27-9C2605A29F52"  // 10D
#define HAP_CHARACTERISTIC_FAKEGATO_AIR_PRESSURE                "E863F10F-079E-48FF-8F27-9C2605A29F52"  // 10F
#define HAP_CHARACTERISTIC_FAKEGATO_RESET_TOTAL                 "E863F112-079E-48FF-8F27-9C2605A29F52"  // 112
#define HAP_CHARACTERISTIC_FAKEGATO_HISTORY_STATUS              "E863F116-079E-48FF-8F27-9C2605A29F52"  // 116  => S2R1
#define HAP_CHARACTERISTIC_FAKEGATO_HISTORY_ENTRIES             "E863F117-079E-48FF-8F27-9C2605A29F52"  // 117  => S2R2
#define HAP_CHARACTERISTIC_FAKEGATO_OPEN_DURATION               "E863F118-079E-48FF-8F27-9C2605A29F52"  // 118
#define HAP_CHARACTERISTIC_FAKEGATO_CLOSED_DURATION             "E863F119-079E-48FF-8F27-9C2605A29F52"  // 119
#define HAP_CHARACTERISTIC_FAKEGATO_LAST_ACTIVATION             "E863F11A-079E-48FF-8F27-9C2605A29F52"  // 11A
#define HAP_CHARACTERISTIC_FAKEGATO_HISTORY_REQUEST             "E863F11C-079E-48FF-8F27-9C2605A29F52"  // 11C  => S2W1
#define HAP_CHARACTERISTIC_FAKEGATO_SENSITIVITY                 "E863F120-079E-48FF-8F27-9C2605A29F52"  // 120
#define HAP_CHARACTERISTIC_FAKEGATO_SET_TIME                    "E863F121-079E-48FF-8F27-9C2605A29F52"  // 121  => S2W2
#define HAP_CHARACTERISTIC_FAKEGATO_ELECTRIC_CURRENT            "E863F126-079E-48FF-8F27-9C2605A29F52"  // 126
#define HAP_CHARACTERISTIC_FAKEGATO_kVAh                        "E863F127-079E-48FF-8F27-9C2605A29F52"  // 127
#define HAP_CHARACTERISTIC_FAKEGATO_TIMES_OPENED                "E863F129-079E-48FF-8F27-9C2605A29F52"  // 129
#define HAP_CHARACTERISTIC_FAKEGATO_PROGRAM_COMMAND             "E863F12C-079E-48FF-8F27-9C2605A29F52"  // 12C
#define HAP_CHARACTERISTIC_FAKEGATO_DURATION                    "E863F12D-079E-48FF-8F27-9C2605A29F52"  // 12D
#define HAP_CHARACTERISTIC_FAKEGATO_VALVE_POSITION              "E863F12E-079E-48FF-8F27-9C2605A29F52"  // 12E
#define HAP_CHARACTERISTIC_FAKEGATO_PROGRAM_DATA                "E863F12E-079E-48FF-8F27-9C2605A29F52"  // 12E
#define HAP_CHARACTERISTIC_FAKEGATO_ELEVATION                   "E863F130-079E-48FF-8F27-9C2605A29F52"  // 130



enum HAPFakeGatoType{
    HAPFakeGatoType_refTime,
    HAPFakeGatoType_weather,
    HAPFakeGatoType_energy,
    HAPFakeGatoType_energy18,
    HAPFakeGatoType_room,
    HAPFakeGatoType_motion,
    HAPFakeGatoType_door,
    HAPFakeGatoType_thermo,
    HAPFakeGatoType_aqua,
    HAPFakeGatoType_aqua21,
    HAPFakeGatoType_switch,
};

union ui32_to_ui8 {
    uint32_t ui32;
    uint8_t ui8[4];
};

union ui16_to_ui8 {
    uint16_t ui16;
    uint8_t ui8[2];
};


union HAPFakeGatoInfoStart {
            struct {
/* 4 */         uint32_t evetime;               // Actual time, in seconds from last time update
/* 4 */         uint32_t negativeOffset;        // negative offset of reference time
/* 4 */         uint32_t refTimeLastUpdate;     // reference time/last Accessory time update
                                                // (taken from E863F117-079E-48FF-8F27-9C2605A29F52)            
/* 1 */         uint8_t sigLength;    // number of 16 bits word of the following "signature" portion
            } data;
            uint8_t bytes[13];
}; 

union HAPFakeGatoInfoEnd {
    struct {
/* 2 */     uint16_t usedMemory;        // last physical memory position occupied (used by Eve.app 
                                        // to understand how many transfers are needed). 
                                        //
                                        // If set to an address lower than the last successfully 
                                        // uploaded entry, forces Eve.app to start from the beginning 
                                        // of the memory, asking address 00 in E863F11C. 
                                        // Accessory answers with entry 01. 
                                        //
                                        // Once the memory is fully written and memory overwriting is necessary 
                                        // this field remains equal to history size.

/* 2 */     uint16_t size;              // history size
/* 4 */     uint32_t rollOver;          // once memory rolling occurred it indicates the address of the oldest entry 
                                        // present in memory 
                                        // (if memory rolling did not occur yet, these bytes are at 0)   

/* 4 */     uint32_t unknown;
/* 2 */     uint16_t end;               // ?? always 01ff or 0101
    } data;
    uint8_t bytes[14];
};


// template <class TFakeGatoData>
class HAPFakeGato {
public:    
    HAPFakeGato();
    ~HAPFakeGato() {};

    void registerFakeGatoService(HAPAccessory* accessory, String name);
    
    void handle(bool forced = false);

    virtual void    begin()     = 0;
    virtual size_t  size()      = 0;
    virtual bool    isFull()    = 0;
    // virtual bool    isEmpty()   = 0;
    virtual void    clear()     = 0;
    virtual void    getData(const size_t count, uint8_t *data, size_t* length, uint16_t offset) = 0;
    // virtual bool    addRefTimeEntry(uint32_t timestmap = 0) = 0;

    inline void setRefTime(uint32_t reftime){
        _refTime = reftime;
    }

    inline unsigned long interval(){
		return _interval;
	}	

	inline void setInterval(unsigned long interval){
		_interval = interval;
	}

	inline void setName(String name){
		_name = name;
	}

    inline String name(){
		return _name;
	}

    inline bool isEnabled(){
		return _isEnabled;
	}

	inline void enable(bool mode){
		_isEnabled = mode;
	}

    inline void registerCallback(std::function<bool()> callback){
        _callbackAddEntry = callback;
    }
    

protected:
    std::function<bool()> _callbackAddEntry = NULL;  
    
    String                _name;
    dataCharacteristics*  _s2r1Characteristics;
    dataCharacteristics*  _s2r2Characteristics;
    dataCharacteristics*  _s2w1Characteristics;
    dataCharacteristics*  _s2w2Characteristics;
    
    enum HAPFakeGatoType    _type;
    bool                    _isEnabled;
    uint32_t                _refTime;    

    
    bool                    _rolledOver;

    uint16_t                _memoryUsed;    // last physical memory position occupied
    //uint16_t                _memorySize;
    

    uint32_t                _idxWrite;      // Write index
    uint32_t                _idxRead;       // Read index

    uint32_t                _requestedEntry;


    uint32_t*               _ptrTimestampLastEntry;

    uint32_t     		    _previousMillis;
    uint32_t     			_interval;
    bool                    _transfer;

    void getRefTime(uint8_t *data, size_t* length, const uint16_t offset);
    
    uint32_t getWriteIndex(){
        return _idxWrite;
    }

    uint32_t getReadIndex(){
        return _idxRead;
    }

    virtual void getSignature(uint8_t* signature) = 0;
    virtual int signatureLength() = 0;


    inline uint32_t incrementIndex(uint32_t index){
        return (index + 1) % HAP_FAKEGATO_BUFFER_SIZE;
    }

    inline uint32_t decrementIndex(uint32_t index){
        return (index + HAP_FAKEGATO_BUFFER_SIZE - 1) % HAP_FAKEGATO_BUFFER_SIZE;
    }

    void getS2R2Callback();

    void updateS2R1Value();
    void updateS2R2Value();

    bool shouldHandle();


    void setS2R1Characteristics(String oldValue, String newValue);
    void setS2R2Characteristics(String oldValue, String newValue);

    void setS2W1Characteristics(String oldValue, String newValue);
    void setS2W2Characteristics(String oldValue, String newValue);
};

#endif /* HAPFAKEGATO_HPP_ */