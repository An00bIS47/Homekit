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
    _shouldSave = false;
    _callbackSaveConfig = nullptr;

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

    // 0b02 0c02 0d02 0702 0e01
	//	|	  |	   |    |    +-> Power on/ff
    //	|	  |	   |    +-> Power 10th Watt
    //	|	  |	   +-> PowerCurrent	
	//  |	  +-> PowerVoltage
	//  +-> PowerWatt
	// 
	// bitmask 0x1F => all			= 11111
	// bitmask 0x01 => watt			= 00001
	// bitmask 0x02 => voltage		= 00010
	// bitmask 0x04 => current		= 00100
    // bitmask 0x08 => 10th w		= 01000
    // bitmask 0x10 => on/off		= 10000

    // bitmask for all: 11111 = 0x1F
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


bool HAPFakeGatoEnergy::addEntry(uint8_t bitmask, String powerWatt, String powerVoltage, String powerCurrent, String stringPower10th, String status){        

    LogD(HAPServer::timeString() + " " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " [   ] " + "Adding entry for " + _name + " [size=" + String(size()) + "]: power10th=" + stringPower10th, true);
    
    uint16_t valuePowerWatt     = (uint16_t) powerWatt.toInt()          * 10;
    uint16_t valuePowerVoltage  = (uint16_t) powerVoltage.toInt()       * 10;
    uint16_t valuePowerCurrent  = (uint16_t) powerCurrent.toInt()       * 10;
    uint16_t valuePower10th     = (uint16_t) stringPower10th.toInt()    * 10;
    bool state                  = status == "1" ? true : false;

    HAPFakeGatoEnergyData data = (HAPFakeGatoEnergyData){
        HAPServer::timestamp(),
        bitmask,
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


#if 1


        uint8_t size = 10;
        uint8_t currentOffset = 0;


        // bitmask 0x1F => all			= 11111
        // bitmask 0x01 => watt			= 00001
        // bitmask 0x02 => voltage		= 00010
        // bitmask 0x04 => current		= 00100
        // bitmask 0x08 => 10th w		= 01000
        // bitmask 0x10 => on/off		= 10000
        
        size += (((entryData.bitmask & 0x10) >> 4) * 1);  
        size += (((entryData.bitmask & 0x08) >> 3) * 2);  
        size += (((entryData.bitmask & 0x04) >> 2) * 2);  
        size += (((entryData.bitmask & 0x02) >> 1) * 2);  
        size +=  ((entryData.bitmask & 0x01) * 2); 

        // size
        memcpy(data + offset + currentOffset, (uint8_t *)&size, 1);
        currentOffset += 1;

        // requestedEntry
        ui32_to_ui8 eC;
        eC.ui32 = _requestedEntry++;
        memcpy(data + offset + currentOffset, eC.ui8, 4);
        currentOffset += 4;

        // Timeestamp
        ui32_to_ui8 secs;
        secs.ui32 = entryData.timestamp - _refTime;
        memcpy(data + offset + currentOffset, secs.ui8, 4);
        currentOffset += 4;

        // bitmask
        memcpy(data + offset + currentOffset, (uint8_t*)&entryData.bitmask, 1);
        currentOffset += 1; 

        // watt
        if ((entryData.bitmask & 0x01) == 1) {            
            ui16_to_ui8 powerWatt;
            powerWatt.ui16 = entryData.powerWatt;
            memcpy(data + offset + currentOffset, powerWatt.ui8, 2);
            currentOffset += 2;
        } 

        // voltage
        if (((entryData.bitmask & 0x02) >> 1) == 1){
            ui16_to_ui8 powerVoltage;
            powerVoltage.ui16 = entryData.powerVoltage;
            memcpy(data + offset + currentOffset, powerVoltage.ui8, 2);
            currentOffset += 2;
        }

        // current
        if (((entryData.bitmask & 0x04) >> 2) == 1) {            
            ui16_to_ui8 powerCurrent;
            powerCurrent.ui16 = entryData.powerCurrent;
            memcpy(data + offset + currentOffset, powerCurrent.ui8, 2);        
            currentOffset += 2;
        } 

        // 10th watt
        if (((entryData.bitmask & 0x08) >> 3) == 1) {            
            ui16_to_ui8 power10th;
            power10th.ui16 = entryData.power10th;
            memcpy(data + offset + currentOffset, power10th.ui8, 2);        
            currentOffset += 2;
        } 

        // on/off
        if (((entryData.bitmask & 0x10) >> 4) == 1) {            
            memcpy(data + offset + currentOffset, (uint8_t*)&entryData.status, 1);        
            currentOffset += 1;
        } 


        offset  += currentOffset;
        *length = offset; 

#if HAP_DEBUG_FAKEGATO   
        HAPHelper::array_print("Fakegato data", data + offset, currentOffset);
#endif

#else
        uint8_t size = HAP_FAKEGATO_DATA_LENGTH;
        memcpy(data + offset, (uint8_t *)&size, 1);

        ui32_to_ui8 eC;
        eC.ui32 = _requestedEntry++;
        memcpy(data + offset + 1, eC.ui8, 4);

        ui32_to_ui8 secs;
        secs.ui32 = entryData.timestamp - _refTime;
        memcpy(data + offset + 1 + 4, secs.ui8, 4);
        
        uint8_t type = HAP_FAKEGATO_TYPE_ENERGY;
        memcpy(data + offset + 1 + 4 + 4, (uint8_t*)&type, 1);
        
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
      
        *length = offset + HAP_FAKEGATO_DATA_LENGTH;    
        offset  = offset + HAP_FAKEGATO_DATA_LENGTH;
#endif


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
        
        if ( _rolledOver == true) { 
            if (tsOld > entryData.timestamp) {
                _transfer = false;  
                LogE("ERROR 3: Fakegato Energy could not send the requested entry. The requested index does not exist!", true);                          
                LogE("   - tmpRequestedEntry=" + String(tmpRequestedEntry), true);
                LogE("   - _requestedEntry=" + String(_requestedEntry), true);
                LogE("   - _idxWrite=" + String(_idxWrite), true);
                LogE("   - _rolledOver=" + String(_rolledOver), true);
                break;
            }
        }      
    }         
}

