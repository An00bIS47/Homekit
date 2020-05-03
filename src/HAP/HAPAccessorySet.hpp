//
// HAPAccessorySet.hpp
// Homekit
//
//  Created on: 18.08.2017
//      Author: michael
//

#ifndef HAPACCESSORYSET_HPP_
#define HAPACCESSORYSET_HPP_

#include <Arduino.h>
#include <vector>

#include "HAPAccessory.hpp"
#include "HAPCategories.hpp"
#include "HAPPairings.hpp"

class HAPAccessorySet {
public:
	HAPAccessorySet();
	~HAPAccessorySet();

	static uint32_t configurationNumber;

	int aid(){ return _aid; };
	void begin();
	
	bool isPaired(){
		return _pairings.size() > 0;
	}
	
	HAPPairings* getPairings() {
		return &_pairings;
	}

	uint8_t accessoryType();
	void setAccessoryType(enum HAP_ACCESSORY_TYPE accessoryType);
	
	void addAccessoryInfo();

	const char* setupID();
	void generateSetupID();

	void setModelName(String name);
	const char* modelName();

	const char* setupHash();

	void setPinCode(const char* pinCode);
	const char* pinCode();
	const char* xhm();

	String describe();
	bool removeAccessory(HAPAccessory *acc);
	void addAccessory(HAPAccessory *acc);
	HAPAccessory* accessoryAtIndex(uint8_t index);
	HAPAccessory* accessoryWithAID(uint8_t aid);

	int32_t getValueForCharacteristics(int aid, int iid, char* out, size_t* outSize);
	characteristics* getCharacteristics(int aid, int iid);

	characteristics* getCharacteristicsOfType(int aid, uint8_t type);

	void setIdentifyCharacteristic(bool value);

	uint8_t numberOfAccessory();

protected:
	
	enum HAP_ACCESSORY_TYPE 	_accessoryType;

	// Setup ID can be provided, although, per spec, should be random
	// every time the instance is started. If not provided on init, will be random.
	// 4 digit string 0-9 A-Z
	String 		_setupID;
	String 		_setupHash;
	String		_xhm;

	String 		_modelName;	
	String 		_pinCode;	// xxx-xx-xxx

private:	
	void computeSetupHash();
	char randomChar(char* letters);

	void generateXMI();

	HAPPairings _pairings;
	std::vector<HAPAccessory *> _accessories;
    int _aid = 0;

    //AccessorySet(AccessorySet const&);
    //void operator=(AccessorySet const&);
};

#endif /* HAPACCESSORYSET_HPP_ */
