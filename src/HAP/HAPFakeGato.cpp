// 
// HAPFakeGato.cpp
// Homekit
//
//  Created on: 29.08.2019
//      Author: michael
//
#include "HAPFakeGato.hpp"
#include "HAPServer.hpp"
#include <base64.h>
#include "HAPHelper.hpp"
#include "HAPLogger.hpp"

HAPFakeGato::HAPFakeGato() {
    
    _s2r1Characteristics = nullptr;
    _s2r2Characteristics = nullptr;
    _s2w1Characteristics = nullptr;
    _s2w2Characteristics = nullptr;
    
    _type = HAPFakeGatoType_weather;
    _isEnabled = true;
    _refTime = 0;    

    
    _rolledOver = false;

    _memoryUsed = 0;    // last physical memory position occupied

    _idxWrite = 0;      // Write index
    _idxRead = 0;       // Read index

    _requestedEntry = 0;


    _ptrTimestampLastEntry = nullptr;


    _transfer = false;
    _interval = 0; 
    _previousMillis = 0;
}


void HAPFakeGato::registerFakeGatoService(HAPAccessory* accessory, String name){
        
    HAPService* fgService = new HAPService(HAP_SERVICE_FAKEGATO_HISTORY);    
    
    stringCharacteristics *accNameCha = new stringCharacteristics(HAP_CHARACTERISTIC_NAME, permission_read, 32);
    accNameCha->setValue("EVE " + name + " History");
    accessory->addCharacteristics(fgService, accNameCha);

    // S2R1 Char
    _s2r1Characteristics = new dataCharacteristics(HAP_CHARACTERISTIC_FAKEGATO_HISTORY_STATUS, permission_read|permission_notify|permission_hidden, 128);    
    auto callbackS2R1 = std::bind(&HAPFakeGato::setS2R1Characteristics, this, std::placeholders::_1, std::placeholders::_2);            
    _s2r1Characteristics->setValue((char*)NULL);    
    _s2r1Characteristics->valueChangeFunctionCall = callbackS2R1;
    accessory->addCharacteristics(fgService, _s2r1Characteristics);

    // S2R2 Char
    _s2r2Characteristics = new dataCharacteristics(HAP_CHARACTERISTIC_FAKEGATO_HISTORY_ENTRIES, permission_read|permission_notify|permission_hidden, HAP_FAKEGATO_CHUNK_BUFFER_SIZE);
    auto callbackS2R2 = std::bind(&HAPFakeGato::setS2R2Characteristics, this, std::placeholders::_1, std::placeholders::_2);            
    _s2r2Characteristics->setValue((char*)NULL);
    _s2r2Characteristics->valueChangeFunctionCall = callbackS2R2;    

    auto callbackGetS2R2 = std::bind(&HAPFakeGato::getS2R2Callback, this);            
    _s2r2Characteristics->valueGetCallback = callbackGetS2R2;

    accessory->addCharacteristics(fgService, _s2r2Characteristics);

    // S2W1 Char
    _s2w1Characteristics = new dataCharacteristics(HAP_CHARACTERISTIC_FAKEGATO_HISTORY_REQUEST, permission_write|permission_hidden, 128);
    auto callbackS2W1 = std::bind(&HAPFakeGato::setS2W1Characteristics, this, std::placeholders::_1, std::placeholders::_2);        
    _s2w1Characteristics->valueChangeFunctionCall = callbackS2W1;
    accessory->addCharacteristics(fgService, _s2w1Characteristics);

    // S2W2 Char
    _s2w2Characteristics = new dataCharacteristics(HAP_CHARACTERISTIC_FAKEGATO_SET_TIME, permission_write|permission_hidden, 128);
    auto callbackS2W2 = std::bind(&HAPFakeGato::setS2W2Characteristics, this, std::placeholders::_1, std::placeholders::_2);        
    _s2w2Characteristics->valueChangeFunctionCall = callbackS2W2;
    accessory->addCharacteristics(fgService, _s2w2Characteristics);
    
    accessory->addService(fgService);   

    begin();   

    enable(true);
}


void HAPFakeGato::handle(bool forced){
    if ( shouldHandle() || forced ){       
        // This line could cause a crash 
        // LogD(HAPServer::timeString() + " " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " [   ] " + "Handle fakegato ", true);         
        if (_callbackAddEntry != NULL){
            bool overwritten = !_callbackAddEntry();  

            // ToDo: Persist history ??
            if (overwritten) {
                // LogV("A fakegato history entry was overwritten!", true);
            }                      
        }                                            
    }
    // ToDo: Needed ?
    if (_transfer == true){

    }
}

