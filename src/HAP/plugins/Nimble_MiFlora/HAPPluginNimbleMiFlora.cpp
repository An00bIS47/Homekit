//
// HAPPluginNimbleMiFlora.cpp
// Homekit
//
//  Created on: 01.08.2019
//      Author: michael
//

#include "HAPServer.hpp"
#include "HAPPluginNimbleMiFlora.hpp"
#include "HAPLogger.hpp"


#define VERSION_MAJOR       1
#define VERSION_MINOR       3
#define VERSION_REVISION    2
#define VERSION_BUILD       4



#if HAP_PLUGIN_MIFLORA_ENABLE_SCANNER 

#ifndef HAP_PLUGIN_MIFLORA_SCAN_INTERVAL
#define HAP_PLUGIN_MIFLORA_SCAN_INTERVAL    300000
#endif

#ifndef HAP_PLUGIN_MIFLORA_INTERVAL
#define HAP_PLUGIN_MIFLORA_INTERVAL			1000
#endif

#include "HAPPluginNimbleMiFloraScanner.hpp"
#endif


std::vector<BLEAddress> HAPPluginNimbleMiFlora::_supportedDevices;
std::vector<HAPPluginNimbleMiFloraDevice*> HAPPluginNimbleMiFlora::_devices;
BLEClient* HAPPluginNimbleMiFlora::_floraClient = nullptr;


HAPPluginNimbleMiFlora::HAPPluginNimbleMiFlora(){
    _type = HAP_PLUGIN_TYPE_ACCESSORY;
    _name = "MiFlora";
    _isEnabled = HAP_PLUGIN_USE_NIMBLE_MIFLORA;
    _interval = HAP_PLUGIN_MIFLORA_INTERVAL;
    _previousMillis = HAP_PLUGIN_MIFLORA_INTERVAL;    	

    _version.major      = VERSION_MAJOR;
    _version.minor      = VERSION_MINOR;
    _version.revision   = VERSION_REVISION;
    _version.build      = VERSION_BUILD;

#if HAP_PLUGIN_MIFLORA_ENABLE_SCANNER
	_intervalScan   	= HAP_PLUGIN_MIFLORA_SCAN_INTERVAL;
	_previousMillisScan	= 0;
#endif	
}

bool HAPPluginNimbleMiFlora::begin(){

    BLEDevice::init("");
    BLEDevice::setPower(ESP_PWR_LVL_P7);    
    _floraClient = BLEDevice::createClient();

#if HAP_PLUGIN_MIFLORA_ENABLE_SCANNER

#else
    _supportedDevices.push_back(BLEAddress("C4:7C:8D:66:57:28"));
#endif

	_previousMillisScan = (HAP_PLUGIN_MIFLORA_SCAN_INTERVAL - 1000);
    return true;
}


void HAPPluginNimbleMiFlora::handleImpl(bool forced){    
	
#if HAP_PLUGIN_MIFLORA_ENABLE_SCANNER

	if (shouldScan()) {

		LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Handle scanning for new devices [" + String(_intervalScan) + "]", true);

		HAPPluginNimbleMiFloraScanner floraScanner;
		if (floraScanner.scan()) {
            
            LogV("Found the following devices: ", true);
			for (int i = 0; i < floraScanner.getDeviceCount(); i++){
                LogV("   - " + String(i) + ": " + String(floraScanner.getDeviceAddress(i).c_str()), true);

				if (!containsDevice(floraScanner.getDeviceAddress(i))){                                        

					LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Found new device [" + String(floraScanner.getDeviceAddress(i).c_str()) + "]: Add to supported list", true);

                    // Commit 353e5a6 breaks this, BLEAddress can not be retrieved afterwards :(
					_supportedDevices.push_back(BLEAddress(floraScanner.getDeviceAddress(i)));
				}
			}
		}
	}

#endif
    
    processDevices();
	
}

HAPAccessory* HAPPluginNimbleMiFlora::initAccessory(){
	

	return nullptr;
}

