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
	
	for (int i=0; i < pairings.size(); i++){
		

#if HAP_DEBUG_PAIRINGS
		LogD(String(HAP_EEPROM_OFFSET_PAIRINGS + ( i * sizeof(HAPPairing) )), true);			
		HAPHelper::array_print("ID:", pairings[i].id, HAP_PAIRINGS_ID_LENGTH);		
		HAPHelper::array_print("Key:", pairings[i].key, HAP_PAIRINGS_LTPK_LENGTH);
		LogD("isAdmin: " + String(pairings[i].isAdmin), true);
		LogD("===============================================", true);
#endif

		size_t written = EEPROM.writeBytes( HAP_EEPROM_OFFSET_PAIRINGS + ( i * sizeof(HAPPairing) ), &pairings[i], sizeof(HAPPairing));
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


#if HAP_DEBUG_PAIRINGS
		LogD(String(HAP_EEPROM_OFFSET_PAIRINGS + ( i * sizeof(HAPPairing) )), true);			
		HAPHelper::array_print("ID:", tmp.id, HAP_PAIRINGS_ID_LENGTH);		
		HAPHelper::array_print("Key:", tmp.key, HAP_PAIRINGS_LTPK_LENGTH);
		LogD("isAdmin: " + String(tmp.isAdmin), true);
		LogD("===============================================", true);
#endif



		if (tmp.id[0] == 0x00 && tmp.id[1] == 0x00 && tmp.key[0] == 0x00 && tmp.key[1] == 0x00 ){
			//do nothing			
		} else if (tmp.id[0] == 0xFF && tmp.id[1] == 0xFF && tmp.key[0] == 0xFF && tmp.key[1] == 0xFF ){
		} else {
			pairings.push_back(tmp);
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




bool HAPPairings::add(const uint8_t* id, const uint8_t* key, bool isAdmin){    

	bool result = false;
	int index = getIndex(id);
	if (index == -1) {
		struct HAPPairing item;
		memcpy(item.id, id, HAP_PAIRINGS_ID_LENGTH);
		memcpy(item.key, key, HAP_PAIRINGS_LTPK_LENGTH);
		item.isAdmin = isAdmin;
		
	#if HAP_DEBUG_PAIRINGS		
		HAPHelper::array_print("ID:", item.id, HAP_PAIRINGS_ID_LENGTH);	
		HAPHelper::array_print("Key:", item.key, HAP_PAIRINGS_LTPK_LENGTH);
	#endif

		pairings.push_back(item);
		result = true;
	} else {
		if (memcmp(pairings[index].key, key, HAP_PAIRINGS_LTPK_LENGTH) == 0) {
			result = true;
		}
		pairings[index].isAdmin = isAdmin;
		
	}

	return result;	
}

/*
struct HAPPairing HAPPairings::get(uint8_t* id) {
	for(size_t i = 0; i < pairings.size(); i++)
	{
		if (memcmp(pairings[i].id, id, HAPpairings_ID_SIZE) ) {
			return pairings[i];
		}
	}
	return struct HAPPairing;
}
*/


int HAPPairings::getIndex(const uint8_t* id) {

	int index = -1;

	for(int i = 0; i < pairings.size(); i++) {
		struct HAPPairing item = pairings[i];
		if ( memcmp(item.id, id, HAP_PAIRINGS_ID_LENGTH) == 0) {						
			index = i;
		}

	}
	return index;
}


int HAPPairings::getKey(const uint8_t* id, uint8_t* outkey) {
	
	// for(size_t i = 0; i < pairings.size(); i++) {
	// 	struct HAPPairing item = pairings[i];

	// 	// LogD("### - ID: ", false);
	// 	// HAPHelper::arrayPrint(item.id, HAPpairings_ID_SIZE);

	// 	if ( memcmp(item.id, id, HAP_PAIRINGS_ID_LENGTH) == 0) {
		
	// 		// LogD("### - KEY found: ", false);
	// 		// HAPHelper::arrayPrint(item.key, HAPpairings_LTPK_SIZE);
			
	// 		if (outkey != NULL)
	// 			memcpy(outkey, item.key, HAP_PAIRINGS_LTPK_LENGTH);
	// 		return 0;
	// 	}

	// }
	// return -1;

	int index = getIndex(id);
	if (index >= 0) {
		memcpy(outkey, pairings[index].key, HAP_PAIRINGS_LTPK_LENGTH);
	}
	return index;
}


uint8_t HAPPairings::size(){
	return pairings.size();
}

void HAPPairings::print(){
	for (int i=0; i < pairings.size(); i++){		
		HAPHelper::array_print("ID:", pairings[i].id, HAP_PAIRINGS_ID_LENGTH);		
		HAPHelper::array_print("Key:", pairings[i].key, HAP_PAIRINGS_LTPK_LENGTH);
		LogD("===============================================", true);
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

#if HAP_DEBUG_PAIRINGS
	HAPHelper::array_print("Long-term public key:", ltpk, HAP_PAIRINGS_LTPK_LENGTH);		
	HAPHelper::array_print("Long-term private key:", ltsk, HAP_PAIRINGS_LTSK_LENGTH);
	LogD("===============================================", true);
#endif
	return true;
}

bool HAPPairings::saveKeys(const uint8_t *ltpk, const uint8_t *ltsk){

	HAPKeys k;
	memcpy(k.ltpk, ltpk, HAP_PAIRINGS_LTPK_LENGTH);
	memcpy(k.ltsk, ltsk, HAP_PAIRINGS_LTSK_LENGTH);

#if HAP_DEBUG_PAIRINGS
	HAPHelper::array_print("Long-term public key:", ltpk, HAP_PAIRINGS_LTPK_LENGTH);		
	HAPHelper::array_print("Long-term private key:", ltsk, HAP_PAIRINGS_LTSK_LENGTH);
	LogD("===============================================", true);
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

bool HAPPairings::removePairing(const uint8_t *id){

	const auto orig_size = pairings.size();

	for (int i=0; i < HAP_PAIRINGS_MAX; i++){
		struct HAPPairing item = pairings[i];		
		if ( memcmp(item.id, id, HAP_PAIRINGS_ID_LENGTH) == 0) {

#if HAP_DEBUG_PAIRINGS
			LogD("Removing pairing for ", false);
			HAPHelper::array_print("ID:", pairings[i].id, HAP_PAIRINGS_ID_LENGTH);
#endif
			pairings.erase(pairings.begin() + i);
			
			if (pairings.size() == orig_size) {
				return false;
			}

			if (pairings.size() == 0) {
			
				resetEEPROM();
				LogI("Last pairing was removed!", true);

			} else {
				save();
			}

			break;
		}
	}

	return true;
}


bool HAPPairings::isAdmin(const uint8_t *id){

	int index = getIndex(id);

	if (index >= 0) {
		return pairings[index].isAdmin;
	}
	return false;
}