bool HAPFakeGato::shouldHandle(){

    if (_isEnabled) {
        unsigned long currentMillis = millis(); // grab current time

        if ((unsigned long)(currentMillis - _previousMillis) >= _interval) {

            // save the last time you blinked the LED
            _previousMillis = currentMillis;

            //LogD("Handle plugin: " + String(_name), true);			
            return true;			
        }
    }

    return false;
}


/*
     * E863F116-079E-48FF-8F27-9C2605A29F52 (tentative)

    This read-only characteristic is used by the accessory to signal how many entries are in the log (and other infos). Comparing this characteristics over different type of accessory, it was possible to obtain the following partial decoding. Data is composed by a fixed size portion (12 bytes) with info about time, 1 byte indicating the length of the following variable length portion with accessory "signature" and finally a fixed length portion with info about memory status.

    4 bytes: Actual time, in seconds from last time update
    4 bytes: negative offset of reference time
    4 bytes: reference time/last Accessory time update (taken from E863F117-079E-48FF-8F27-9C2605A29F52)
    1 byte: number of 16 bits word of the following "signature" portion
    2-12 bytes: variable length "signature"
    2 bytes: last physical memory position occupied (used by Eve.app to understand how many transfers are needed). If set to an address lower than the last successfully uploaded entry, forces Eve.app to start from the beginning of the memory, asking address 00 in E863F11C. Accessory answers with entry 01. Once the memory is fully written and memory overwriting is necessary this field remains equal to history size.
    2 bytes: history size
    4 bytes: once memory rolling occurred it indicates the address of the oldest entry present in memory (if memory rolling did not occur yet, these bytes are at 0)
    4 bytes:??
    2 bytes:?? always 01ff or 0101

 * Energy (working, tweeking from Room): 
 * 4 16bits word, "0102 0202 0702 0f03", 
 * example of full data "58020000 00000000 cd8f0220 04 0102 0202 0702 0f03 0300 c00f 00000000 00000000 0101"
 * 
 **/
void HAPFakeGato::updateS2R1Value(){

    // "01010000 FF000000 3C0F0000 03 0102 0202 0302 1D00 F50F 00000000 00000000 01FF"

    int sigLength = signatureLength() * 2;    
    uint8_t signature[sigLength];        
    getSignature(signature);


    // uint32_t eveTime = (*_ptrTimestampLastEntry) - _refTime;  
    // Serial.println("Now: " + String(now));
    // Serial.print("_ptrTimestampLastEntry: ");
    // Serial.println(*_ptrTimestampLastEntry);

    // Serial.print("_refTime: ");
    // Serial.println(_refTime - FAKEGATO_EPOCH_OFFSET);

    // Serial.print("eveTime: ");  
    // Serial.println(eveTime);
    
    
    HAPFakeGatoInfoStart infoStart;
    infoStart.data.evetime              = (*_ptrTimestampLastEntry) - _refTime; // Time from last update in seconds
    infoStart.data.negativeOffset       = 0x00;                                 // Negativ offset of reference time
    infoStart.data.refTimeLastUpdate    = _refTime - FAKEGATO_EPOCH_OFFSET;     // reference time/last Accessory time update (taken from E863F117-079E-48FF-8F27-9C2605A29F52)
    infoStart.data.sigLength            = sigLength / 2;                        // signature length -> stefdude comment

    HAPFakeGatoInfoEnd infoEnd;
    infoEnd.data.usedMemory             = _memoryUsed;                                // last physical memory position occupied
    infoEnd.data.size                   = HAP_FAKEGATO_BUFFER_SIZE;              // history size
    
    if (_rolledOver){        
        // LogW(">>> Fakegato data rolled over", true);        
        infoEnd.data.rollOver           = _idxRead;                              // once memory rolling occurred it indicates the address of the oldest entry present in memory (if memory rolling did not occur yet, these bytes are at 0)
    } else {
        // (if memory rolling did not occur yet, these bytes are at 0)        
        infoEnd.data.rollOver           = 0x00;                                  
    }
    
    infoEnd.data.unknown                = 0x00;
    infoEnd.data.end                    = 0x0101;

    uint8_t data[13 + 14 + sigLength];
    memcpy(data, infoStart.bytes, 13);
    memcpy(data + 13, signature, sigLength);
    memcpy(data + 13 + sigLength, infoEnd.bytes, 14);

    // HAPHelper::array_print("S2R1", data, 13 + sigLength + 14);
    // int s = 13 + sigLength + 14;
    // for (int i=0; i < s; i++){
    //     Serial.printf("%02X", data[i]);
    // }
    // Serial.println("");


    String encoded = base64::encode(data, 13 + sigLength + 14);
    _s2r1Characteristics->setValue(encoded);    
}

