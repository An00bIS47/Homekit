//
// HAPDeviceID.cpp
// Homekit
//
//  Created on: 23.08.2017
//      Author: michael
//

#include "HAPDeviceID.hpp"
#include "HAPHelper.hpp"


byte HAPDeviceID::_deviceID[] = {'\0'};


byte* HAPDeviceID::generateID() {
    if (_deviceID[0] == '\0')
        esp_read_mac(_deviceID, ESP_MAC_WIFI_STA);

#if 0
	uint8_t tmp = _deviceID[2];

	_deviceID[2] = _deviceID[1];
	_deviceID[1] = tmp;
#endif

    return _deviceID;
}   
    
/*
const char* HAPDeviceID::deviceID(){
    char baseMacChr[18];
    sprintf(baseMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", _deviceID[0], _deviceID[1], _deviceID[2], _deviceID[3], _deviceID[4], _deviceID[5]);
    return baseMacChr;
}
*/

String HAPDeviceID::deviceID(){
    char baseMacChr[18];
    sprintf(baseMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", _deviceID[0], _deviceID[1], _deviceID[2], _deviceID[3], _deviceID[4], _deviceID[5]);
    return String(baseMacChr);
}

String HAPDeviceID::chipID(){
    char baseMacChr[18];
    sprintf(baseMacChr, "%02X%02X%02X%02X%02X%02X", _deviceID[5], _deviceID[4], _deviceID[3], _deviceID[2], _deviceID[1], _deviceID[0]);
    return String(baseMacChr);
}
