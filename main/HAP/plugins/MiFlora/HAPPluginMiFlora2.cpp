//
// HAPPluginMiFlora2.cpp
// Homekit
//
//  Created on: 01.08.2019
//      Author: michael
//

#include "HAPServer.hpp"
#include "HAPPluginMiFlora2.hpp"
#include "HAPPluginMiFloraScanner.hpp"
#include "HAPLogger.hpp"

#define VERSION_MAJOR       1
#define VERSION_MINOR       0
#define VERSION_REVISION    3
#define VERSION_BUILD       2


BLEUUID HAPPluginMiFlora2::_serviceUUID                 = BLEUUID::fromString("00001204-0000-1000-8000-00805f9b34fb");

BLEUUID HAPPluginMiFlora2::_uuid_write_mode             = BLEUUID::fromString("00001a00-0000-1000-8000-00805f9b34fb");
BLEUUID HAPPluginMiFlora2::_uuid_sensor_data            = BLEUUID::fromString("00001a01-0000-1000-8000-00805f9b34fb");
BLEUUID HAPPluginMiFlora2::_uuid_version_battery        = BLEUUID::fromString("00001a02-0000-1000-8000-00805f9b34fb");

#if HAP_PLUGIN_MIFLORA2_FETCH_HISTORY    
BLEUUID HAPPluginMiFlora2::_serviceHistoryUUID          = BLEUUID::fromString("00001206-0000-1000-8000-00805f9b34fb");

BLEUUID HAPPluginMiFlora2::_uuid_write_history_mode     = BLEUUID::fromString("00001a10-0000-1000-8000-00805f9b34fb");
BLEUUID HAPPluginMiFlora2::_uuid_history_read           = BLEUUID::fromString("00001a11-0000-1000-8000-00805f9b34fb");
BLEUUID HAPPluginMiFlora2::_uuid_device_time            = BLEUUID::fromString("00001a12-0000-1000-8000-00805f9b34fb");
#endif

std::vector<BLEAddress> HAPPluginMiFlora2::_supportedDevices;
std::vector<HAPPluginMiFloraDevice*> HAPPluginMiFlora2::_devices;
BLEClient* HAPPluginMiFlora2::_floraClient = nullptr;


HAPPluginMiFlora2::HAPPluginMiFlora2(){
    _type = HAP_PLUGIN_TYPE_ACCESSORY;
    _name = "MiFlora2";
    _isEnabled = HAP_PLUGIN_USE_MIFLORA2;
    _interval = HAP_MIFLORA_INTERVAL;
    _previousMillis = 0;    


    _version.major      = VERSION_MAJOR;
    _version.minor      = VERSION_MINOR;
    _version.revision   = VERSION_REVISION;
    _version.build      = VERSION_BUILD;
}

bool HAPPluginMiFlora2::begin(){

#if HAP_MIFLORA_DISABLE_BT_MODE_CLASSIC
    esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
#endif

    BLEDevice::init("");
    BLEDevice::setPower(ESP_PWR_LVL_P7);    
    _floraClient = BLEDevice::createClient();


    _supportedDevices.push_back(BLEAddress("C4:7C:8D:66:57:28"));

    return true;
}


void HAPPluginMiFlora2::handleImpl(bool forced){

    LogD("Handle " + String(__PRETTY_FUNCTION__) + " - interval: " + String(interval()), true);        

	struct floraData* deviceData = (struct floraData*)malloc(_supportedDevices.size() * sizeof(struct floraData));	
    processDevices(deviceData);



	free(deviceData);
}

HAPAccessory* HAPPluginMiFlora2::initAccessory(){
	

	return nullptr;
}

void HAPPluginMiFlora2::identify( bool oldValue, bool newValue) {
    printf("Start Identify MiFlora2 from member\n");
}

bool HAPPluginMiFlora2::containsDevice(std::string address){
    for (auto& device : _devices){
        if (device->address() == address) {
            return true;
        }
    }
    return false;
}


HAPPluginMiFloraDevice* HAPPluginMiFlora2::getDevice(std::string address){
    for (auto& device : _devices){
        if (device->address() == address) {
            return device;
        }
    }
    return nullptr;
}

