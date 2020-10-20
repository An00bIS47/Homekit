//
// HAPFakeGatoHygrometer.cpp
// Homekit
//
//  Created on: 29.08.2019
//      Author: michael
//
#include "HAPFakeGatoHygrometer.hpp"
#include "HAPServer.hpp"

#define HAP_FAKEGATO_SIGNATURE_LENGTH    3     // number of 16 bits word of the following "signature" portion

HAPFakeGatoHygrometer::HAPFakeGatoHygrometer(){    
    
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

HAPFakeGatoHygrometer::~HAPFakeGatoHygrometer(){

    if (_vectorBuffer != nullptr){
        _vectorBuffer->clear();
        delete _vectorBuffer;
    }
}

void HAPFakeGatoHygrometer::begin(){              

    if (_vectorBuffer == nullptr) {
        _vectorBuffer = new std::vector<HAPFakeGatoHygrometerData>(HAP_FAKEGATO_BUFFER_SIZE);
    }
}

int HAPFakeGatoHygrometer::signatureLength(){
    return HAP_FAKEGATO_SIGNATURE_LENGTH;    
}

void HAPFakeGatoHygrometer::getSignature(uint8_t* signature){

    signature[0] = (uint8_t)HAPFakeGatoSignature_Temperature;
    signature[1] = 2;

    signature[2] = (uint8_t)HAPFakeGatoSignature_Humidity;
    signature[3] = 2;
    
    signature[4] = (uint8_t)HAPFakeGatoSignature_AirPressure;
    signature[5] = 2;     

#if HAP_DEBUG_FAKEGATO   
    HAPHelper::array_print("Fakegato signature", signature, 2);
#endif

}


bool HAPFakeGatoHygrometer::addEntry(String stringHumidity){        


    LogD(HAPServer::timeString() + " " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " [   ] " + "Adding entry for " + _name + " [size=" + String(_memoryUsed) + "]: hum=" + stringHumidity, true);
    
    uint16_t valueHumidity      = (uint16_t) (stringHumidity.toFloat()       * 100);

    HAPFakeGatoHygrometerData data = (HAPFakeGatoHygrometerData){
        HAPServer::timestamp(),
        0x02,
        valueHumidity,
    };    

    return addEntry(data);
}

bool HAPFakeGatoHygrometer::addEntry(uint32_t timestamp, String stringHumidity){        


    LogD(HAPServer::timeString() + " " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " [   ] " + "Adding entry for " + _name + " [size=" + String(_memoryUsed) + "]: hum=" + stringHumidity, true);
    
    uint16_t valueHumidity      = (uint16_t) (stringHumidity.toFloat()       * 100);

    HAPFakeGatoHygrometerData data = (HAPFakeGatoHygrometerData){
        timestamp,
        0x02,       
        valueHumidity,       
    };    

    return addEntry(data);
}


bool HAPFakeGatoHygrometer::addEntry(HAPFakeGatoHygrometerData data){

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
            Serial.printf("No. %d - %d  hum: %d<<< w:%d\n", i, entryData.timestamp, entryData.humidity, _idxWrite);
        } else {
            Serial.printf("No. %d - %d  hum: %d\n", i, entryData.timestamp, entryData.humidity);
        }
        
    }
#endif


    updateS2R1Value();        
    
    return !_rolledOver;

}

// TODO: Read from index requested by EVE app
void HAPFakeGatoHygrometer::getData(const size_t count, uint8_t *data, size_t* length, uint16_t offset){

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
            Serial.printf("No. %d - %d  hum: %d  <<< w:%d\n", i, entryData.timestamp, entryData.humidity, _idxWrite);
        } else {
            Serial.printf("No. %d - %d  hum: %d  \n", i, entryData.timestamp, entryData.humidity);
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

    HAPFakeGatoHygrometerData entryData;    
    entryData = (*_vectorBuffer)[tmpRequestedEntry];

    for (int i = 0; i < count; i++){            
        uint8_t currentOffset = 0;



        // uint8_t size = HAP_FAKEGATO_DATA_LENGTH;  
        // ToDo: ToDo: Hygrometer test
        uint8_t size = 10;
        size += (((entryData.bitmask & 0x04) >> 2) * 2);  
        size += (((entryData.bitmask & 0x02) >> 1) * 2);  
        size +=  ((entryData.bitmask & 0x01) * 2);        

        memcpy(data + offset + currentOffset, (uint8_t *)&size, 1);
        currentOffset += 1;

        // ToDo: Rewrite and remove unions
        ui32_to_ui8 eC;
        eC.ui32 = _requestedEntry++;
        memcpy(data + offset + currentOffset, eC.ui8, 4);
        currentOffset += 4;

        ui32_to_ui8 secs;
        secs.ui32 = entryData.timestamp - _refTime;
        memcpy(data + offset + currentOffset, secs.ui8, 4);
        currentOffset += 4;

         // ToDo: make proper use of the bitmask!
        memcpy(data + offset + currentOffset, (uint8_t*)&entryData.bitmask, 1);
        currentOffset += 1;                

        // if ((entryData.bitmask & 0x01) == 1) {            
        //     ui16_to_ui8 temp;
        //     temp.ui16 = entryData.temperature;
        //     memcpy(data + offset + currentOffset, temp.ui8, 2);
        //     currentOffset += 2;
        // } 

        if (((entryData.bitmask & 0x02) >> 1) == 1){
            ui16_to_ui8 hum;
            hum.ui16 = entryData.humidity;
            memcpy(data + offset + currentOffset, hum.ui8, 2);
            currentOffset += 2;
        }

        // if (((entryData.bitmask & 0x04) >> 2) == 1) {            
        //     ui16_to_ui8 pressure;
        //     pressure.ui16 = entryData.pressure;
        //     memcpy(data + offset + currentOffset, pressure.ui8, 2);        
        //     currentOffset += 2;
        // }

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
