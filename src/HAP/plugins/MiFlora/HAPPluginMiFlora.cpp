//
// HAPPluginMiFlora.cpp
// Homekit
//
//  Created on: 01.08.2019
//      Author: michael
//

#include "HAPServer.hpp"
#include "HAPPluginMiFlora.hpp"
#include "HAPPluginMiFloraScanner.hpp"
#include "HAPLogger.hpp"


#if HAP_PLUGIN_USE_MIFLORA


#define VERSION_MAJOR       1
#define VERSION_MINOR       0
#define VERSION_REVISION    3
#define VERSION_BUILD       2

#define USE_FERT            1



BLEUUID HAPPluginMiFlora::_serviceUUID          = BLEUUID::fromString("00001204-0000-1000-8000-00805f9b34fb");
BLEUUID HAPPluginMiFlora::_uuid_version_battery = BLEUUID::fromString("00001a02-0000-1000-8000-00805f9b34fb");
BLEUUID HAPPluginMiFlora::_uuid_sensor_data     = BLEUUID::fromString("00001a01-0000-1000-8000-00805f9b34fb");
BLEUUID HAPPluginMiFlora::_uuid_write_mode      = BLEUUID::fromString("00001a00-0000-1000-8000-00805f9b34fb");

std::vector<HAPPluginMiFloraDevice*> HAPPluginMiFlora::_devices;

BLEClient* HAPPluginMiFlora::_floraClient = nullptr;

HAPPluginMiFloraDevicesScanner HAPPluginMiFlora::floraScanner;

TaskHandle_t HAPPluginMiFlora::_task;
hw_timer_t* HAPPluginMiFlora::_timer;

bool HAPPluginMiFlora::isConnected = false;
bool HAPPluginMiFlora::isInitialized = false;


// char* FLORA_DEVICES[] = {
//     "C4:7C:8D:66:57:28"
// };

void IRAM_ATTR resetModule() {
    //  ets_printf("WDTDog triggert -> delete task\n");
    ets_printf("\n");
    ets_printf("=====================================================\n");
    ets_printf(" !!! Watchdog got triggered -> Deleting ble task !!! \n");
    ets_printf("=====================================================\n");
    ets_printf("\n");

    ets_printf("elapsed %d", timerReadSeconds(HAPPluginMiFlora::_timer));

    // esp_restart();
    if (HAPPluginMiFlora::_task){

        ets_printf("Feed wdt");
        timerWrite(HAPPluginMiFlora::_timer, 0); //reset timer (feed watchdog

        ets_printf("Stopping ble client ...\n");
        HAPPluginMiFlora::stopClient();

        ets_printf("Stopping ble scan ...\n");
        if (HAPPluginMiFlora::floraScanner.isScanning()) {
            ets_printf("Stopping ble scan - now really...\n");
            HAPPluginMiFlora::floraScanner.stop();
        }
        
        
        // ets_printf("deinit ble device ...\n");
        // BLEDevice::deinit();
        HAPPluginMiFlora::isInitialized = false;

        if (HAPPluginMiFlora::_task) {
            ets_printf("Deleting task ...\n");
            vTaskDelete(HAPPluginMiFlora::_task);
        }
            
        HAPPluginMiFlora::_task = NULL;

        ets_printf("Disabling wdt ...\n");
        timerAlarmDisable(HAPPluginMiFlora::_timer);

        ets_printf("Stopping wdt ...\n");
        timerStop(HAPPluginMiFlora::_timer);
        
        

    } else {
        ets_printf("NO task handle available");        
    }
}




#if 0
class MyClientCallback : public BLEClientCallbacks {
    void onConnect(BLEClient* pclient) {
        HAPPluginMiFlora::isConnected = true;
        Serial.println("onConnect");
    }

    void onDisconnect(BLEClient* pclient) {
        HAPPluginMiFlora::isConnected = false;
        Serial.println("onDisconnect");
        //BLEDevice::deinit();        
    }
};

