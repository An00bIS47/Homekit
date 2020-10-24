//
// HAPFakeGatoSwitch.cpp
// Homekit
//
//  Created on: 21.12.2019
//      Author: michael
//
#include "HAPFakeGatoSwitch.hpp"
#include "HAPServer.hpp"

#define HAP_FAKEGATO_SIGNATURE_LENGTH    1     // number of 16 bits word of the following "signature" portion
#define HAP_FAKEGATO_DATA_LENGTH        11     // length of the data

HAPFakeGatoSwitch::HAPFakeGatoSwitch(){    
    
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

    _periodicUpdates = false;       // only write a fakegato entry if an action; not periodic !
}

HAPFakeGatoSwitch::~HAPFakeGatoSwitch(){
    
    if (_vectorBuffer != nullptr){
        _vectorBuffer->clear();
        delete _vectorBuffer;
    }
}

void HAPFakeGatoSwitch::begin(){              

    if (_vectorBuffer == nullptr) {
        _vectorBuffer = new std::vector<HAPFakeGatoSwitchData>(HAP_FAKEGATO_BUFFER_SIZE);
    }
}

int HAPFakeGatoSwitch::signatureLength(){
    return HAP_FAKEGATO_SIGNATURE_LENGTH;
}

void HAPFakeGatoSwitch::getSignature(uint8_t* signature){
    ui16_to_ui8 s1;    
    s1.ui16 = __builtin_bswap16(0x0E01);

    memcpy(signature, s1.ui8, 2);
}


bool HAPFakeGatoSwitch::addEntry(String status){        

    LogD(HAPServer::timeString() + " " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " [   ] " + "Adding entry for " + _name + " [size=" + String(_memoryUsed) + "]: status=" + status, true);
    
    uint8_t valueStatus   = status.toInt();    

    HAPFakeGatoSwitchData data = (HAPFakeGatoSwitchData){
        HAPServer::timestamp(),
        valueStatus       
    };    

    return addEntry(data);
}

bool HAPFakeGatoSwitch::addEntry(HAPFakeGatoSwitchData data){
    
    // if (_ringbuffer == nullptr) {
    //     begin();
    // }
    
    LogD(HAPServer::timeString() + " " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " [   ] " + "Add fakegato data for " + _name + " ..." , true);

    if (_vectorBuffer == nullptr) {
        begin();
    }
    
    if (_memoryUsed < HAP_FAKEGATO_BUFFER_SIZE){
        _memoryUsed++;
    }

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

#if HAP_DEBUG_FAKEGATO
    Serial.print("_memoryUsed: ");
    Serial.println(_memoryUsed);

    for (int i=0; i< HAP_FAKEGATO_BUFFER_SIZE; i++){
        HAPFakeGatoSwitchData entryData;    
        entryData = (*_vectorBuffer)[i];

        if (i == _idxWrite) {
            Serial.printf("No. %d - %d  status: %d <<< w:%d\n", i, entryData.timestamp,entryData.status, _idxWrite);
        } else {
            Serial.printf("No. %d - %d  status: %d\n", i, entryData.timestamp,entryData.status);
        }        
    }
#endif


    updateS2R1Value();        
    
    return !_rolledOver;

}

