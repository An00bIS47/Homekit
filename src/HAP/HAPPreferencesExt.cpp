//
// HAPPreferences.cpp
// Homekit
//
//  Created on: 15.05.2020
//      Author: michael
//
#include "HAPPreferencesExt.hpp"
#include "HAPLogger.hpp"
#include "nvs.h"
#include "nvs_flash.h"

bool HAPPreferencesExt::begin(const char* partitionName, const char * name, bool readOnly){
    if(_started){
        return false;
    }

    // Initialize NVS.
	esp_err_t err = nvs_flash_init_partition(partitionName);
	if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
		LogE("ERROR: ESP_ERR_NVS_NO_FREE_PAGES", true);
		ESP_ERROR_CHECK(nvs_flash_erase());
		err = nvs_flash_init_partition(partitionName);
	}
	ESP_ERROR_CHECK( err );

	// Open
    _readOnly = readOnly;
    // esp_err_t err = nvs_open(name, readOnly?NVS_READONLY:NVS_READWRITE, &_handle);
    err = nvs_open_from_partition(partitionName, name, readOnly?NVS_READONLY:NVS_READWRITE, &_handle);
    if(err){
        Serial.println(err, HEX);
        return false;
    }
    _started = true;
    return true;
}