static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    // Serial.print("data: ");
    // Serial.println((char*)pData);
    Serial.print("Hex: ");
    for (int i = 0; i < length; i++) {
        Serial.print((int)pData[i], HEX);
        Serial.print(" ");
    }
    Serial.println(" ");
}
#endif

HAPPluginMiFlora::HAPPluginMiFlora(){
    _type = HAP_PLUGIN_TYPE_ACCESSORY;
    _name = "MiFlora";
    _isEnabled = HAP_PLUGIN_USE_MIFLORA;
    _interval = HAP_MIFLORA_INTERVAL;
    _previousMillis = 0;    

    _previousMillisWDT  = 0;

    _version.major      = VERSION_MAJOR;
    _version.minor      = VERSION_MINOR;
    _version.revision   = VERSION_REVISION;
    _version.build      = VERSION_BUILD;

    _wdtTimeout = HAP_MIFLORA_WDT_TIMEOUT;

#if !HAP_MIFLORA_USE_BLE_SCANNER
    _deviceCount = sizeof FLORA_DEVICES / sizeof FLORA_DEVICES[0];
#endif


}

bool HAPPluginMiFlora::begin(){
    

    HAPPluginMiFlora::_timer = timerBegin(0, 80, true);                  //timer 0, div 80
    timerAttachInterrupt(HAPPluginMiFlora::_timer, &resetModule, true);  //attach callback
    timerAlarmWrite(HAPPluginMiFlora::_timer, _wdtTimeout * 1000, false); //set time in us
    timerAlarmEnable(HAPPluginMiFlora::_timer);                          //enable interrupt


#if HAP_MIFLORA_DISABLE_BT_MODE_CLASSIC
    esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
#endif

    BLEDevice::init("");
    BLEDevice::setPower(ESP_PWR_LVL_P7);    
    _floraClient = BLEDevice::createClient();
    isInitialized = true;
    
    return true;
}


void HAPPluginMiFlora::handleImpl(bool forced){

    LogD("Handle " + String(__PRETTY_FUNCTION__) + " - interval: " + String(interval()), true);        

    if (isInitialized == false){
        BLEDevice::init("");
        BLEDevice::setPower(ESP_PWR_LVL_P7);
        isInitialized = true;
        _floraClient = BLEDevice::createClient();
    }


    if (HAPPluginMiFlora::_task) {
        eTaskState state = eTaskGetState(HAPPluginMiFlora::_task);

        // Serial.println("State: " + String(state));

        if (state != 1) {
            LogW("State not ready (!=1): " + String(state) + " - Don't create new tasked!", true);
            return;
        }
    }                             

    xTaskCreatePinnedToCore(
            Task1code,                  /* Task function. */
            "Task1",                    /* name of task. */
            10000,                      /* Stack size of task */
            (void*)this,                /* parameter of the task */
            1,                          /* priority of the task */
            &HAPPluginMiFlora::_task,   /* Task handle to keep track of created task */
            0);                         /* pin task to core 0 */

}