void HAPFakeGato::updateS2R2Value(){
    
}

/*
    E863F117-079E-48FF-8F27-9C2605A29F52

    This read-only characteristics is used to send the actual log entry to Eve.app It is an array of logs with each entry having x bytes as determined by Byte 1. The first portion up to byte 10 included is common, the last portion contain the data specific for each accessory.

    Byte 1: Length (i.e. 14 for 20 Bytes)
    Bytes 2-5: entry counter
    Bytes 6-9: Seconds since reference time set with type 0x81 entry. In order to account for multiple iOS devices, the actual reference time and offset are apparently always reported also in E863F116
    Byte 10: Entry type
    0x81 Entry to set the reference timestamp, to be written on bytes 11-14. A negative offset can be written on bytes 6-9, and it's probably neeeded when the clock is updated (21 bytes in total).
    0x07 Entry for Eve Weather log.
    0x1f Entry for Eve Energy log (20 bytes in total)
    0x1e Entry for Eve Energy log (18 bytes in total - not working)
    0x0f Entry for Eve Room log.
    0x02 Entry for Eve Motion log.
    0x01 Entry for Eve Door log.
    0x1f Entry for Eve Thermo log.
    0x05 Entry for Eve Aqua, valve on, 13 bytes in total
    0x07 Entry for Eve Aqua, valve off + water usage, 21 bytes in total
*/
// 
// Prepare next values
// 
void HAPFakeGato::getS2R2Callback(){
#if HAP_DEBUG_FAKEGATO    
    LogD(HAPServer::timeString() + " " + "HAPFakeGato" + "->" + String(__FUNCTION__) + " [   ] " + "Callback S2R2: Set next history entry", true);      
#endif
    if (_transfer)
    {

        uint8_t data[HAP_FAKEGATO_CHUNK_BUFFER_SIZE];
        uint16_t offset = 0;
        uint8_t chunksize = HAP_FAKEGATO_CHUNK_SIZE;
        size_t len = 0;

        // if (_requestedEntry == 1) {
        //     getRefTime(data, &len, 0);
        //     offset += len;
        //     chunksize--;
        //     len = 0;
        // }                
        
        getData(chunksize, data, &len, offset);

#if HAP_DEBUG_FAKEGATO
        HAPHelper::array_print("data (next entry)", (uint8_t*)data, len);
#endif        
        _s2r2Characteristics->setValue(base64::encode(data, len));  

        // LogE("Number of entries sent (total): " + String(_noOfEntriesSent), true); 
    } else {
        _s2r2Characteristics->setValue("AA==");   
    }
}


// = CALLBACK
void HAPFakeGato::setS2R1Characteristics(String oldValue, String newValue){
    LogD(HAPServer::timeString() + " " + __CLASS_NAME__ + "->" + String(__FUNCTION__) + " [   ] " + "Getting S2R1 iid " + String(_s2r1Characteristics->iid) +  " oldValue: " + oldValue + " -> newValue: " + newValue, true);      
}

// = CALLBACK
void HAPFakeGato::setS2R2Characteristics( String oldValue, String newValue){
#if HAP_DEBUG_FAKEGATO
    LogD(HAPServer::timeString() + " " + __CLASS_NAME__ + "->" + String(__FUNCTION__) + " [   ] " + "Getting S2R2 iid " + String(_s2r2Characteristics->iid) +  " oldValue: " + oldValue + " -> newValue: " + newValue, true);      
#endif    
}

/**
     * @brief 
     * This write-only characteristic seem to control data flux from accessory to Eve.app. 
     * A typical value when reading from a fake Eve Weather accessory is 01140100 000000. Tentative decoding:

    byte 1: ??
    byte 2: ??
    byte 3-6: Requested memory entry, based on the last entry that Eve.app downloaded. 
            If set to 0000, asks the accessory the start restart from the beginning of the memory
    byte 7-8: ??
    * @param oldValue 
    * @param newValue 
 **/
