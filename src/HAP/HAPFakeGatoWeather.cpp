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

    _periodicUpdates = true;
}

HAPFakeGatoWeather::~HAPFakeGatoWeather(){

    if (_vectorBuffer != nullptr){
        _vectorBuffer->clear();
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
    // ToDo: Hygrometer test
    // return 1;
}

void HAPFakeGatoWeather::getSignature(uint8_t* signature){

    // 0102 0202 0302
	//	|	  |	   +-> Pressure	
	//  |	  +-> Humidity
	//  +-> Temp
	// 
	// bitmask 0x07 => all			= 111
	// bitmask 0x01 => temp			= 001
	// bitmask 0x02 => hum			= 010
	// bitmask 0x04 => pressure		= 100


    signature[0] = (uint8_t)HAPFakeGatoSignature_Temperature;
    signature[1] = 2;

    signature[2] = (uint8_t)HAPFakeGatoSignature_Humidity;
    signature[3] = 2;
    
    signature[4] = (uint8_t)HAPFakeGatoSignature_AirPressure;
    signature[5] = 2;     

#if HAP_DEBUG_FAKEGATO
    HAPHelper::array_print("Fakegato signature", signature, 6);
#endif

}


bool HAPFakeGatoWeather::addEntry(uint8_t bitmask, String stringTemperature, String stringHumidity, String stringPressure){        


    LogD(HAPServer::timeString() + " " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " [   ] " + "Adding entry for " + _name + " [size=" + String(_memoryUsed) + "]: temp=" + stringTemperature + " hum=" + stringHumidity + " pres=" + stringPressure, true);
    
    uint16_t valueTemperature   = (uint16_t) (stringTemperature.toFloat()    * 100);
    uint16_t valueHumidity      = (uint16_t) (stringHumidity.toFloat()       * 100);
    uint16_t valuePressure      = (uint16_t) (stringPressure.toInt()         * 10);        

#if HAP_DEBUG_FAKEGATO       
    Serial.printf("valueTemperature: %d   valueHumidity: %d   valuePressure: %d\n", valueTemperature, valueHumidity, valuePressure);
#endif


    HAPFakeGatoWeatherData data = (HAPFakeGatoWeatherData){
        HAPServer::timestamp(),
        // false,
        bitmask,
        valueTemperature,
        valueHumidity,
        valuePressure,        
    };    

    return addEntry(data);
}

bool HAPFakeGatoWeather::addEntry(uint8_t bitmask, uint32_t timestamp, String stringTemperature, String stringHumidity, String stringPressure){        


    LogD(HAPServer::timeString() + " " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " [   ] " + "Adding entry for " + _name + " [size=" + String(_memoryUsed) + "]: temp=" + stringTemperature + " hum=" + stringHumidity + " pres=" + stringPressure, true);
    
    uint16_t valueTemperature   = (uint16_t) (stringTemperature.toFloat()    * 100);
    uint16_t valueHumidity      = (uint16_t) (stringHumidity.toFloat()       * 100);
    uint16_t valuePressure      = (uint16_t) (stringPressure.toInt()         * 10);        

#if HAP_DEBUG_FAKEGATO   
    Serial.printf("valueTemperature: %d   valueHumidity: %d   valuePressure: %d\n", valueTemperature, valueHumidity, valuePressure);
#endif

    HAPFakeGatoWeatherData data = (HAPFakeGatoWeatherData){
        timestamp,
        // false,
        bitmask,       
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

    // ToDo: Fix?
    _timestampLastEntry = data.timestamp;
    

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
    Serial.print("ADD DATA: _memoryUsed: ");
    Serial.println(_memoryUsed);

    for (int i=0; i< HAP_FAKEGATO_BUFFER_SIZE; i++){
        HAPFakeGatoWeatherData entryData;    
        entryData = (*_vectorBuffer)[i];

        if (entryData.timestamp == 0) break;
        if (i == _idxWrite) {
            Serial.printf("No. %d - %d  temp: %d  hum: %d  pres: %d <<< w:%d\n", i, entryData.timestamp, entryData.temperature, entryData.humidity, entryData.pressure, _idxWrite);
        } else {
            Serial.printf("No. %d - %d  temp: %d  hum: %d  pres: %d \n", i, entryData.timestamp, entryData.temperature, entryData.humidity, entryData.pressure);
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
    Serial.print("GET DATA: _memoryUsed: ");
    Serial.println(_memoryUsed);
    for (int i=0; i< HAP_FAKEGATO_BUFFER_SIZE; i++){
        HAPFakeGatoWeatherData entryData;    
        entryData = (*_vectorBuffer)[i];

        if (entryData.timestamp == 0) break;
        if (i == _idxWrite) {
            Serial.printf("No. %d - %d  temp: %d  hum: %d  pres: %d <<< w:%d\n", i, entryData.timestamp, entryData.temperature, entryData.humidity, entryData.pressure, _idxWrite);
        } else {
            Serial.printf("No. %d - %d  temp: %d  hum: %d  pres: %d \n", i, entryData.timestamp, entryData.temperature, entryData.humidity, entryData.pressure);
        }
    }
#endif
    uint32_t tmpRequestedEntry = (_requestedEntry - 1) % HAP_FAKEGATO_BUFFER_SIZE;


    if ( (tmpRequestedEntry >= _idxWrite) && ( _rolledOver == false) ){
        _transfer = false;
#if HAP_DEBUG_FAKEGATO              
        LogW("ERROR: No newer entries available. Requested entry goes beyond write index!", true);                          
        LogW("   - tmpRequestedEntry=" + String(tmpRequestedEntry), true);
        LogW("   - _requestedEntry=" + String(_requestedEntry), true);
        LogW("   - _idxWrite=" + String(_idxWrite), true);
        LogW("   - _rolledOver=" + String(_rolledOver), true);                            
#endif 
        return;
    }

    HAPFakeGatoWeatherData entryData;    
    entryData = (*_vectorBuffer)[tmpRequestedEntry];

    for (int i = 0; i < count; i++){            
        uint8_t currentOffset = 0;


        uint8_t size = 10;
        size += (((entryData.bitmask & 0x04) >> 2) * 2);  
        size += (((entryData.bitmask & 0x02) >> 1) * 2);  
        size +=  ((entryData.bitmask & 0x01) * 2);        

        // size
        memcpy(data + offset + currentOffset, (uint8_t *)&size, 1);
        currentOffset += 1;

        // requested Entry
        ui32_to_ui8 eC;
        eC.ui32 = _requestedEntry++;
        memcpy(data + offset + currentOffset, eC.ui8, 4);
        currentOffset += 4;

        // timestamp
        ui32_to_ui8 secs;
        secs.ui32 = entryData.timestamp - _refTime;
        memcpy(data + offset + currentOffset, secs.ui8, 4);
        currentOffset += 4;

        // bitmask
        memcpy(data + offset + currentOffset, (uint8_t*)&entryData.bitmask, 1);
        currentOffset += 1;        

        // temperature
        if ((entryData.bitmask & 0x01) == 1) {            
            ui16_to_ui8 temp;
            temp.ui16 = entryData.temperature;
            memcpy(data + offset + currentOffset, temp.ui8, 2);
            currentOffset += 2;
        } 

        // humidity
        if (((entryData.bitmask & 0x02) >> 1) == 1){
            ui16_to_ui8 hum;
            hum.ui16 = entryData.humidity;
            memcpy(data + offset + currentOffset, hum.ui8, 2);
            currentOffset += 2;
        }

        // pressure
        if (((entryData.bitmask & 0x04) >> 2) == 1) {            
            ui16_to_ui8 pressure;
            pressure.ui16 = entryData.pressure;
            memcpy(data + offset + currentOffset, pressure.ui8, 2);        
            currentOffset += 2;
        }                                            

        offset  += currentOffset;
        *length = offset;   


#if HAP_DEBUG_FAKEGATO   
        HAPHelper::array_print("Fakegato data", data + offset, currentOffset);
#endif

        // _noOfEntriesSent++;            
        if ( (tmpRequestedEntry + 1 >= _idxWrite )  && ( _rolledOver == false) ){
            _transfer = false;  
#if HAP_DEBUG_FAKEGATO              
            LogW("ERROR: No newer entries available. Requested entry goes beyond write index!", true);                          
            LogW("   - tmpRequestedEntry=" + String(tmpRequestedEntry), true);
            LogW("   - _requestedEntry=" + String(_requestedEntry), true);
            LogW("   - _idxWrite=" + String(_idxWrite), true);
            LogW("   - _rolledOver=" + String(_rolledOver), true);                            
#endif            
            break;
        }

        uint32_t tsOld = entryData.timestamp;
        
        tmpRequestedEntry = incrementIndex(tmpRequestedEntry);
        entryData = (*_vectorBuffer)[tmpRequestedEntry];
        
        
        if ( _rolledOver == true) { 
            if (tsOld > entryData.timestamp) {
                _transfer = false;  
#if HAP_DEBUG_FAKEGATO                
                LogW("ERROR: No newer entries available. Older timestamp is newer than the new timestamp!", true);                          
                LogW("   - tmpRequestedEntry=" + String(tmpRequestedEntry), true);
                LogW("   - _requestedEntry=" + String(_requestedEntry), true);
                LogW("   - _idxWrite=" + String(_idxWrite), true);
                LogW("   - _rolledOver=" + String(_rolledOver), true);                              
#endif                    
                break;
            }
        }
    }             
}
