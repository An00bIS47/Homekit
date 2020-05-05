//
// HAPFakeGatoWeather.cpp
// Homekit
//
//  Created on: 29.08.2019
//      Author: michael
//
#include "HAPFakeGatoWeather.hpp"
#include "HAPServer.hpp"

#define HAP_FAKEGATO_SIGNATURE_LENGTH    3     // number of 16 bits word of the following "signature" portion
#define HAP_FAKEGATO_DATA_LENGTH        16     // length of the data

HAPFakeGatoWeather::HAPFakeGatoWeather(){    
    
    _interval       = HAP_FAKEGATO_INTERVAL;
	_previousMillis = 0;
    _type           = HAPFakeGatoType_weather;
    _isEnabled      = true;
    _name           = "";
    _memoryUsed     = 0;
    _requestedEntry = 0;
    
    _refTime        = 0;
    _vectorBuffer   = nullptr;

    _idxRead        = 0;            // gets incremented when poped
    _idxWrite       = 0;            // gets incremented when pushed
	_transfer       = false;
    _rolledOver     = false;    
}

HAPFakeGatoWeather::~HAPFakeGatoWeather(){


    _vectorBuffer->clear();
    if (_vectorBuffer != nullptr){
        delete _vectorBuffer;
    }
}

void HAPFakeGatoWeather::begin(){              

    if (_vectorBuffer == nullptr) {
        _vectorBuffer = new std::vector<HAPFakeGatoWeatherData>(HAP_FAKEGATO_BUFFER_SIZE);
    }
}

int HAPFakeGatoWeather::signatureLength(){
    return HAP_FAKEGATO_SIGNATURE_LENGTH;
}

void HAPFakeGatoWeather::getSignature(uint8_t* signature){
    ui16_to_ui8 s1, s2, s3;    
    s1.ui16 = __builtin_bswap16(0x0102);
    s2.ui16 = __builtin_bswap16(0x0202);
    s3.ui16 = __builtin_bswap16(0x0302);

    memcpy(signature, s1.ui8, 2);
    memcpy(signature + 2, s2.ui8, 2);
    memcpy(signature + 2 + 2, s3.ui8, 2);
    // *length = signatureLength();    
}


bool HAPFakeGatoWeather::addEntry(String stringTemperature, String stringHumidity, String stringPressure){        


    LogD(HAPServer::timeString() + " " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " [   ] " + "Adding entry for " + _name + " [size=" + String(_memoryUsed) + "]: temp=" + stringTemperature + " hum=" + stringHumidity + " pres=" + stringPressure, true);
    
    uint16_t valueTemperature   = (uint16_t) stringTemperature.toFloat()    * 100;
    uint16_t valueHumidity      = (uint16_t) stringHumidity.toFloat()       * 100;
    uint16_t valuePressure      = (uint16_t) stringPressure.toInt()         * 10;        

    HAPFakeGatoWeatherData data = (HAPFakeGatoWeatherData){
        HAPServer::timestamp(),
        // false,
        valueTemperature,
        valueHumidity,
        valuePressure,        
    };    

    return addEntry(data);
}


bool HAPFakeGatoWeather::addEntry(HAPFakeGatoWeatherData data){

    //LogD(HAPServer::timeString() + " " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " [   ] " + "Add fakegato data for " + _name + " ..." , true);

    if (_vectorBuffer == nullptr) {
        begin();
    }
    
    if (_memoryUsed < HAP_FAKEGATO_BUFFER_SIZE){
        _memoryUsed++;
    }

    // ToDo: Fix
    _ptrTimestampLastEntry = &data.timestamp;
    

    (*_vectorBuffer)[_idxWrite] = data;
    
    // increment write index
    _idxWrite = incrementIndex(_idxWrite);

    // Serial.print("_idxWrite: ");
    // Serial.println(_idxWrite);


    if (_memoryUsed == _vectorBuffer->size()){
        //  Serial.println("Rolled over");

        _rolledOver = true;  
        _idxRead = incrementIndex(_idxWrite);      
    }

    // Serial.print("_idxRead: ");
    // Serial.println(_idxRead);

#if HAP_DEBUG_FAKEGATO_DETAILED
    Serial.print("_memoryUsed: ");
    Serial.println(_memoryUsed);

    for (int i=0; i< HAP_FAKEGATO_BUFFER_SIZE; i++){
        HAPFakeGatoWeatherData entryData;    
        entryData = (*_vectorBuffer)[i];

        if (i == _idxWrite) {
            Serial.printf("No. %d - %d  temp: %d <<< w:%d\n", i, entryData.timestamp,entryData.temperature, _idxWrite);
        } else {
            Serial.printf("No. %d - %d  temp: %d\n", i, entryData.timestamp,entryData.temperature);
        }
        
    }
#endif


    updateS2R1Value();        
    
    return !_rolledOver;

}

