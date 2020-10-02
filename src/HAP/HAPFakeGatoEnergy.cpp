//
// HAPFakeGatoWeatther.cpp
// Homekit
//
//  Created on: 29.08.2019
//      Author: michael
//
#include "HAPFakeGatoEnergy.hpp"
#include "HAPServer.hpp"
#include "HAPTLV8.hpp"


#define HAP_FAKEGATO_SIGNATURE_LENGTH    5      // number of 16 bits word of the following "signature" portion
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

    _periodicUpdates = true;   


    _schedule = nullptr;    

    begin(); 
} 

HAPFakeGatoEnergy::~HAPFakeGatoEnergy(){
    
    if (_vectorBuffer != nullptr){
        _vectorBuffer->clear();
        delete _vectorBuffer;
    }

    if (_schedule != nullptr) {
        _schedule->clear();
        delete _schedule;
    }
}


void HAPFakeGatoEnergy::begin(){
    
    if (_vectorBuffer == nullptr) {
        _vectorBuffer = new std::vector<HAPFakeGatoEnergyData>(HAP_FAKEGATO_BUFFER_SIZE);
    }

    if (_schedule == nullptr) {
        _schedule = new HAPFakeGatoScheduleEnergy();
    }

    _schedule->setCallbackGetReftime(std::bind(&HAPFakeGatoEnergy::getTimestampRefTime, this));
    _schedule->setCallbackGetTimestampLastEntry(std::bind(&HAPFakeGatoEnergy::getTimestampLastEntry, this));
    _schedule->setCallbackGetRolledOverIndex(std::bind(&HAPFakeGatoEnergy::getRolledOverIndex, this));
    // _configReadCharacteristics->setValue(_schedule->buildScheduleString());

}

int HAPFakeGatoEnergy::signatureLength(){
    return HAP_FAKEGATO_SIGNATURE_LENGTH;
}

void HAPFakeGatoEnergy::getSignature(uint8_t* signature){
    // ui16_to_ui8 s1, s2, s3, s4;    
    // s1.ui16 = __builtin_bswap16(0x0102);
    // s2.ui16 = __builtin_bswap16(0x0202);
    // s3.ui16 = __builtin_bswap16(0x0702);
    // s4.ui16 = __builtin_bswap16(0x0F03);

    // memcpy(signature, s1.ui8, 2);
    // memcpy(signature + 2, s2.ui8, 2);
    // memcpy(signature + 2 + 2, s3.ui8, 2);
    // memcpy(signature + 2 + 2 + 2, s4.ui8, 2);
    // // *length = signatureLength();    

    signature[0] = (uint8_t)HAPFakeGatoSignature_PowerWatt;
    signature[1] = 2;

    signature[2] = (uint8_t)HAPFakeGatoSignature_PowerVoltage;
    signature[3] = 2;

    signature[4] = (uint8_t)HAPFakeGatoSignature_PowerCurrent;
    signature[5] = 2;

    signature[6] = (uint8_t)HAPFakeGatoSignature_Power10thWh;
    signature[7] = 2;

    signature[8] = (uint8_t)HAPFakeGatoSignature_PowerOnOff;
    signature[9] = 1;
}