HAPConfigValidationResult HAPPluginMiFlora2::validateConfig(JsonObject object){

    return HAPPlugin::validateConfig(object);
}

JsonObject HAPPluginMiFlora2::getConfigImpl(){
    DynamicJsonDocument doc(1);
	return doc.as<JsonObject>();
}

void HAPPluginMiFlora2::setConfigImpl(JsonObject root){

}


/**************************************************************************************************************
 *  Bluetooth implementation
 *   
 */

BLEClient* HAPPluginMiFlora2::getFloraClient(BLEAddress floraAddress) {	

	if (!_floraClient->connect(floraAddress)) {
		LogD("[MiFlora:" + String(floraAddress.toString().c_str()) + "] Connection failed, skipping", true);
		return nullptr;
	}

	LogD("[MiFlora:" + String(floraAddress.toString().c_str()) + "] Connectioning successful", true);
	return _floraClient;
}


BLERemoteService* HAPPluginMiFlora2::getFloraService(BLEClient* floraClient, BLEUUID uuid) {
	BLERemoteService* floraService = nullptr;

	try {
		floraService = floraClient->getService(uuid);
	}
	catch (...) {
		// something went wrong
	}
	if (floraService == nullptr) {
		LogD("- Failed to find data service", true);
	}
	else {
		LogD("- Found data service", true);
	}

	return floraService;
}

bool HAPPluginMiFlora2::forceFloraServiceDataMode(BLERemoteService* floraService, BLEUUID uuid, uint8_t* data, size_t dataLength) {
	BLERemoteCharacteristic* floraCharacteristic;
	
	// get device mode characteristic, needs to be changed to read data
	LogD("- Force device in data mode", true);
	floraCharacteristic = nullptr;
	try {
		floraCharacteristic = floraService->getCharacteristic(uuid);
	}
	catch (...) {
		// something went wrong
	}
	if (floraCharacteristic == nullptr) {
		LogD("-- Failed, skipping device", true);
		return false;
	}

	// write the magic data
	//uint8_t buf[2] = {0xA0, 0x1F};
	floraCharacteristic->writeValue(data, dataLength, true);

	delay(100);
	return true;
}

bool HAPPluginMiFlora2::readFloraDataCharacteristic(BLERemoteService* floraService, struct floraData* retData) {
	BLERemoteCharacteristic* floraCharacteristic = nullptr;

	// get the main device data characteristic
	LogD("- Access characteristic from device", true);
	try {
		floraCharacteristic = floraService->getCharacteristic(_uuid_sensor_data);
	}
	catch (...) {
		// something went wrong
	}
	if (floraCharacteristic == nullptr) {
		Serial.println("-- Failed, skipping device");
		return false;
	}

	// read characteristic value
	LogD("- Read value from characteristic", true);
	std::string value;
	try{
		value = floraCharacteristic->readValue();
	}
	catch (...) {
		// something went wrong
		LogD("-- Failed, skipping device", true);
		return false;
	}
	const char *val = value.c_str();

#if HAP_PLUGIN_MIFLORA2_DEBUG
    HAPHelper::array_print("value", (const unsigned char*)val, 16);
#endif

	int16_t* temp_raw = (int16_t*)val;
	float temperature = (*temp_raw) / ((float)10.0);

#if HAP_PLUGIN_MIFLORA2_DEBUG    
	Serial.print("-- Temperature: ");
	Serial.println(temperature);
#endif


	int moisture = val[7];

#if HAP_PLUGIN_MIFLORA2_DEBUG
	Serial.print("-- Moisture: ");
	Serial.println(moisture);
#endif

	int light = val[3] + val[4] * 256;
	
#if HAP_PLUGIN_MIFLORA2_DEBUG    
    Serial.print("-- Light: ");
	Serial.println(light);
#endif

	int conductivity = val[8] + val[9] * 256;

#if HAP_PLUGIN_MIFLORA2_DEBUG    
	Serial.print("-- Conductivity: ");
	Serial.println(conductivity);
#endif

	if ((temperature > 200) || (temperature < -100)) {
		Serial.println("-- Unreasonable values received, skip publish");
		return false;
	}

	retData->temperature = temperature;
	retData->moisture = moisture;
	retData->light = light;
	retData->conductivity = conductivity;

	return true;
}