void HAPPluginMiFlora::Task1code( void * pvParameters ){
    LogD("MiFlora running on core " + String(xPortGetCoreID()), true);    
    
    HAPPluginMiFlora* foo = reinterpret_cast<HAPPluginMiFlora*>(pvParameters);
 
    // Serial.println("Enabling wdt");
    timerAlarmEnable(HAPPluginMiFlora::_timer);                          //enable interrupt    
    
    // Serial.println("Starting timer");
    timerStart(foo->_timer);

    // Serial.println("Feed wdt");
    timerWrite(foo->_timer, 0); //reset timer (feed watchdog
    // *(pvParameters).shouldHandle()
        
    bool readBattery = true;

    bool processedData = false;
    
    if (floraScanner.scan()) {            

        struct floraData* deviceData = (struct floraData*)malloc(floraScanner.getDeviceCount() * sizeof(struct floraData));

        // process devices
        for (int i = 0; i < floraScanner.getDeviceCount(); i++) {

            HAPPluginMiFloraDevice* newDevice = nullptr;
            // ToDo: Proper implement with deivces
            std::string deviceAddress = floraScanner.getDeviceAddress(i);
            if (!containsDevice(deviceAddress)){
                // not in devices list -> initialize and add to accessorySet
                LogI(HAPServer::timeString() + " " + "MiFlora" + "->" + String(__FUNCTION__) + " [   ] " + "Add MiFlora device to devices list: " + String(deviceAddress.c_str()), true);
                
                newDevice = new HAPPluginMiFloraDevice(deviceAddress);

                newDevice->setFakeGatoFactory(foo->_fakeGatoFactory);
                newDevice->setEventManager(foo->_eventManager);

                newDevice->begin();                
                foo->_accessorySet->addAccessory(newDevice->initAccessory());
                foo->_devices.push_back(newDevice);

            } else {
                newDevice = getDevice(deviceAddress);
            }

            BLEAddress floraAddress(deviceAddress.c_str());
            LogD(HAPServer::timeString() + " " + "MiFlora" + "->" + String(__FUNCTION__) + " [   ] " + "Feed the timer cause we can process device " + String(deviceAddress.c_str()), true);
            timerWrite(foo->_timer, 0); //reset timer (feed watchdog
            
            int retryCount = 0;            
            for (retryCount=0; retryCount < 3; retryCount++){
                
                foo->stopClient();

                if (foo->processFloraDevice(floraAddress, readBattery, retryCount, &(deviceData[i]))) {                    
                    LogD(HAPServer::timeString() + " " + "MiFlora" + "->" + String(__FUNCTION__) + " [   ] " + "Duration for processing " + String(timerReadSeconds(HAPPluginMiFlora::_timer)), true);
                    newDevice->setValue(deviceData[i]);
                    
                    processedData = true;

                    LogD(HAPServer::timeString() + " " + "MiFlora" + "->" + String(__FUNCTION__) + " [   ] " + "Feed the timer cause we are done with device " + String(deviceAddress.c_str()), true);
                    timerWrite(foo->_timer, 0); //reset timer (feed watchdog
                    break;
                }  

                delay(2000);
                timerWrite(foo->_timer, 0); //reset timer (feed watchdog
            }           
        }

        free(deviceData);
    }
    
    if (!processedData) {
        LogD(HAPServer::timeString() + " " + "MiFlora" + "->" + String(__FUNCTION__) + " [   ] " + "Duration for scanning: " + String(timerReadSeconds(HAPPluginMiFlora::_timer)), true);
        // timerWrite(foo->_timer, 0); //reset timer (feed watchdog    
    }


    LogD(HAPServer::timeString() + " " + "MiFlora" + "->" + String(__FUNCTION__) + " [   ] " + "Stop timer", true);
    timerStop(HAPPluginMiFlora::_timer);
    
    // LogD(HAPServer::timeString() + " " + "MiFlora" + "->" + String(__FUNCTION__) + " [   ] " + "Disable timer", true);
    timerAlarmDisable(HAPPluginMiFlora::_timer);

    floraScanner.clearDevices();

    LogD(HAPServer::timeString() + " " + "MiFlora" + "->" + String(__FUNCTION__) + " [   ] " + "Task ended", true);
    vTaskDelete( NULL );
}



HAPAccessory* HAPPluginMiFlora::initAccessory(){
	

	return nullptr;
}




void HAPPluginMiFlora::identify( bool oldValue, bool newValue) {
    printf("Start Identify MiFlora from member\n");
}

bool HAPPluginMiFlora::containsDevice(std::string address){
    for (auto& device : _devices){
        if (device->address() == address) {
            return true;
        }
    }
    return false;
}


HAPPluginMiFloraDevice* HAPPluginMiFlora::getDevice(std::string address){
    for (auto& device : _devices){
        if (device->address() == address) {
            return device;
        }
    }
    return nullptr;
}