void HAPPluginNimbleMiFlora::identify( bool oldValue, bool newValue) {
    printf("Start Identify %s -> from member\n", _name.c_str());
}

bool HAPPluginNimbleMiFlora::containsDevice(const std::string address){
    for (auto& device : _devices){
        if (device->address() == address) {
            return true;
        }
    }
    return false;
}


HAPPluginNimbleMiFloraDevice* HAPPluginNimbleMiFlora::getDevice(std::string address){
    for (auto& device : _devices){
        if (device->address() == address) {
            return device;
        }
    }
    return nullptr;
}

HAPConfigValidationResult HAPPluginNimbleMiFlora::validateConfig(JsonObject object){

	HAPConfigValidationResult result;
    
    result = HAPPlugin::validateConfig(object);
    if (result.valid == false) {
        return result;
    }
    result.valid = false;
    
    // plugin._name.username
    if (object.containsKey("intervalScan") && !object["intervalScan"].is<uint32_t>()) {
        result.reason = "plugins." + _name + ".intervalScan is not an integer";
        return result;
    }

    result.valid = true;
    return result;
}

JsonObject HAPPluginNimbleMiFlora::getConfigImpl(){
    DynamicJsonDocument doc(128);
#if HAP_PLUGIN_MIFLORA_ENABLE_HISTORY 
	doc["intervalScan"] = _intervalScan;
#endif
	return doc.as<JsonObject>();
}

void HAPPluginNimbleMiFlora::setConfigImpl(JsonObject root){

#if HAP_PLUGIN_MIFLORA_ENABLE_HISTORY 	
    if (root.containsKey("intervalScan")){
        // LogD(" -- password: " + String(root["password"]), true);
        _intervalScan = root["intervalScan"].as<uint32_t>();
    }	
#endif	

}





void HAPPluginNimbleMiFlora::processDevices(){
	// check if battery status should be read - based on boot count
    // process devices    

#if 0
    LogV("Supported devices list: ", true);
#endif

    for (int i = 0; i < _supportedDevices.size(); i++) {

        HAPPluginNimbleMiFloraDevice* newDevice = nullptr;        
        BLEAddress deviceAddress = _supportedDevices[i];

#if 0        
        LogV("   - " + String(i) + ": " + String(_supportedDevices[i].toString().c_str()), true);
#endif


        if (!containsDevice(deviceAddress.toString())){
            // not in devices list -> initialize and add to accessorySet
            LogI(HAPServer::timeString() + " " + "MiFlora" + "->" + String(__FUNCTION__) + " [   ] " + "Add MiFlora device to devices list [" + String(deviceAddress.toString().c_str()) + "]", true);
            
            newDevice = new HAPPluginNimbleMiFloraDevice(_floraClient, deviceAddress.toString());

            newDevice->setFakeGatoFactory(_fakeGatoFactory);
            newDevice->setEventManager(_eventManager);

            newDevice->begin();                
            _accessorySet->addAccessory(newDevice->initAccessory());
            _devices.push_back(newDevice);

        } else {
            newDevice = getDevice(deviceAddress.toString());
        }
        
        
        int retryCount = 0;            
        for (retryCount=0; retryCount < HAP_PLUGIN_MIFLORA_RETRY; retryCount++){

            if (newDevice->processFloraDevice()) {                                                    				                                
                break;
            }  
			LogE(HAPServer::timeString() + " " + "MiFlora" + "->" + String(__FUNCTION__) + " [   ] " + "Could not process device [" + String(deviceAddress.toString().c_str()) + "]", true);
            yield();
            
        }           
    }	
}



bool HAPPluginNimbleMiFlora::shouldScan(){
    
    unsigned long currentMillis = millis(); // grab current time

    if ((unsigned long)(currentMillis - _previousMillisScan) >= _intervalScan) {

        // save the last time you blinked the LED
        _previousMillisScan = currentMillis;

        //LogD("Handle plugin: " + String(_name), true);			
        return true;			
    }
    

    return false;
}