void HAPFakeGato::setS2W1Characteristics(String oldValue, String newValue){
    LogD(HAPServer::timeString() + " " + "HAPFakeGato" + "->" + String(__FUNCTION__) + " [   ] " + "Setting S2W1 iid " + String(_s2w1Characteristics->iid) +  " oldValue: " + oldValue + " -> newValue: " + newValue, true);    
    
    size_t outputLength;
    uint8_t* decoded = base64_decode((const unsigned char *)newValue.c_str(), newValue.length(), &outputLength);
    
#if HAP_DEBUG_FAKEGATO    
    HAPHelper::array_print("S2W1", decoded, outputLength);
#endif

    ui32_to_ui8 tmp;

    int n = 0;
    for (unsigned idx = 2; idx < 6; idx++) {
        tmp.ui8[n++] = decoded[idx];
    }

    ui32_to_ui8 address;
    address.ui32 = __builtin_bswap32(tmp.ui32);

#if HAP_DEBUG_FAKEGATO    
    HAPHelper::array_print("S2W1 address",  address.ui8, 4);
#endif    
    _requestedEntry = tmp.ui32;

#if HAP_DEBUG_FAKEGATO
    Serial.print("_requestedEntry: ");
    Serial.print(_requestedEntry);
#endif

    // LogE("REQUESTED ENTRY: " + String(_requestedEntry) + " - ALREADY SENT: " +  String(_noOfEntriesSent), true);

    if (_requestedEntry == 0) {
        // Set reference time for EVE.app to S2R2        
#if HAP_DEBUG_FAKEGATO        
        Serial.println("_requestedEntry == 0 - > Set reftime from iOS device -> send entries");
#endif        

    } else if (_requestedEntry == 1) {
        // set reftime as first entry
#if HAP_DEBUG_FAKEGATO        
        Serial.println("_requestedEntry == 1 - > Set Reftime as 1st entry");
#endif           
        uint8_t data[22];
        size_t len = 0;
        getRefTime(data, &len, 0);
        _s2r2Characteristics->setValue(base64::encode(data, len));

    } else {        
        // set the data entries in S2R2 char for EVE.app   
#if HAP_DEBUG_FAKEGATO           
        Serial.println(" - > Set entries data");
#endif        
        _transfer = true;
        getS2R2Callback();   
    }

    free(decoded);
}

/**
 * @brief 
 *  In this write-only characteristics a time stamp is written by Eve.app every second if the accessory is selected in the app. Format from https://gist.github.com/gomfunkel/b1a046d729757120907c#gistcomment-1841206:

the current timestamp is in seconds since 1.1.2001 e.g.: 
written value: cf1b521d -> reverse 1d521bcf -> hex2dec 491920335 
Epoch Timestamp 1.1.2001 = 978307200; 978307200 + 491920335 = 1470227535 
= Wed, 03 Aug 2016 12:32:15 GMT = 3.8.2016, 14:32:15 GMT+2:00 DST (MEST)

It is probably used to set time/date of the accessory.
 * @param oldValue 
 * @param newValue 
 **/
void HAPFakeGato::setS2W2Characteristics(String oldValue, String newValue){
    LogD(HAPServer::timeString() + " " + "HAPFakeGato" + "->" + String(__FUNCTION__) + " [   ] " + "Setting S2W2 iid " + String(_s2w2Characteristics->iid) +  " oldValue: " + oldValue + " -> newValue: " + newValue, true);    
    
    // "SPMZIw=="
    size_t outputLength;
    uint8_t* decoded = base64_decode((const unsigned char *)newValue.c_str(), newValue.length(), &outputLength);

    // ToDo: Set current time from this? (Only if NTP is not connected!)
    HAPHelper::array_print("S2W2", decoded, outputLength);
    
    free(decoded);
}

void HAPFakeGato::getRefTime(uint8_t *data, size_t* length, const uint16_t offset){

#if HAP_DEBUG_FAKEGATO
    LogD(HAPServer::timeString() + " " + "HAPFakeGato" + "->" + String(__FUNCTION__) + " [   ] " + "Get ref time entry", true);    
#endif

    uint8_t size = 21;
    uint8_t typ = HAP_FAKEGATO_TYPE_REFTIME;
    memcpy(data + offset, (uint8_t *)&size, 1);


    ui32_to_ui8 eC;

    // ToDo: incrmeent function ??
    _idxRead = incrementIndex(_idxRead);
    eC.ui32 = _idxRead;
    memcpy(data + offset + 1, eC.ui8, 4);

    ui32_to_ui8 secs;
    secs.ui32 = 0;
    memcpy(data + offset + 1 + 4, secs.ui8, 4);
    memcpy(data + offset + 1 + 4 + 4, (uint8_t*)&typ, 1);


    ui32_to_ui8 refTime;
    refTime.ui32 = _refTime - FAKEGATO_EPOCH_OFFSET;

#if HAP_DEBUG_FAKEGATO
    LogD(">>>>> Fakegato RefTime: " + String( refTime.ui32 ), true);
#endif
    // memcpy(data + offset, common.bytes, 10);                
    memcpy(data + offset + 10, refTime.ui8, 4);
    memset(data + offset + 10 + 4, 0x00, 7);                
    *length = offset + 21;    
}
