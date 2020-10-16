//
// HAPDeviceID.cpp
// Homekit
//
//  Created on: 23.08.2017
//      Author: michael
//

#include "HAPDeviceID.hpp"
#include "HAPHelper.hpp"


uint8_t HAPDeviceID::_deviceID[] = {'\0'};


uint8_t* HAPDeviceID::generateID() {
    if (_deviceID[0] == '\0')
        esp_read_mac(_deviceID, ESP_MAC_WIFI_STA);
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

String HAPDeviceID::serialNumber(String type, String id){
    char serialNumber[6 + 2 + type.length() + id.length()];
    sprintf(serialNumber, "%02X%02X%02X-%s-%s", _deviceID[3], _deviceID[4], _deviceID[5], type.c_str(), id.c_str());
    return String(serialNumber);
}