// TODO: Read from index requested by EVE app
void HAPFakeGatoWeather::getData(const size_t count, uint8_t *data, size_t* length, uint16_t offset){

#if HAP_DEBUG_FAKEGATO      
    LogD(HAPServer::timeString() + " " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " [   ] " + "Get fakegato data for " + _name + " ..." , true);
#endif

#if HAP_DEBUG_FAKEGATO_DETAILED
    for (int i=0; i< HAP_FAKEGATO_BUFFER_SIZE; i++){
        HAPFakeGatoWeatherData entryData;    
        entryData = (*_vectorBuffer)[i];
        if (i == _idxWrite) {
            Serial.printf("No. %d - %d  temp: %d <<< w:%d\n", i, entryData.timestamp,entryData.temperature, _idxWrite);
        } else {
            Serial.printf("No. %d - %d  temp: %d\n", i, entryData.timestamp,entryData.temperature);
        }
    }
#endif
    uint32_t tmpRequestedEntry = (_requestedEntry - 1) % HAP_FAKEGATO_BUFFER_SIZE;


    if ( (tmpRequestedEntry >= _idxWrite) && ( _rolledOver == false) ){
        _transfer = false;
        LogW("WARNING: Fakegato could not send the requested entry. The requested index does not exist!", true);                          
        return;
    }

    HAPFakeGatoWeatherData entryData;    
    entryData = (*_vectorBuffer)[tmpRequestedEntry];
       
    for (int i = 0; i < count; i++){            

        uint8_t size = HAP_FAKEGATO_DATA_LENGTH;
        uint8_t typ = HAP_FAKEGATO_TYPE_WEATHER;
        memcpy(data + offset, (uint8_t *)&size, 1);

        ui32_to_ui8 eC;
        eC.ui32 = _requestedEntry++;
        memcpy(data + offset + 1, eC.ui8, 4);

        ui32_to_ui8 secs;
        secs.ui32 = entryData.timestamp - _refTime;
        memcpy(data + offset + 1 + 4, secs.ui8, 4);
        
        memcpy(data + offset + 1 + 4 + 4, (uint8_t*)&typ, 1);

        ui16_to_ui8 temp, hum, pressure;
        temp.ui16       = entryData.temperature;
        hum.ui16        = entryData.humidity;
        pressure.ui16   = entryData.pressure;
        memcpy(data + offset + 1 + 4 + 4 + 1, temp.ui8, 2);
        memcpy(data + offset + 1 + 4 + 4 + 1 + 2, hum.ui8, 2);
        memcpy(data + offset + 1 + 4 + 4 + 1 + 2 + 2, pressure.ui8, 2);
        
        *length = offset + HAP_FAKEGATO_DATA_LENGTH;    
        offset  = offset + HAP_FAKEGATO_DATA_LENGTH;

        // _noOfEntriesSent++;            
        if ( (tmpRequestedEntry + 1 >= _idxWrite )  && ( _rolledOver == false) ){
            _transfer = false;    

            LogW("WARNING: Fakegato could not send the requested entry", true);                              
            break;
        }

        
        uint32_t tsOld = entryData.timestamp;
        
        tmpRequestedEntry = incrementIndex(tmpRequestedEntry);
        entryData = (*_vectorBuffer)[tmpRequestedEntry];
        
        if ( _rolledOver == true) { 
            if (tsOld > entryData.timestamp) {
                _transfer = false;  
                LogW("WARNING: Fakegato could not send the requested entry", true);                                 
                break;
            }
        }
    }             
}