void HAPFakeGatoEnergy::scheduleRead(String oldValue, String newValue){
    LogD(HAPServer::timeString() + " " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " [   ] " + "Schedule Read " + _name + " ..." , true);
}

void HAPFakeGatoEnergy::scheduleWrite(String oldValue, String newValue){
    LogD(HAPServer::timeString() + " " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " [   ] " + "Schedule Write " + _name + " ..." , true);

    size_t outputLength = 0;        
    mbedtls_base64_decode(NULL, NULL, &outputLength, (const uint8_t*)newValue.c_str(), newValue.length());
    uint8_t decoded[outputLength];

    mbedtls_base64_decode(decoded, sizeof(decoded), &outputLength, (const uint8_t*)newValue.c_str(), newValue.length());    

#if HAP_DEBUG_FAKEGATO_SCHEDULE	
    HAPHelper::array_print("decoded", decoded, outputLength);    
#endif

    TLV8 tlv;
    tlv.encode(decoded, outputLength);

    if (tlv.hasType(HAP_FAKEGATO_SCHEDULE_TYPE_COMMAND_TOGGLE_SCHEDULE)){        
        TLV8Entry* tlvEntry = tlv.getType(HAP_FAKEGATO_SCHEDULE_TYPE_COMMAND_TOGGLE_SCHEDULE);                        
        _schedule->enable(_schedule->decodeToggleOnOff(tlvEntry->value));
        _shouldSave = true; 
    } 

    if (tlv.hasType(HAP_FAKEGATO_SCHEDULE_TYPE_COMMAND_STATUS_LED)){        
        TLV8Entry* tlvEntry = tlv.getType(HAP_FAKEGATO_SCHEDULE_TYPE_COMMAND_STATUS_LED);  
        _schedule->setStatusLED(tlvEntry->value[0]);
    } 

    if (tlv.hasType(HAP_FAKEGATO_SCHEDULE_TYPE_DAYS)){        
        TLV8Entry* tlvEntry = tlv.getType(HAP_FAKEGATO_SCHEDULE_TYPE_DAYS);  
        _schedule->decodeDays(tlvEntry->value);
        _shouldSave = true; 
    }
    
    if (tlv.hasType(HAP_FAKEGATO_SCHEDULE_TYPE_PROGRAMS)){        
        TLV8Entry* tlvEntry = tlv.getType(HAP_FAKEGATO_SCHEDULE_TYPE_PROGRAMS);  
        _schedule->decodePrograms(tlvEntry->value);
        _shouldSave = true; 
    }

    _configReadCharacteristics->setValue(_schedule->buildScheduleString());

    if (_shouldSave){
        _callbackSaveConfig();
    }

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

JsonObject HAPFakeGatoEnergy::scheduleToJson(){
    return _schedule->toJson();
}

void HAPFakeGatoEnergy::scheduleFromJson(JsonObject &root){
    _schedule->fromJson(root);
}