bool HAPPluginMiFlora2::readFloraBatteryCharacteristic(BLERemoteService* floraService, struct floraData* retData) {
	BLERemoteCharacteristic* floraCharacteristic = nullptr;

	// get the device battery characteristic
	LogD("- Access battery characteristic from device", true);
	try {
		floraCharacteristic = floraService->getCharacteristic(_uuid_version_battery);
	}
	catch (...) {
		// something went wrong
	}
	if (floraCharacteristic == nullptr) {
		LogD("-- Failed, skipping battery level", true);
		return false;
	}

	// read characteristic value
	LogD("- Read value from characteristic", true);
	std::string value;

	try{
		value = floraCharacteristic->readValue();
	}
	catch (...) {
		// something went wrong
		LogD("-- Failed, skipping battery level", true);
		return false;
	}
	
    const char *val2 = value.c_str();

#if HAP_PLUGIN_MIFLORA2_DEBUG
    HAPHelper::array_print("value", (const unsigned char*)val2, strlen(val2));
#endif


	int battery = val2[0];

#if HAP_PLUGIN_MIFLORA2_DEBUG
	Serial.print("-- Battery: ");
	Serial.println(battery);
#endif

	retData->battery = battery;


    if (strlen(val2) == 7) {
        retData->firmware[0] = val2[2];
        retData->firmware[1] = val2[3];
        retData->firmware[2] = val2[4];
        retData->firmware[3] = val2[5];
        retData->firmware[4] = val2[6];
        retData->firmware[5] = '\0';

#if HAP_PLUGIN_MIFLORA2_DEBUG
    	Serial.print("-- Firmware: ");
    	Serial.println(retData->firmware);
#endif
    }

	return true;
}

#if HAP_PLUGIN_MIFLORA2_FETCH_HISTORY 
bool HAPPluginMiFlora2::readFloraDeviceTimeCharacteristic(BLERemoteService* floraService, uint32_t* deviceTime) {
	BLERemoteCharacteristic* floraCharacteristic = nullptr;

	// get the device device time characteristic
	LogD("- Access time characteristic from device", true);
	try {
		floraCharacteristic = floraService->getCharacteristic(_uuid_device_time);
	}
	catch (...) {
		// something went wrong
	}
	if (floraCharacteristic == nullptr) {
		LogD("-- Failed - char not found?, skipping device time", true);
		return false;
	}

	// read characteristic value
	LogD("- Read value from characteristic", true);
	std::string value;
	try{
		value = floraCharacteristic->readValue();
	}
	catch (...) {
		// something went wrong
		LogD("-- Failed - reading value, skipping device time", true);
		return false;
	}
	const char *val = value.c_str();
	*deviceTime = val[0] + val[1] * 256;

#if HAP_PLUGIN_MIFLORA2_DEBUG
	Serial.print("-- DeviceTime: ");
	Serial.println(*deviceTime);
	// retData->deviceTime = deviceTime;
#endif

	return true;
}

bool HAPPluginMiFlora2::readFloraHistoryEntryCountCharacteristic(BLERemoteService* floraService, uint16_t* entryCount) {
	BLERemoteCharacteristic* floraCharacteristic = nullptr;

	// get the device device time characteristic
	LogD("- Access history entry count characteristic from device", true);
	try {
		floraCharacteristic = floraService->getCharacteristic(_uuid_history_read);
	}
	catch (...) {
		// something went wrong
	}
	if (floraCharacteristic == nullptr) {
		LogD("-- Failed - char not found?, skipping device time", true);
		return false;
	}

	// read characteristic value
	LogD("- Read value from characteristic", true);
	std::string value;
	try{
		value = floraCharacteristic->readValue();
	}
	catch (...) {
		// something went wrong
		LogD("-- Failed - reading value, skipping device time", true);
		return false;
	}
	const char *val = value.c_str();
	*entryCount = val[0] + val[1] * 256;

#if HAP_PLUGIN_MIFLORA2_DEBUG
	Serial.print("-- Entry Count: ");
	Serial.println(*entryCount);
	// retData->deviceTime = deviceTime;
#endif


	return true;
}


