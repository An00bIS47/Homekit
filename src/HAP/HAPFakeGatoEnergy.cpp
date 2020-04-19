//
// HAPFakeGatoWeatther.cpp
// Homekit
//
//  Created on: 29.08.2019
//      Author: michael
//
#include "HAPFakeGatoEnergy.hpp"
#include "HAPServer.hpp"

#define HAP_FAKEGATO_SIGNATURE_LENGTH    4      // number of 16 bits word of the following "signature" portion
#define HAP_FAKEGATO_DATA_LENGTH        20      // length of the data

HAPFakeGatoEnergy::HAPFakeGatoEnergy(){
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

HAPFakeGatoEnergy::~HAPFakeGatoEnergy(){

    _vectorBuffer->clear();
    if (_vectorBuffer != nullptr){
        delete _vectorBuffer;
    }

}


void HAPFakeGatoEnergy::begin(){
               
    if (_vectorBuffer == nullptr) {
        _vectorBuffer = new std::vector<HAPFakeGatoEnergyData>(HAP_FAKEGATO_BUFFER_SIZE);
    }
}

int HAPFakeGatoEnergy::signatureLength(){
    return HAP_FAKEGATO_SIGNATURE_LENGTH;
}

void HAPFakeGatoEnergy::getSignature(uint8_t* signature){
    ui16_to_ui8 s1, s2, s3, s4;    
    s1.ui16 = __builtin_bswap16(0x0102);
    s2.ui16 = __builtin_bswap16(0x0202);
    s3.ui16 = __builtin_bswap16(0x0702);
    s4.ui16 = __builtin_bswap16(0x0f03);


    memcpy(signature, s1.ui8, 2);
    memcpy(signature + 2, s2.ui8, 2);
    memcpy(signature + 2 + 2, s3.ui8, 2);
    memcpy(signature + 2 + 2 + 2, s4.ui8, 2);
    // *length = signatureLength();    
}


bool HAPFakeGatoEnergy::addEntry(String stringPower){        

    LogD(HAPServer::timeString() + " " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " [   ] " + "Adding entry for " + _name + " [size=" + String(size()) + "]: power=" + stringPower, true);
    
    uint16_t valuePower         = (uint16_t) stringPower.toInt()    * 10;

    HAPFakeGatoEnergyData data = (HAPFakeGatoEnergyData){
        HAPServer::timestamp(),
        // false,
        valuePower       
    };    

    return addEntry(data);
}

bool HAPFakeGatoEnergy::addEntry(HAPFakeGatoEnergyData data){

#if HAP_DEBUG_FAKEGATO    
    LogD(HAPServer::timeString() + " " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " [   ] " + "Add fakegato data for " + _name + " ..." , true);
#endif

    if (_vectorBuffer == nullptr) {
        begin();
    }
    
    if (_memoryUsed < HAP_FAKEGATO_BUFFER_SIZE){
        _memoryUsed++;
    }
    

    _ptrTimestampLastEntry = &data.timestamp;
    

    (*_vectorBuffer)[_idxWrite] = data;
    
    // increment write index
    _idxWrite = incrementIndex(_idxWrite);

    
    if (_memoryUsed == _vectorBuffer->size()){
        //  Serial.println("Rolled over");

        _rolledOver = true;  
        _idxRead = incrementIndex(_idxWrite);      
    }

    updateS2R1Value();        
    
    return !_rolledOver;

}


void HAPFakeGatoEnergy::getData(const size_t count, uint8_t *data, size_t* length, uint16_t offset){
#if HAP_DEBUG_FAKEGATO      
    LogD(HAPServer::timeString() + " " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " [   ] " + "Get fakegato data for " + _name + " ..." , true);
#endif
    uint32_t tmpRequestedEntry = (_requestedEntry - 1) % HAP_FAKEGATO_BUFFER_SIZE;

    if ( (tmpRequestedEntry >= _idxWrite) && ( _rolledOver == false) ){
        _transfer = false;
        LogW("WARNING: Fakegato could not send the requested entry. The requested index does not exist!", true);                          
        return;
    }

    HAPFakeGatoEnergyData entryData;    
    entryData = (*_vectorBuffer)[tmpRequestedEntry];
       
    for (int i = 0; i < count; i++){
        
        uint8_t size = HAP_FAKEGATO_DATA_LENGTH;
        uint8_t typ = HAP_FAKEGATO_TYPE_ENERGY;
        memcpy(data + offset, (uint8_t *)&size, 1);

        ui32_to_ui8 eC;
        eC.ui32 = _requestedEntry++;
        memcpy(data + offset + 1, eC.ui8, 4);

        ui32_to_ui8 secs;
        secs.ui32 = entryData.timestamp - _refTime;
        memcpy(data + offset + 1 + 4, secs.ui8, 4);
        
        memcpy(data + offset + 1 + 4 + 4, (uint8_t*)&typ, 1);

        // 2x unknown
        memset(data + offset + 1 + 4 + 4 + 1, 0x00, 2);
        memset(data + offset + 1 + 4 + 4 + 1 + 2, 0x00, 2);


        ui16_to_ui8 power;
        power.ui16       = entryData.power;
        memcpy(data + offset + 1 + 4 + 4 + 1 + 2 + 2, power.ui8, 2);


        // 2x unknown
        memset(data + offset + 1 + 4 + 4 + 1 + 2 + 2 + 2, 0x00, 2);
        memset(data + offset + 1 + 4 + 4 + 1 + 2 + 2 + 2 + 2, 0x00, 2);
        
        *length = offset + HAP_FAKEGATO_DATA_LENGTH;    
        offset  = offset + HAP_FAKEGATO_DATA_LENGTH;

        // _noOfEntriesSent++;

        if ( (tmpRequestedEntry + 1 >= _idxWrite )  && ( _rolledOver == false) ){
            _transfer = false;    

            //Serial.println(">>>>>>>>>>>> ABORT 1");                                
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