// TODO: Read from index requested by EVE app
void HAPFakeGatoSwitch::getData(const size_t count, uint8_t *data, size_t* length, uint16_t offset){
#if HAP_DEBUG_FAKEGATO      
    LogD(HAPServer::timeString() + " " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " [   ] " + "Get fakegato data for " + _name + " ..." , true);
#endif

#if HAP_DEBUG_FAKEGATO
    for (int i=0; i< HAP_FAKEGATO_BUFFER_SIZE; i++){
        HAPFakeGatoSwitchData entryData;    
        entryData = (*_vectorBuffer)[i];
        if (i == _idxWrite) {
            Serial.printf("No. %d - %d  status: %d <<< w:%d\n", i, entryData.timestamp,entryData.status, _idxWrite);
        } else {
            Serial.printf("No. %d - %d  status: %d\n", i, entryData.timestamp,entryData.status);
        }
    }
#endif
    uint32_t tmpRequestedEntry = (_requestedEntry - 1) % HAP_FAKEGATO_BUFFER_SIZE;


#if HAP_DEBUG_FAKEGATO    
    Serial.println(">>>>> getData");
    Serial.print("0 tmpRequestedEntry: ");
    Serial.println(tmpRequestedEntry);

    Serial.print("0 _idxRead: ");
    Serial.println(_idxRead);

    Serial.print("0 _requestedEntry: ");
    Serial.println(_requestedEntry);

    Serial.print("0 _idxWrite: ");
    Serial.println(_idxWrite);
#endif

    if ( (tmpRequestedEntry >= _idxWrite) && ( _rolledOver == false) ){
        _transfer = false;
        LogE("ERROR: Fakegato Switch could not send the requested entry. The requested index does not exist!", true);                          
        LogE("   - tmpRequestedEntry=" + String(tmpRequestedEntry), true);
        LogE("   - _requestedEntry=" + String(_requestedEntry), true);
        LogE("   - _idxWrite=" + String(_idxWrite), true);
        LogE("   - _rolledOver=" + String(_rolledOver), true);
        return;
    }

    HAPFakeGatoSwitchData entryData;    
    entryData = (*_vectorBuffer)[tmpRequestedEntry];
       
    for (int i = 0; i < count; i++){
            
#if HAP_DEBUG_FAKEGATO    
        Serial.print("tmpRequestedEntry: ");
        Serial.println(tmpRequestedEntry);

        Serial.print("_idxRead: ");
        Serial.println(_idxRead);

        Serial.print("_requestedEntry: ");
        Serial.println(_requestedEntry);
#endif

        uint8_t size = HAP_FAKEGATO_DATA_LENGTH;
        memcpy(data + offset, (uint8_t *)&size, 1);

        ui32_to_ui8 eC;
        eC.ui32 = _requestedEntry++;
        memcpy(data + offset + 1, eC.ui8, 4);

        ui32_to_ui8 secs;
        secs.ui32 = entryData.timestamp - _refTime;
        memcpy(data + offset + 1 + 4, secs.ui8, 4);
        
        uint8_t bitmask = 0x01;
        memcpy(data + offset + 1 + 4 + 4, (uint8_t*)&bitmask, 1);
        
        uint8_t status   = entryData.status;
        memcpy(data + offset + 1 + 4 + 4 + 1, (uint8_t*)&status, 1);
        
        *length = offset + HAP_FAKEGATO_DATA_LENGTH;    
        offset  = offset + HAP_FAKEGATO_DATA_LENGTH;

        // _noOfEntriesSent++;            
        if ( (tmpRequestedEntry + 1 >= _idxWrite )  && ( _rolledOver == false) ){
            _transfer = false;    

            LogE("ERROR: Fakegato Switch could not send the requested entry. The requested index does not exist!", true);                          
            LogE("   - tmpRequestedEntry=" + String(tmpRequestedEntry), true);
            LogE("   - _requestedEntry=" + String(_requestedEntry), true);
            LogE("   - _idxWrite=" + String(_idxWrite), true);
            LogE("   - _rolledOver=" + String(_rolledOver), true);                          
            break;
        }

        
        uint32_t tsOld = entryData.timestamp;
        
        tmpRequestedEntry = incrementIndex(tmpRequestedEntry);
        entryData = (*_vectorBuffer)[tmpRequestedEntry];
        
        if ( _rolledOver == true) { 
            if (tsOld > entryData.timestamp) {
                _transfer = false;  
                LogE("ERROR: Fakegato Switch could not send the requested entry. The requested index does not exist!", true);                          
                LogE("   - tmpRequestedEntry=" + String(tmpRequestedEntry), true);
                LogE("   - _requestedEntry=" + String(_requestedEntry), true);
                LogE("   - _idxWrite=" + String(_idxWrite), true);
                LogE("   - _rolledOver=" + String(_rolledOver), true);                            
                break;
            }
        }
#if HAP_DEBUG_FAKEGATO    
        Serial.print("2 _idxWrite: ");
        Serial.println(_idxWrite);  

        Serial.print("2 _idxRead: ");
        Serial.println(_idxRead);  

        Serial.print("2 tmpRequestedEntry: ");
        Serial.println(tmpRequestedEntry);

        Serial.println("=====================================================================");
#endif
    }             
}