#if 0
bool HAPPluginMiFlora::processFloraDevice2(BLEAddress floraAddress, bool getBattery, int tryCount, struct floraData* retData) {
    _floraClient->setClientCallbacks(new MyClientCallback());

    if (!isConnected) {
        _floraClient = getFloraClient(floraAddress);
        if (_floraClient == nullptr) {        
            return false;
        }
    }

    BLERemoteService* floraService = getFloraService();

    if (floraService == nullptr) {        
        _floraClient->disconnect();
        return false;
    }


    // Force Data Mode
    BLERemoteCharacteristic* floraWriteCharacteristic = floraService->getCharacteristic(_uuid_write_mode);
    if (floraWriteCharacteristic == nullptr) {
        Serial.println("-- Failed, skipping device");
        _floraClient->disconnect();
        return false;
    }

    // write the magic data    
    if (floraWriteCharacteristic->canWrite()) {
        uint8_t buf[2] = {0xA0, 0x1F};
        floraWriteCharacteristic->writeValue(buf, 2, true);
    }

    BLERemoteCharacteristic* floraDataCharacteristic = floraService->getCharacteristic(_uuid_sensor_data);
    if (floraDataCharacteristic == nullptr) {
        Serial.println("-- Failed, skipping device");
        return false;
    }    

    std::string value = "";
    if(floraDataCharacteristic->canRead()) {
        value = floraDataCharacteristic->readValue();
    }

    if(floraDataCharacteristic->canNotify()){
        floraDataCharacteristic->registerForNotify(notifyCallback);
    }

    isConnected = true;
    return true;
}
#endif

bool HAPPluginMiFlora::processFloraDevice(BLEAddress floraAddress, bool getBattery, int tryCount, struct floraData* retData) {
    Serial.print("Processing Flora device at ");
    Serial.print(floraAddress.toString().c_str());
    Serial.print(" (try ");
    Serial.print(tryCount);
    Serial.println(")");

    bool success = false;
    try {
        
        _floraClient = getFloraClient(floraAddress);
        if (_floraClient == nullptr) {        
            return false;
        }

        // connect data service
        //BLERemoteService* floraService = getFloraService(floraClient);    
        // Serial.println(_serviceUUID.toString().c_str());
        
        BLERemoteService* floraService = getFloraService();


        if (floraService == nullptr) {        
            _floraClient->disconnect();
            return false;
        }

        // process devices data
        success = processFloraService(floraService, getBattery, retData);    
        
            
        // disconnect from device
        _floraClient->disconnect();
        
        // if (floraClient)
        //   delete floraClient;



    } catch(...) {
        LogE(HAPServer::timeString() + " " + "MiFlora" + "->" + String(__FUNCTION__) + " [   ] " + "Process device failed!", true);
    }
    


    return success;
}

bool HAPPluginMiFlora::processFloraService(BLERemoteService* floraService, bool readBattery, struct floraData* retData) {
    // set device in data mode

    bool dataSuccess = true;
    bool batterySuccess = true;
    
    // try {


    if (!forceFloraServiceDataMode(floraService)) {
        return false;
    }

    dataSuccess = readFloraDataCharacteristic(floraService, retData);            
    
    if (readBattery) {
        batterySuccess = readFloraBatteryCharacteristic(floraService, retData);
    }
    // } catch(...) {
    //     LogE(HAPServer::timeString() + " " + "MiFlora" + "->" + String(__FUNCTION__) + " [   ] " + "Get Service for device failed!", true);
    // }
    
    retData->success = dataSuccess && batterySuccess;
    return retData->success;
}

