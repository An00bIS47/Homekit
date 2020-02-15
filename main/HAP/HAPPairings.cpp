//
// HAPPairings.cpp
// Homekit
//
//  Created on: 13.04.2018
//      Author: michael
//

#include "HAPPairings.hpp"
#include <algorithm>
#include "HAPHelper.hpp"
#include "HAPLogger.hpp"

HAPPairings::HAPPairings(){

}


bool HAPPairings::begin(){
	if (!EEPROM.begin(HAP_EEPROM_SIZE)) {
		LogE("[ERROR] Failed to initialise EEPROM", true); 
		return false;
	}    
	return true;
}


bool HAPPairings::save(){
	
	for (int i=0; i < _pairings.size(); i++){
		

#if 0	
		Serial.println("\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>: " + String(HAP_EEPROM_OFFSET_PAIRINGS + ( i * sizeof(HAPPairing) )));	
		Serial.println("id: ");
		HAPHelper::arrayPrint(_pairings[i].id, HAP_PAIRINGS_ID_LENGTH);

		Serial.println("key: ");
		HAPHelper::arrayPrint(_pairings[i].key, HAP_PAIRINGS_LTPK_LENGTH);

		Serial.println("===============================================");
#endif

		size_t written = EEPROM.writeBytes( HAP_EEPROM_OFFSET_PAIRINGS + ( i * sizeof(HAPPairing) ), &_pairings[i], sizeof(HAPPairing));
		if ( written != sizeof(HAPPairing) ) {
			LogE("[ERROR] Failed to save pairing to EEPROM!", true);
			return false;
		}
	}

	if (!EEPROM.commit()){
		LogE("Failed to commit EEPROM!", true);
		return false;
	}
	return true;
}

bool HAPPairings::load(){
	
	for (int i=0; i < HAP_PAIRINGS_MAX; i++){
		HAPPairing tmp;

		size_t read = EEPROM.readBytes( HAP_EEPROM_OFFSET_PAIRINGS + ( i * sizeof(HAPPairing) ), &tmp, sizeof(HAPPairing));

		if ( read != sizeof(HAPPairing) ) {
			LogE("[ERROR] Failed to load pairings from EEPROM!", true);
			return false;
		}

#if 0	
		Serial.println("\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>: " + String(HAP_EEPROM_OFFSET_PAIRINGS + ( i * sizeof(HAPPairing) )));	
		Serial.println("id: ");
		HAPHelper::arrayPrint(tmp.id, HAP_PAIRINGS_ID_LENGTH);

		Serial.println("key: ");
		HAPHelper::arrayPrint(tmp.key, HAP_PAIRINGS_LTPK_LENGTH);

		Serial.println("===============================================");
#endif

		if (tmp.id[0] == 0x00 && tmp.id[1] == 0x00 && tmp.key[0] == 0x00 && tmp.key[1] == 0x00 ){
			//do nothing			
		} else if (tmp.id[0] == 0xFF && tmp.id[1] == 0xFF && tmp.key[0] == 0xFF && tmp.key[1] == 0xFF ){
		} else {
			_pairings.push_back(tmp);
		}
		
	}	
	return true;
}


void HAPPairings::resetEEPROM(){
	for (int i=0 ; i < HAP_EEPROM_SIZE; i++){
		EEPROM.write(i, 0x00);
	}
	if (!EEPROM.commit()){
		LogE("Failed to commit EEPROM!", true);
	}
}



void HAPPairings::add(uint8_t* id, uint8_t* key){    

	struct HAPPairing item;
	memcpy(item.id, id, HAP_PAIRINGS_ID_LENGTH);
	memcpy(item.key, key, HAP_PAIRINGS_LTPK_LENGTH);

#if HAP_DEBUG	
	LogD("### Save pairing:", true);

	LogD("### - ID: ", false);
	HAPHelper::arrayPrint(item.id, HAP_PAIRINGS_ID_LENGTH);

	LogD("### - KEY: ", false);
	HAPHelper::arrayPrint(item.key, HAP_PAIRINGS_LTPK_LENGTH);
#endif

	_pairings.push_back(item);
}

/*
struct HAPPairing HAPPairings::get(uint8_t* id) {
	for(size_t i = 0; i < _pairings.size(); i++)
	{
		if (memcmp(_pairings[i].id, id, HAP_PAIRINGS_ID_SIZE) ) {
			return _pairings[i];
		}
	}
	return struct HAPPairing;
}
*/

