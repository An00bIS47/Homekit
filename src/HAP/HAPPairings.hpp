//
// HAPPairings.hpp
// Homekit
//
//  Created on: 13.04.2018
//      Author: michael
//
// 
// 
//  EEPROM
//    size = 4096 
//  -----------------------------
// | id  |  key	| id  |  key  | ...
//  -----------------------------
//   36     32     36     32 
//		 68			   68
// 
//  max 60 key pairs available 
//	for 16 pairings -> 68 * 16 = 1088
// 
// 	EEPROM structure:
// 
//  address | desc					 | length                       		
//  --------|------------------------|--------------------------------------	
//   0x00	| Long term public key 	 | 32 == 0x20								 
// 	 0x20   | Long term private key  | 64 == 0x40 -> + 0x20 = 0x60 ==  96   	
//  ________|________________________|______________________________________	
// | 0x60   | 1. Pairing: ID         | 36 == 0x24 -> + 0x60 = 0x84 == 132	|
// | 0x84	| 1. Pairing: public key | 36 == 0x24 -> + 0x84 = 0x8A == 168	|
// |--------|------------------------|--------------------------------------| 	
// | 0x8A	| 2. Pairing: ID 		 | ...									|
// | ...	| 2. Pairing: public key | ...									|
//  --------|------------------------|--------------------------------------
//  		| 						 |										
//  		| 						 |										
// 


#ifndef HAPPAIRINGS_HPP_
#define HAPPAIRINGS_HPP_

#include <Arduino.h>
#include <vector>
#include <EEPROM.h>

#include "HAPGlobals.hpp"

#ifndef HAP_PAIRINGS_MAX
#define HAP_PAIRINGS_MAX			16
#endif

#define HAP_EEPROM_SIZE				4096
// #define HAP_EEPROM_PARTITION		"eeprom"

#define HAP_EEPROM_OFFSET_PAIRINGS 	HAP_PAIRINGS_LTPK_LENGTH + HAP_PAIRINGS_LTSK_LENGTH


// HAPPairing 
//	id 	=  36
//	key =  32
// 	bool = 1
//	================
//         69 Bytes
struct HAPPairing {
	uint8_t id[HAP_PAIRINGS_ID_LENGTH];
	uint8_t key[HAP_PAIRINGS_LTPK_LENGTH];
	bool isAdmin;
};

struct HAPKeys {
	uint8_t ltpk[HAP_PAIRINGS_LTPK_LENGTH];
	uint8_t ltsk[HAP_PAIRINGS_LTSK_LENGTH];
};

class HAPPairings {

public:

	HAPPairings();

	bool begin();

	bool load();
	bool save();
	static void resetEEPROM();

	bool isAdmin(const uint8_t *id);
	

	bool saveKeys(const uint8_t *ltpk, const uint8_t *ltsk);
	bool loadKeys(uint8_t *ltpk, uint8_t *ltsk);

	bool removePairing(const uint8_t *id);

	void print();

	bool add(const uint8_t* id, const uint8_t* key, bool isAdmin);
	//struct HAPPairing get(uint8_t* id);
	int getKey(const uint8_t* id, uint8_t* outkey);
	uint8_t size();

	std::vector<HAPPairing> pairings;	
	int getIndex(const uint8_t* id);
	

};

#endif /* HAPPAIRINGS_HPP_ */