bool HAPPluginMiFlora::readFloraBatteryCharacteristic(BLERemoteService* floraService, struct floraData* retData) {
  BLERemoteCharacteristic* floraCharacteristic = nullptr;

    // get the device battery characteristic
    Serial.println("- Access battery characteristic from device");
    try {
        floraCharacteristic = floraService->getCharacteristic(_uuid_version_battery);

        // for (auto &myPair : *floraService->getCharacteristics()) {
        //     std::string uuidPadded = std::string(36 - myPair.first.length(), '0') + myPair.first;

        //     Serial.print("UUID: ");
        //     Serial.print(uuidPadded.c_str());
        //     Serial.print(" ?? ");
        //     Serial.print(_uuid_version_battery.toString().c_str());

        //     if (uuidPadded == _uuid_version_battery.toString()) {
        //         Serial.println(" - MATCH");
        //         Serial.println(myPair.second->toString().c_str());
        //         _floraCharacteristic = myPair.second;
        //         break;
        //     } else {
        //         Serial.println(" - NO MATCH");
        //         _floraCharacteristic = nullptr;
        //     }
        // }
    }
    catch (...) {
        // something went wrong
        Serial.println("-- Failed, skipping battery level");
        _floraClient->disconnect();
        return false;
    }


    if (floraCharacteristic == nullptr) {
        Serial.println("-- Failed, skipping battery level");
        _floraClient->disconnect();
        return false;
    }

    // read characteristic value
    Serial.println("- Read value from characteristic");
    std::string value = "";
    try{
        value = floraCharacteristic->readValue();        
    }
    catch (...) {
        // something went wrong
        Serial.println("-- Failed, skipping battery level");
        _floraClient->disconnect();
        return false;
    }

    const char *val2 = value.c_str();

    int battery = val2[0];
    retData->battery = battery;

    Serial.println("-- Battery: " + String(retData->battery));

    char firmware[6];
    int count = 0;

    for (int i = 2; i < sizeof(value); i++) { 
        firmware[count] = value[i];                 
        count++;
    } 
    firmware[count] = '\0';  
    strncpy(retData->firmware, firmware, 6);

    Serial.println("-- Firmware: " + String(retData->firmware));  

    return true;
}