bool HAPFakeGatoEnergy::addEntry(String powerWatt, String powerVoltage, String powerCurrent, String stringPower10th, String status){        

    LogD(HAPServer::timeString() + " " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " [   ] " + "Adding entry for " + _name + " [size=" + String(size()) + "]: power10th=" + stringPower10th, true);
    
    uint16_t valuePowerWatt     = (uint16_t) powerWatt.toInt()          * 10;
    uint16_t valuePowerVoltage  = (uint16_t) powerVoltage.toInt()       * 10;
    uint16_t valuePowerCurrent  = (uint16_t) powerCurrent.toInt()       * 10;
    uint16_t valuePower10th     = (uint16_t) stringPower10th.toInt()    * 10;
    bool state                  = status == "1" ? true : false;

    HAPFakeGatoEnergyData data = (HAPFakeGatoEnergyData){
        HAPServer::timestamp(),
        valuePowerWatt,
        valuePowerVoltage,
        valuePowerCurrent,
        valuePower10th,
        state       
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
    

    _timestampLastEntry = data.timestamp;
    

    (*_vectorBuffer)[_idxWrite] = data;
    
    // increment write index
    _idxWrite = incrementIndex(_idxWrite);

    
    if (_memoryUsed == _vectorBuffer->size()){
        //  Serial.println("Rolled over");

        _rolledOver = true;  
        _idxRead = incrementIndex(_idxWrite);      
    }

    updateS2R1Value();        
    
    // Update schedule here for last act. 
    _schedule->buildScheduleString();
    
    return !_rolledOver;
}


void HAPFakeGatoEnergy::getData(const size_t count, uint8_t *data, size_t* length, uint16_t offset){
#if HAP_DEBUG_FAKEGATO      
    LogD(HAPServer::timeString() + " " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " [   ] " + "Get fakegato data for " + _name + " ..." , true);
#endif
    uint32_t tmpRequestedEntry = (_requestedEntry - 1) % HAP_FAKEGATO_BUFFER_SIZE;

    if ( (tmpRequestedEntry >= _idxWrite) && ( _rolledOver == false) ){
        _transfer = false;
        LogE("ERROR 1: Fakegato Energy could not send the requested entry. The requested index does not exist!", true);                          
        LogE("   - tmpRequestedEntry=" + String(tmpRequestedEntry), true);
        LogE("   - _requestedEntry=" + String(_requestedEntry), true);
        LogE("   - _idxWrite=" + String(_idxWrite), true);
        LogE("   - _rolledOver=" + String(_rolledOver), true);
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

        // PowerWatt
        memset(data + offset + 1 + 4 + 4 + 1, 0x00, 2);
        
        // PowerVoltage
        memset(data + offset + 1 + 4 + 4 + 1 + 2, 0x00, 2);
        
        // PowerCurrent
        memset(data + offset + 1 + 4 + 4 + 1 + 2 + 2, 0x00, 2);

        // Power10th
        ui16_to_ui8 power;
        power.ui16       = entryData.power10th;
        memcpy(data + offset + 1 + 4 + 4 + 1 + 2 + 2 + 2, power.ui8, 2);

        // PowerOnOff
        uint8_t status   = entryData.status;
        memcpy(data + offset + 1 + 4 + 4 + 2 + 2 + 2 + 2 + 1, (uint8_t*)&status, 1);


        // 2x unknown
        // memset(data + offset + 1 + 4 + 4 + 1 + 2 + 2 + 2 + 2, 0x00, 2);
        // memset(data + offset + 1 + 4 + 4 + 1 + 2 + 2 + 2 + 2 + 2, 0x00, 2);
        
        *length = offset + HAP_FAKEGATO_DATA_LENGTH;    
        offset  = offset + HAP_FAKEGATO_DATA_LENGTH;

        if ( (tmpRequestedEntry >= (_idxWrite - 1))  && ( _rolledOver == false) ){
            _transfer = false;    

            //Serial.println(">>>>>>>>>>>> ABORT 1");                                
            LogE("ERROR 2: Fakegato Energy could not send the requested entry. The requested index does not exist!", true);                          
            LogE("   - tmpRequestedEntry=" + String(tmpRequestedEntry), true);
            LogE("   - _requestedEntry=" + String(_requestedEntry), true);
            LogE("   - _idxWrite=" + String(_idxWrite), true);
            LogE("   - _rolledOver=" + String(_rolledOver), true);
            break;
        }
        
        uint32_t tsOld = entryData.timestamp;
        
        tmpRequestedEntry = incrementIndex(tmpRequestedEntry);
        entryData = (*_vectorBuffer)[tmpRequestedEntry];
        
        // if ( _rolledOver == true) { 
        //     if (tsOld > entryData.timestamp) {
        //         _transfer = false;  
        //         LogE("ERROR 3: Fakegato Energy could not send the requested entry. The requested index does not exist!", true);                          
        //         LogE("   - tmpRequestedEntry=" + String(tmpRequestedEntry), true);
        //         LogE("   - _requestedEntry=" + String(_requestedEntry), true);
        //         LogE("   - _idxWrite=" + String(_idxWrite), true);
        //         LogE("   - _rolledOver=" + String(_rolledOver), true);
        //         break;
        //     }
        // }      
    }         
}

void HAPFakeGatoEnergy::scheduleRead(String oldValue, String newValue){
    LogE(HAPServer::timeString() + " " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " [   ] " + "Schedule Read " + _name + " ..." , true);
}

void HAPFakeGatoEnergy::scheduleWrite(String oldValue, String newValue){
    LogE(HAPServer::timeString() + " " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " [   ] " + "Schedule Write " + _name + " ..." , true);

    size_t outputLength = 0;        
    mbedtls_base64_decode(NULL, NULL, &outputLength, (const uint8_t*)newValue.c_str(), newValue.length());
    uint8_t decoded[outputLength];

    mbedtls_base64_decode(decoded, sizeof(decoded), &outputLength, (const uint8_t*)newValue.c_str(), newValue.length());    

    HAPHelper::array_print("decoded", decoded, outputLength);    

    TLV8 tlv;
    tlv.encode(decoded, outputLength);

    if (tlv.hasType(HAP_FAKEGATO_SCHEDULE_TYPE_COMMAND_TOGGLE_SCHEDULE)){        
        TLV8Entry* tlvEntry = tlv.getType(HAP_FAKEGATO_SCHEDULE_TYPE_COMMAND_TOGGLE_SCHEDULE);  
        _schedule->enable(_schedule->decodeToggleOnOff(tlvEntry->value));
    } 

    if (tlv.hasType(HAP_FAKEGATO_SCHEDULE_TYPE_COMMAND_STATUS_LED)){        
        TLV8Entry* tlvEntry = tlv.getType(HAP_FAKEGATO_SCHEDULE_TYPE_COMMAND_STATUS_LED);  
        _schedule->setStatusLED(tlvEntry->value[0]);
    } 

    if (tlv.hasType(HAP_FAKEGATO_SCHEDULE_TYPE_DAYS)){        
        TLV8Entry* tlvEntry = tlv.getType(HAP_FAKEGATO_SCHEDULE_TYPE_DAYS);  
        _schedule->decodeDays(tlvEntry->value);
    }
    
    if (tlv.hasType(HAP_FAKEGATO_SCHEDULE_TYPE_PROGRAMS)){        
        TLV8Entry* tlvEntry = tlv.getType(HAP_FAKEGATO_SCHEDULE_TYPE_PROGRAMS);  
        _schedule->decodePrograms(tlvEntry->value);
    }

    _configReadCharacteristics->setValue(_schedule->buildScheduleString());
}

void HAPFakeGatoEnergy::beginSchedule(){
    _configReadCharacteristics->setValue(_schedule->buildScheduleString());    
}

void HAPFakeGatoEnergy::setSerialNumber(String serialNumber) {
    _serialNumber = serialNumber;
    _schedule->setSerialNumber(serialNumber);
}


void HAPFakeGatoEnergy::handle(bool forced){        
    if ( shouldHandle() || forced ){       
        // This line could cause a crash 
        // LogD(HAPServer::timeString() + " " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " [   ] " + "Handle fakegato ", true);         
        
        if (_periodicUpdates) {
            if (_callbackAddEntry != NULL){
                bool overwritten = !_callbackAddEntry();  

                // ToDo: Persist history ??
                if (overwritten) {
                    LogW("A fakegato history entry was overwritten!", true);
                }                      
            }   
        }                                                            
    }
    
    _schedule->handle();
}