int HAPPairings::getKey(const uint8_t* id, uint8_t* outkey) {
	LogD("Get iOS DeviceID LTPK: ", true);
	for(size_t i = 0; i < _pairings.size(); i++) {
		struct HAPPairing item = _pairings[i];

		// LogD("### - ID: ", false);
		// HAPHelper::arrayPrint(item.id, HAP_PAIRINGS_ID_SIZE);

		if ( memcmp(item.id, id, HAP_PAIRINGS_ID_LENGTH) == 0) {
		
			// LogD("### - KEY found: ", false);
			// HAPHelper::arrayPrint(item.key, HAP_PAIRINGS_LTPK_SIZE);
			
			if (outkey != NULL)
				memcpy(outkey, item.key, HAP_PAIRINGS_LTPK_LENGTH);
			return 0;
		}

	}
	return -1;
}


uint8_t HAPPairings::size(){
	return _pairings.size();
}

void HAPPairings::print(){
	for (int i=0; i < _pairings.size(); i++){
		Serial.println("id: ");
		HAPHelper::arrayPrint(_pairings[i].id, HAP_PAIRINGS_ID_LENGTH);

		Serial.println("key: ");
		HAPHelper::arrayPrint(_pairings[i].key, HAP_PAIRINGS_LTPK_LENGTH);
	}
}

bool HAPPairings::loadKeys(uint8_t *ltpk, uint8_t *ltsk){
	
	HAPKeys k;
	size_t read = EEPROM.readBytes( 0, &k, sizeof(HAPKeys));

	if ( read != sizeof(HAPKeys) ) {
		LogE("[ERROR] Failed to load keys from EEPROM!", true);
		return false;
	}

	memcpy(ltpk, k.ltpk, HAP_PAIRINGS_LTPK_LENGTH);
	memcpy(ltsk, k.ltsk, HAP_PAIRINGS_LTSK_LENGTH);

#if HAP_DEBUG
	LogD("Loaded LTPK from EEPROM: ", true);
	HAPHelper::arrayPrint(ltpk, HAP_PAIRINGS_LTPK_LENGTH);

	LogD("Loaded LTSK from EEPROM: ", true);
	HAPHelper::arrayPrint(ltsk, HAP_PAIRINGS_LTSK_LENGTH);
#endif
	return true;
}

bool HAPPairings::saveKeys(uint8_t *ltpk, uint8_t *ltsk){

	HAPKeys k;
	memcpy(k.ltpk, ltpk, HAP_PAIRINGS_LTPK_LENGTH);
	memcpy(k.ltsk, ltsk, HAP_PAIRINGS_LTSK_LENGTH);

#if 0
	LogD("Saving LTPK to EEPROM: ", true);
	HAPHelper::arrayPrint(ltpk, HAP_PAIRINGS_LTPK_LENGTH);

	LogD("Saving LTSK to EEPROM: ", true);
	HAPHelper::arrayPrint(ltsk, HAP_PAIRINGS_LTSK_LENGTH);
#endif

	size_t written = EEPROM.writeBytes( 0, &k, sizeof(HAPKeys));
	// Serial.println(written);

	if ( written != sizeof(HAPKeys) ) {
		LogE("[ERROR] Failed to save LTPK to EEPROM!", true);
		return false;
	}

	if (!EEPROM.commit()){
		LogE("Failed to commit EEPROM!", true);
		return false;
	}
	return true;
}

bool HAPPairings::removePairing(uint8_t *id){

	const auto orig_size = _pairings.size();

	for (int i=0; i < HAP_PAIRINGS_MAX; i++){
		struct HAPPairing item = _pairings[i];		
		if ( memcmp(item.id, id, HAP_PAIRINGS_ID_LENGTH) == 0) {

#if HAP_DEBUG
			LogD("Removing pairing for ", false);
			HAPHelper::arrayPrint(_pairings[i].id, HAP_PAIRINGS_ID_LENGTH);
#endif
			_pairings.erase(_pairings.begin() + i);
			
			if (_pairings.size() == orig_size) {
				return false;
			}

			if (_pairings.size() == 0) {
#if HAP_DEBUG				
				resetEEPROM();
				LogD("Removing long term keys", true);
#endif				
			} else {
				save();
			}

			break;
		}
	}

	return true;
}