bool HAPPluginMiFlora::readFloraDataCharacteristic(BLERemoteService* floraService, struct floraData* retData) {
    //BLERemoteCharacteristic* floraCharacteristic = nullptr;
    BLERemoteCharacteristic* floraDataCharacteristic = nullptr;
    // get the main device data characteristic
    Serial.println("- Access characteristic from device");
    try {
        floraDataCharacteristic = floraService->getCharacteristic(_uuid_sensor_data);

        // for (auto &myPair : *floraService->getCharacteristics()) {
        //     std::string uuidPadded = std::string(36 - myPair.first.length(), '0') + myPair.first;

        //     Serial.print("UUID: ");
        //     Serial.print(uuidPadded.c_str());
        //     Serial.print(" ?? ");
        //     Serial.print(_uuid_sensor_data.toString().c_str());

        //     if (uuidPadded == _uuid_sensor_data.toString()) {
        //         Serial.println(" - MATCH");
        //         Serial.println(myPair.second->toString().c_str());
        //         _floraCharacteristic = myPair.second;
        //         break;
        //     } else {
        //         Serial.println(" - NO MATCH");
        //         _floraCharacteristic = nullptr;
        //     }
        // }

    }
    catch (...) {
        // something went wrong
        Serial.println("-- Failed, skipping device");
        _floraClient->disconnect();
        return false;
    }
    if (floraDataCharacteristic == nullptr) {
        Serial.println("-- Failed, skipping device");
        return false;
    }    

    // read characteristic value
    Serial.println("- Read value from characteristic");
    std::string value;
    try{
        if(floraDataCharacteristic->canRead()) {

            value = floraDataCharacteristic->readValue();
        }

        // if(_floraCharacteristic->canNotify())
        //     _floraCharacteristic->registerForNotify(notifyCallback);
    }
    catch (...) {
        // something went wrong
        Serial.println("-- Failed, skipping device");
        _floraClient->disconnect();
        return false;
    }
    const char *val = value.c_str();

    Serial.print("Hex: ");
    for (int i = 0; i < 16; i++) {
        Serial.print((int)val[i], HEX);
        Serial.print(" ");
    }
    Serial.println(" ");

    int16_t* temp_raw = (int16_t*)val;
    float temperature = (*temp_raw) / ((float)10.0);
    Serial.print("-- Temperature: ");
    Serial.println(temperature);

    int moisture = val[7];
    Serial.print("-- Moisture: ");
    Serial.println(moisture);

    int light = val[3] + val[4] * 256;
    Serial.print("-- Light: ");
    Serial.println(light);
    
    int conductivity = val[8] + val[9] * 256;
    Serial.print("-- Conductivity: ");
    Serial.println(conductivity);

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

bool HAPPluginMiFlora::forceFloraServiceDataMode(BLERemoteService* floraService) {
    BLERemoteCharacteristic* floraCharacteristic;
    
    // get device mode characteristic, needs to be changed to read data
    Serial.println("- Force device in data mode");
    floraCharacteristic = nullptr;
    try {       
        
        // for (auto &myPair : *floraService->getCharacteristics()) {
        //     std::string uuidPadded = std::string(36 - myPair.first.length(), '0') + myPair.first;

        //     Serial.print("UUID: ");
        //     Serial.print(uuidPadded.c_str());
        //     Serial.print(" ?? ");
        //     Serial.print(_uuid_write_mode.toString().c_str());

        //     if (uuidPadded == _uuid_write_mode.toString()) {
        //         Serial.println(" - MATCH");
        //         Serial.println(myPair.second->toString().c_str());
        //         _floraCharacteristic = myPair.second;
        //         break;
        //     } else {
        //         Serial.println(" - NO MATCH");
        //         _floraCharacteristic = nullptr;
        //     }
        // }
        
        floraCharacteristic = floraService->getCharacteristic(_uuid_write_mode);        
    }
    catch (...) {
        // something went wrong
        Serial.println("-- Failed, skipping device");
        _floraClient->disconnect();
        return false;
    }
    if (floraCharacteristic == nullptr) {
        Serial.println("-- Failed, skipping device");
        _floraClient->disconnect();
        return false;
    }

    // write the magic data
    uint8_t buf[2] = {0xA0, 0x1F};
    floraCharacteristic->writeValue(buf, 2, true);

    delay(250);
    return true;
}


BLERemoteService* HAPPluginMiFlora::getFloraService() {
    //BLERemoteService* floraService = nullptr;

    BLERemoteService* floraService = nullptr;        

    try {        
        // BLEUUID serviceUUID(BLE_MIFLORA_SERVICE_UUID);        
        floraService = _floraClient->getService(_serviceUUID);


    // if (_floraClient->isConnected() ) {
    //     for (auto &myPair : *_floraClient->getServices()) {

    //         std::string uuidPadded = std::string(36 - myPair.first.length(), '0') + myPair.first;

    //         Serial.print("UUID: ");
    //         Serial.print(uuidPadded.c_str());
    //         Serial.print(" ?? ");
    //         Serial.print(_serviceUUID.toString().c_str());

    //         if (uuidPadded == _serviceUUID.toString()) {
    //             Serial.println(" - MATCH");
    //             Serial.println(myPair.second->toString().c_str());
    //             floraService = myPair.second;
    //             break;
    //         } else {
    //             Serial.println(" - NO MATCH");  
    //             floraService = nullptr;              
    //         }
    //     } // End of each of the services.
    // }
        
    }
    catch (...) {
        // something went wrong
    }


    if (floraService == nullptr) {
        Serial.println("- Failed to find data service");
        _floraClient->disconnect();
    }
    else {
        Serial.println("- Found data service");
    }

    return floraService;
}

BLEClient* HAPPluginMiFlora::getFloraClient(BLEAddress floraAddress) {
    //BLEClient* floraClient = BLEDevice::createClient();  
    try
    {   
        if (_floraClient != nullptr ){
            if (_floraClient->isConnected()){
                _floraClient->disconnect();
            }

            if (!_floraClient->connect(floraAddress)) {
                Serial.println("- Connection failed, skipping");
                return nullptr;
            }
        }

    }
    catch(...)
    {
       
    }
    

    Serial.println("- Connection to " + String(floraAddress.toString().c_str()) + " successful");
    return _floraClient;
}


HAPConfigValidationResult HAPPluginMiFlora::validateConfig(JsonObject object){

    return HAPPlugin::validateConfig(object);
}

JsonObject HAPPluginMiFlora::getConfigImpl(){
    DynamicJsonDocument doc(1);
	return doc.as<JsonObject>();
}

void HAPPluginMiFlora::setConfigImpl(JsonObject root){

}

#endif