void HAPPluginMiFlora2::entryAddress(uint8_t *address, uint16_t entry){
	address[0] = 0xA1;
    address[1] = entry;
    address[2] = entry << 8;
}


bool HAPPluginMiFlora2::readFloraHistoryEntryCharacteristic(BLERemoteService* floraService, struct floraHistory* history) {
	BLERemoteCharacteristic* floraCharacteristic = nullptr;

	// get the device device time characteristic
	Serial.println("- Access history entry characteristic from device");
	try {
		floraCharacteristic = floraService->getCharacteristic(_uuid_history_read);
	}
	catch (...) {
		// something went wrong
	}
	if (floraCharacteristic == nullptr) {
		Serial.println("-- Failed - char not found?, skipping device time");
		return false;
	}

	// read characteristic value
	Serial.println("- Read value from characteristic");
	std::string value;
	try{
		value = floraCharacteristic->readValue();
	}
	catch (...) {
		// something went wrong
		Serial.println("-- Failed - reading value, skipping device time");
		return false;
	}
	const char *val = value.c_str();

#if HAP_PLUGIN_MIFLORA2_DEBUG
    HAPHelper::array_print("value", (const unsigned char*)val, strlen(val));
#endif


	uint32_t timestamp = val[0] | ((uint32_t)val[1] << 8) |
                    ((uint32_t)val[2] << 16) | ((uint32_t)val[3] << 24);
	
	Serial.print("-- Timestamp: ");
	Serial.println(timestamp);

	int16_t temp_raw = val[4] | ((int16_t)val[5] << 8);
	float temperature = temp_raw / ((float)10.0);

	Serial.print("-- Temperature: ");
	Serial.println(temperature);

	uint32_t light = val[7] | ((uint32_t)val[8] << 8) |
                    ((uint32_t)val[9] << 16) | ((uint32_t)val[10] << 24);
	Serial.print("-- Light: ");
	Serial.println(light);

	int moisture = val[11];
	Serial.print("-- Moisture: ");
	Serial.println(moisture);
	
	int conductivity = val[12] | ((uint32_t)val[13] << 8);
	Serial.print("-- Conductivity: ");
	Serial.println(conductivity);

	if ((temperature > 200) || (temperature < -100)) {
		Serial.println("-- Unreasonable values received, skip publish");
		return false;
	}

	history->timestamp = timestamp;
	history->temperature = temperature;
	history->moisture = moisture;
	history->light = light;
	history->conductivity = conductivity;
	// retData->deviceTime = deviceTime;

	return true;
}


bool HAPPluginMiFlora2::getEntryCount(BLERemoteService* floraService, uint16_t *entryCount){
    // set device in data mode
	// write the magic data
	uint8_t buf[3] = {0xA0, 0x00, 0x00};
	if (!forceFloraServiceDataMode(floraService, _uuid_write_history_mode, buf, 3)) {
		return false;
	}

	bool entryCountSuccess = readFloraHistoryEntryCountCharacteristic(floraService, entryCount);

#if HAP_PLUGIN_MIFLORA2_DEBUG
	Serial.print(">>> entryCount: ");
	Serial.println(*entryCount);
#endif

    return entryCountSuccess;
}

bool HAPPluginMiFlora2::processFloraHistoryService(BLERemoteService* floraService, struct floraHistory* history, uint16_t entryCount) {
	
    uint32_t deviceTime = 0;
	bool deviceTimeSuccess = readFloraDeviceTimeCharacteristic(floraService, &deviceTime);


	if (entryCount == 0) {
		history->success = true;
		return history->success;
	}

	bool entrySuccess = false;
	
	
	Serial.println("History Data:");
	Serial.println("=============================================================");

	
	
	for (int i = 0; i < entryCount; i++){

		uint8_t address[3];
		entryAddress(&(*address), i);

		Serial.print("Address: ");
		for (int j = 0; j < 3; j++){
			Serial.printf("%02X", address[j]);			
		}
		Serial.println("");


		if (!forceFloraServiceDataMode(floraService, _uuid_write_history_mode, address, 3)) {
			break;
		}

		entrySuccess = readFloraHistoryEntryCharacteristic(floraService, &(history[i]));

		Serial.println("=============================================================");
		if (!entrySuccess){
			break;
		}
	}

	history->success = deviceTimeSuccess && entrySuccess;
	return history->success;
}

