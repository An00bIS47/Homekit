//
// HAPDeviceID.hpp
// Homekit
//
//  Created on: 23.08.2017
//      Author: michael
//

#ifndef HAPDEVICEID_HPP_
#define HAPDEVICEID_HPP_

#include <Arduino.h>
#include <WiFi.h>
#include "esp_system.h"

class HAPDeviceID {
public:

	static byte* generateID();
	//static byte* deviceID();
	//static const char* deviceID();

	static String deviceID();	// mac address -> change to random
	static String chipID();		// basically mac reverse

private:
	static byte _deviceID[6];
};

#endif /* HAPDEVICEID_HPP_ */