#endif

bool HAPPluginMiFlora2::processFloraService(BLERemoteService* floraService, struct floraData* retData) {
	
    bool batterySuccess = readFloraBatteryCharacteristic(floraService, retData);
	
	// set device in data mode
	// write the magic data
	uint8_t buf[2] = {0xA0, 0x1F};
	if (!forceFloraServiceDataMode(floraService, _uuid_write_mode, buf, 2)) {
		return false;
	}

	bool dataSuccess = readFloraDataCharacteristic(floraService, retData);
	
	retData->success = dataSuccess && batterySuccess;
	return retData->success;
}


bool HAPPluginMiFlora2::processFloraDevice(BLEAddress floraAddress, int tryCount, struct floraData* retData) {
	Serial.print("Processing Flora device at ");
	Serial.print(floraAddress.toString().c_str());
	Serial.print(" (try ");
	Serial.print(tryCount);
	Serial.println(")");

	// connect to flora ble server
	BLEClient* floraClient = getFloraClient(floraAddress);
	if (floraClient == nullptr) {
		return false;
	}


	// connect data service
	BLERemoteService* floraService = getFloraService(floraClient, _serviceUUID);
	if (floraService == nullptr) {
		floraClient->disconnect();
		return false;
	}

	// process devices data
	bool success = processFloraService(floraService, retData);
	// blink(floraService);


#if HAP_PLUGIN_MIFLORA2_FETCH_HISTORY
	BLERemoteService* floraHistoryService = getFloraService(floraClient, _serviceHistoryUUID);
	if (floraHistoryService == nullptr) {
		floraClient->disconnect();
		return false;
	}

	Serial.print("HEAP: ");
	Serial.println(ESP.getFreeHeap());

    uint16_t entryCount = 0;
    getEntryCount(floraHistoryService, &entryCount);	

    struct floraHistory* history = (struct floraHistory*)malloc(entryCount * sizeof(struct floraHistory));
	bool successHistory = processFloraHistoryService(floraHistoryService, history, entryCount);
	// blink(floraService);

    success = success && successHistory;

    free(history);

#endif

	// disconnect from device
	floraClient->disconnect();

	Serial.print("HEAP: ");
	Serial.println(ESP.getFreeHeap());


	return success;
}


void HAPPluginMiFlora2::processDevices(struct floraData* deviceData){
	// check if battery status should be read - based on boot count
    // process devices    


    for (int i = 0; i < _supportedDevices.size(); i++) {

        HAPPluginMiFloraDevice* newDevice = nullptr;        
        BLEAddress deviceAddress = _supportedDevices[i];
    
        if (!containsDevice(deviceAddress.toString())){
            // not in devices list -> initialize and add to accessorySet
            LogI(HAPServer::timeString() + " " + "MiFlora2" + "->" + String(__FUNCTION__) + " [   ] " + "Add MiFlora device to devices list: " + String(deviceAddress.toString().c_str()), true);
            
            newDevice = new HAPPluginMiFloraDevice(deviceAddress.toString());

            newDevice->setFakeGatoFactory(_fakeGatoFactory);
            newDevice->setEventManager(_eventManager);

            newDevice->begin();                
            _accessorySet->addAccessory(newDevice->initAccessory());
            _devices.push_back(newDevice);

        } else {
            newDevice = getDevice(deviceAddress.toString());
        }

        
        
        int retryCount = 0;            
        for (retryCount=0; retryCount < HAP_MIFLORA_RETRY; retryCount++){

            if (processFloraDevice(deviceAddress, retryCount, &(deviceData[i]))) {                                    
                newDevice->setValue(deviceData[i]);
                                
                break;
            }  

            delay(2000);
            
        }           
    }	
}