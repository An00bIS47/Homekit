//
// HAPPluginPCA301.cpp
// Homekit
//
//  Created on: 19.08.2019
//      Author: michael
//

#include "HAPPluginPCA301.hpp"

#include "HAPServer.hpp"

//
// SPI Connections for ESP32:
// 
//    SPI	    MOSI	    MISO	    CLK	        CS
//    VSPI	    GPIO 23	    GPIO 19	    GPIO 18	    GPIO 5      -> default for RFM69X
//    HSPI	    GPIO 13	    GPIO 12	    GPIO 14	    GPIO 15     
// 
// RX Protokoll (Anzeigeeinheit):
// RX: 01 04 07 F8 92 00 00 00 00 00 0E 9F
// RX: 01 04 07 F8 92 01 00 00 00 00 8E E4
// 
// RX: 01 05 07 F8 92 01 AA AA AA AA 77 4A
// 
//       0 1 2   3  4 5  6   7 8   9
//       | | |   |  | |  |   | |   |  
// OK 24 1 4 3  86 44 1 45 252 0   2
// OK 24 1 4 3  86 44 1 45 201 0   3
// OK 24 2 4 2 189 72 1  1 152 2 145    
//       | | |        |  |     |--> 2 Byte: absoluter Verbrauch in kWh (Faktor 1/100) 
//       | | |        |  |--------> 2 Byte: aktueller Verbrauch in W (Faktor 1/10)   
//       | | |        |-----------> 1 Byte: Data [Befehl 04: (01 = rücksetzten des absoluten Verbrauchs), Befehl 05: (00 = aus, 01 = ein)]
//       | | |--------------------> 3 Byte: Adresse 
//       | |----------------------> 1 Byte: Befehl
//       |------------------------> 1 Byte: Kanal
//          
// Interpretation:
// 1 Byte: Kanal
// 1 Byte: Befehl (04 = Abfrage Messwerte, 05 = Schaltbefehl)
// 3 Byte: Adresse (UID) Steckdose
// 1 Byte: Data [Befehl 04: (01 = rücksetzten des absoluten Verbrauchs), Befehl 05: (00 = aus, 01 = ein)]
// 2 Byte: aktueller Verbrauch in W (Faktor 1/10)
// 2 Byte: absoluter Verbrauch in kWh (Faktor 1/100)
// 2 Byte: CRC16 (Polynom 8005h)
// 
// Also ich möchte nochmal auf die Erklärung des Protokoll hinweisen die ich bereits erstellt habe.
// Und ja der aktuelle Wert ist durch 10 zu teilen 
// und der komulierte durch 100. 
// 
// Der komulierte Wert wird durch die Anzeigeeinheit per Befehl zurückgesetzt,
// dieser Reset erfolgt immer um 24:00/0:00Uhr und wird durch die interne Uhr der Anzeigeeinheit gesteuert.
// Hierzu wird der Befehl zum auslesen der Messwerte verwendet, jedoch nicht mit 00 im Datenbereich sondern mit 01.
// 
// my $power        = ($bytes[6]*256 + $bytes[7]) / 10.0;
// my $consumption  = ($bytes[8]*256 + $bytes[9]) / 100.0;
//
//         1 2   3  4 5  6   7 8   9
// OK 24 1 4 3  86 44 1 45 201 0   3
// consumptionTotal:    = ( 0 * 256) +   3 == 3 / 100    =   0,03 kWh
// power:               = (45 * 256) + 201 == 11721 / 10 = 1172,1 W


#define HAP_PCA301_INTERVAL  0
#define HAP_PCA301_POLL_INTERVAL 300    // 1/10 second
#define HAP_PCA301_DEAD_INTERVAL 5000   // 1/10 second

#define HAP_PCA301_QUIET    1

#define VERSION_MAJOR       0
#define VERSION_MINOR       0
#define VERSION_REVISION    3
#define VERSION_BUILD       2

uint8_t HAPPluginPCA301::pca301_sync_values[2] = { 0x2d, 0xd4 }; /**< sync word values */

HAPPluginPCA301::HAPPluginPCA301(){
    _type               = HAP_PLUGIN_TYPE_ACCESSORY;
    _name               = "PCA301";
    _isEnabled          = HAP_PLUGIN_USE_PCA301;
    _interval           = HAP_PCA301_INTERVAL;
    _previousMillis     = 0;    

    _version.major      = VERSION_MAJOR;
    _version.minor      = VERSION_MINOR;
    _version.revision   = VERSION_REVISION;
    _version.build      = VERSION_BUILD;

    _accessory = nullptr;

    numDev      = 0;
    pollIntv    = HAP_PCA301_POLL_INTERVAL;
    deadIntv    = HAP_PCA301_DEAD_INTERVAL;
    quiet       = 1;
    crc         = 0;

    rfm69_crc           = 0;
    rxfill              = 0;
    rfm69_len           = 7;
    rfm69_center_freq   = 868950;
}

bool HAPPluginPCA301::begin(){

    pca301_board_init();
    pca301serial_setup();


    std::string path = "/";
    String tmpName = _name;
    tmpName.toLowerCase();
    path += tmpName.c_str();

    // 
    // Webserver register node for /apu/plugin/_name
    // 
    // Store member function and the instance using std::bind.
    // Callback<void(HTTPRequest*, HTTPResponse*)>::func = std::bind(&HAPPluginPCA301::handleRoot, this, std::placeholders::_1, std::placeholders::_2);

    // // Convert callback-function to c-pointer.
    // void (*c_func)(HTTPRequest*, HTTPResponse*) = static_cast<decltype(c_func)>(Callback<void(HTTPRequest*, HTTPResponse*)>::callback);    
    // _webserver->registerPluginNode(std::string(_name.c_str()), path, "GET", c_func);

    return true;
}


int HAPPluginPCA301::indexOfDevice(HAPPluginPCA301Device* device){
    // Check if element 22 exists in vector
	std::vector<HAPPluginPCA301Device*>::iterator it = std::find(_devices.begin(), _devices.end(), device);
 
	if (it != _devices.end())
	{		
		// Get index of element from iterator
		return std::distance(_devices.begin(), it);		
	} else {
        return -1;
    }

}


void HAPPluginPCA301::handleImpl(bool forced){
    
    // LogD("Handle " + String(__PRETTY_FUNCTION__) + " - interval: " + String(interval()), true);
    rfm69_timer_loop();
    bool updated = pca301serial_loop();

    if (updated){

#if HAP_DEBUG
        LogD("<<< RX: ", false);
        for (int i = 0; i < PCA301_PACKET_LENGTH; i++){
            LogD(rfm69_buf[i], false);
            LogD(" ", false);
        }
        LogD("", true);      
#endif
        // 0 1 2 3   4   5 6 7  8 9
        // 2 4 3 132 244 1 0 10 0 0 
        // 1 4 3 86 44 1 0 7 0 4 
        // 
        // ((0 * 256) + 10) / 10    = 1         -> power
        // ((0 * 256) +  7) / 10    = 0,7       -> power

        // ((0 * 256) +  4) / 100    = 0,04     -> total consumption

        // float currentPower       = ((rfm69_buf[6] * 256) + rfm69_buf[7]) / 10;
        // float totalConsumption   = ((rfm69_buf[8] * 256) + rfm69_buf[9]) / 100;
        bool isOn;

        // uint8_t channel = rfm69_buf[0];      // unused

        if ( rfm69_buf[1] == 0x04){
            // if ((rfm69_buf[6] != 170) && (rfm69_buf[7] != 170) && (rfm69_buf[8] != 170) && (rfm69_buf[9] != 170)){
            if (mem2long(rfm69_buf+6) != 0xAAAAAAAA && mem2long(rfm69_data+6) != 0xFFFFFFFF) {
                uint16_t power          = (rfm69_buf[6] << 8) | rfm69_buf[7];
                uint16_t consumption    = (rfm69_buf[8] << 8) | rfm69_buf[9];                    

                // state - isOn
                rfm69_buf[5] == 0x00 ? isOn = false : isOn = true;

                // values
                float currentPower = power * 0.1;
                float totalConsumption = consumption * 0.01;
                float ampere = currentPower / 230;

#if HAP_DEBUG
                Serial.printf("currentPower:     %.4f\n", currentPower);
                Serial.printf("totalConsumption: %.4f\n", totalConsumption);
                Serial.printf("ampere:           %.4f\n", ampere);

                LogD("PCA301 [", false);
                LogD(rfm69_buf[2], false);
                LogD(" ", false);
                LogD(rfm69_buf[3], false);
                LogD(" ", false);
                LogD(rfm69_buf[4], false);
                LogD("]: ", false);
                LogD("isOn: " + String(isOn), false);
                // Serial.printf("power: %.3f", currentPower);
                // Serial.printf(" - consumption: %.3f\n", totalConsumption);
                LogD(" power: " + String(currentPower), false);
                LogD(" consumption: " + String(totalConsumption), true);
#endif

                
                uint32_t devId = mem2devId(rfm69_buf+2);

                int index = getDevice(devId);

                if (index != -1) {
                    _devices[index]->setPowerState(isOn == true ? "1" : "0");
                    _devices[index]->setCurrentPower(String(currentPower));
                    _devices[index]->setTotalPower(String(totalConsumption));
                }
                
            }
        } else if ( rfm69_buf[1] == 0x05){
            rfm69_buf[5] == 0x00 ? isOn = false : isOn = true;
        }
        

    }        
    
}




HAPAccessory* HAPPluginPCA301::initAccessory(){
	LogD("\nInitializing plugin: HAPPluginPCA301 ...", false);

    // // Create accessory if not already created
    // _accessory = new HAPAccessory();
    // //HAPAccessory::addInfoServiceToAccessory(_accessory, "Builtin LED", "ACME", "LED", "123123123", &identify);
    // auto callbackIdentify = std::bind(&HAPPlugin::identify, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    // HAPAccessory::addInfoServiceToAccessory(_accessory, "PCA301 Bridge", "ACME", "PCA301", "123123123", callbackIdentify, version());


    for (auto& dev : _devices){   
        
        dev->setFakeGatoFactory(_fakeGatoFactory);
        dev->setEventManager(_eventManager);
        
        auto callbackSend = std::bind(&HAPPluginPCA301::sendDeviceCallback, this, std::placeholders::_1, std::placeholders::_2);        
        dev->setPCA301SendCallback(callbackSend);

        _accessorySet->addAccessory(dev->initAccessory());
    }


    // LogD("OK", true);

    // return _accessory;   // -> will be added in configAccessory()
    return nullptr;
}


void HAPPluginPCA301::identify(int iid, bool oldValue, bool newValue) {
    printf("Start Identify pca301 from member\n");
}


void HAPPluginPCA301::sendDeviceCallback(uint32_t devId, char cmd_){
    LogD(HAPServer::timeString() + " " + "HAPPluginPCA301" + "->" + String(__FUNCTION__) + " [>>>] " + "Callback send device " + String(devId) + "  - cmd: " + String(cmd), true);

    sendDevice(getDevice(devId) + 1, cmd_);
    cmd = cmd_;
}



/*
 * Config validation
 */
HAPConfigValidationResult HAPPluginPCA301::validateConfig(JsonObject object){
    
    LogD(String(__PRETTY_FUNCTION__), true);


    /*
        {
            "enabled": true,
            "interval": 0,
            "pollInterval": 30,
            "deadInterval": 500,       
            "centerFrequency": 868950,
            "devices": [
                {
                    "channel": 1,
                    "id": 123456,
                    "name": "test1"
                    "state": true
                }
            ]
        }
     */
    HAPConfigValidationResult result;
    
    result = HAPPlugin::validateConfig(object);
    if (result.valid == false) {
        return result;
    }

    result.valid = false;

    // plugin._name.pollInterval
    if (!object.containsKey("pollInterval") ) {
        result.reason = "plugins." + _name + ".pollInterval is required";
        return result;
    }

    
    if (object.containsKey("pollInterval") && !object["pollInterval"].is<uint16_t>()) {
        result.reason = "plugins." + _name + ".pollInterval is not an integer";
        return result;
    }

    // plugin._name.deadInterval
    if (!object.containsKey("deadInterval") ) {
        result.reason = "plugins." + _name + ".deadInterval is required";
        return result;
    }

    if (object.containsKey("deadInterval") && !object["deadInterval"].is<uint16_t>()) {
        result.reason = "plugins." + _name + ".deadInterval is not an integer";
        return result;
    }

    // optional
    // plugin._name.centerFrequency
    if (object.containsKey("centerFrequency") && !object["centerFrequency"].is<uint32_t>()) {
        result.reason = "plugins." + _name + ".centerFrequency is not an integer";
        return result;
    }   


    // plugin._name.devices
    if (object.containsKey("devices") && !object["devices"].is<JsonArray>()) {
        result.reason = "plugins." + _name + ".devices is not an array";
        return result;
    }

    // plugin._name.devices array
    uint8_t count = 0;
    for( const auto& value : object["devices"].as<JsonArray>() ) {
        
        // plugin._name.devices.count.channel
        if (!value.containsKey("channel") ) {
            result.reason = "plugins." + _name + ".devices." + String(count) + ".channel is required";
            return result;
        }
        if (value.containsKey("channel") && !value["channel"].is<uint8_t>()) {
            result.reason = "plugins." + _name + ".devices." + String(count) + ".channel is not an integer";
            return result;
        }

        // plugin._name.devices.count.id
        if (!value.containsKey("id") ) {
            result.reason = "plugins." + _name + ".devices." + String(count) + ".id is required";
            return result;
        }
        if (value.containsKey("id") && !value["id"].is<uint32_t>()) {
            result.reason = "plugins." + _name + ".devices." + String(count) + ".id is not an integer";
            return result;
        }    

        // optional
        // plugin._name.devices.count.name
        if (value.containsKey("name") && !value["name"].is<const char*>()) {
            result.reason = "plugins." + _name + ".devices." + String(count) + ".name is not a string";
            return result;
        }   

        // plugin._name.devices.count.name - length
        if (strlen(value["name"]) + 1 > HAP_STRING_LENGTH_MAX) {
            result.reason = "plugins." + _name + ".devices." + String(count) + ".name is too long";
            return result;
        }   

        // optional
        // plugin._name.devices.count.state
        if (value.containsKey("name")) {
            if (strlen(value["name"]) + 1 > HAP_STRING_LENGTH_MAX) {
                result.reason = "plugins." + _name + ".devices." + String(count) + ".name is too long";
                return result;
            } 
        }
        
        count++;
    }


    result.valid = true;
    return result;
}

JsonObject HAPPluginPCA301::getConfigImpl(){  

    LogD(String(__PRETTY_FUNCTION__), true);          

    DynamicJsonDocument doc(HAP_ARDUINOJSON_BUFFER_SIZE / 8);
    doc["pollInterval"] = pollIntv;
    doc["deadInterval"] = deadIntv;
    doc["centerFrequency"] = rfm69_center_freq;

    JsonArray devices = doc.createNestedArray("devices");

    for (auto& dev : _devices){
        JsonObject devices_ = devices.createNestedObject();
        devices_["channel"] = dev->channel;
        devices_["id"]      = dev->devId;
        devices_["name"]    = dev->name;
        devices_["state"]   = dev->pState;
    }

    return doc.as<JsonObject>();
}

void HAPPluginPCA301::setConfigImpl(JsonObject root){

    LogD(String(__PRETTY_FUNCTION__), true);
    
    if (root.containsKey("pollInterval")){
        LogD(" -- pollInterval: " + root["pollInterval"].as<String>(), true);        
        pollIntv = root["pollInterval"].as<uint16_t>();
    }

    if (root.containsKey("deadInterval")){        
        LogD(" -- deadInterval: " + root["deadInterval"].as<String>(), true);
        deadIntv = root["deadInterval"].as<uint16_t>();
    }

    if (root.containsKey("centerFrequency")){        
        LogD(" -- centerFrequency: " + root["centerFrequency"].as<String>(), true);
        rfm69_center_freq = root["centerFrequency"].as<uint32_t>();
    }

#if HAP_DEBUG    
    int count = 0;

#endif
    if (root.containsKey("devices")){        
        for (JsonVariant dev : root["devices"].as<JsonArray>()) {

#if HAP_DEBUG            
            LogD(" -- device " + String(count) + ": devId "     + dev["id"].as<String>()        , true);            
            LogD(" -- device " + String(count) + ": name "      + dev["name"].as<String>()      , true);                        
            LogD(" -- device " + String(count) + ": channel "   + dev["channel"].as<String>()   , true);                    
            LogD(" -- device " + String(count) + ": state "     + dev["state"].as<String>()     , true);

            count++;
#endif
            
            HAPPluginPCA301Device* newDevice = new HAPPluginPCA301Device(
                dev["channel"].as<uint8_t>(),
                dev["id"].as<uint32_t>(),
                dev["state"].as<bool>(),
                dev["name"].as<String>()
            );

            int index = indexOfDevice(newDevice);
            if ( index == -1 ){
                _devices.push_back(newDevice);
            } else {
                _devices[index] = newDevice;
            }         
        }
    } 
}


/*****************************************************************************/
/** Board Initialization
 */
void HAPPluginPCA301::pca301_board_init() {    

    /* initialize RFM69 for PCA301 */
    pca301_rfm69_init();

    /* configure IRQ for RFM69 */
    pinMode(PCA301_PIN_INT, INPUT);

    /* enable interrupts */
    rfm69_int_enable();
}

/*****************************************************************************/
/** RFM69 Initialization
 */
void HAPPluginPCA301::pca301_rfm69_init(){
    /* configure RFM69 SPI */
    rfm69_init(VSPI, PCA301_PIN_SPI_CLK, PCA301_PIN_SPI_MISO, PCA301_PIN_SPI_MOSI, PCA301_PIN_SPI_SS, RFM69_IS_HW);

    /* put transceiver in standby mode */
    rfm69_opmode_set(RFM69_OPMODE_STANDBY);

    /* frequency: 868.950 MHz */
    rfm69_freq_carrier_khz(PCA301_FREQ_CARRIER_KHZ);

    /* bitrate: 6.631 kb/s */
    rfm69_bitrate_bs(PCA301_BITRATE_BS);

    /* configure RX and TX interrupt generators */
    rfm69_dio_mapping_rx(0, RFM69_DIO0_RX_PAYLOADREADY_TX_TXREADY);
    rfm69_dio_mapping_tx(0, RFM69_DIO0_RX_CRCOK_TX_PACKETSENT);

    /* disable CLKOUT to save power */
    rfm69_clkout(RFM69_CLKOUT_OFF);

    /* configure CRC */
    rfm69_crc_on(false);
    rfm69_crc_auto_clear_off(true);

    /* set payload length */
    rfm69_payload_length(12);

    /* configure sync word */
    rfm69_sync_word(2, pca301_sync_values);
    rfm69_sync_on(true);

    /* RX bandwidth exponent */
    rfm69_rx_bw_exp(2);

    /* RSSI threshold */
    rfm69_rssi_threshold(0xdc);

    /* variable length packet format */
    rfm69_packet_format_var_len(false);

    /* TX start condition */
    rfm69_tx_start_cond(RFM69_FIFO_NOT_EMPTY);

    /* set frequency deviation in Hz */
    rfm69_fdev_hz(45000);

    /* enable receiver mode */
    rfm69_opmode_set(RFM69_OPMODE_RX);

    /* clear fifo */
    rfm69_fifo_clear();
}


//- report pcaConf ---------------------------------------------------------------------------------
void HAPPluginPCA301::reportConf(uint8_t repMode){
    
    int counter = 1;
    for (auto& dev : _devices){
        switch (repMode) {
            case 1:                
                Serial.print("L <node>:");
                Serial.print(NODEID);
                Serial.print(" <cmd idx>");
                Serial.print(counter++);
                Serial.print(' ');
                break;
            case 2:
                Serial.print("R ");                
                break;
            default:
                break;
        }
        Serial.print(" <retries>");
        Serial.print(dev->retries);
        Serial.print(" : ");
        Serial.print(" <channel>");
        Serial.print(dev->channel);
        Serial.print(" 4 ");
        Serial.print(" <address>");
        Serial.print((byte)(dev->devId >> 16));
        Serial.print(' ');
        Serial.print((byte)(dev->devId >> 8));
        Serial.print(' ');
        Serial.print((byte)(dev->devId));
        Serial.print(' ');
        Serial.print(" <state>");
        Serial.print(dev->pState);
        Serial.print(' ');
        Serial.print(" <power>");
        Serial.print((byte)(dev->pNow >> 8));
        Serial.print(' ');
        Serial.print((byte)(dev->pNow));
        Serial.print(' ');
        Serial.print(" <pwrTtl>");
        Serial.print((byte)(dev->pTtl >> 8));
        Serial.print(' ');
        Serial.print((byte)(dev->pTtl));
        Serial.println();

    }  
}

//- modify pcaConf ---------------------------------------------------------------------------------
void HAPPluginPCA301::modifyConf(volatile uint8_t value) {
    switch (value) {
        case 0: fillConf();  break;
        case 1: loadConf();  break;
        case 2: saveConf();  break;
        case 3: eraseConf(); break;
    };
}

//- pcaTask ----------------------------------------------------------------------------------------
void HAPPluginPCA301::pcaTask() {

    for (int i = 0; i < _devices.size(); i++){
        
        if (millis() / 100 > _devices[i]->nextTX) {
            if (_devices[i]->retries <= 255) {
                _devices[i]->retries += 1;
            }

            if (_devices[i]->retries < PCA_MAXRETRIES) {
                _devices[i]->nextTX = millis() / 100 + random(0,30) + 10;
            } else {
                _devices[i]->nextTX = millis() / 100 + random(0,30) + deadIntv;
            }
                        
            sendDevice(i + 1, 'p');  
            cmd = 'p';  
        }
    }
}
  
//- send device ------------------------------------------------------------------------------------
//  index 1 based
void HAPPluginPCA301::sendDevice(uint8_t devPtr, char cmd) {
    
    LogD(HAPServer::timeString() + " " + "HAPPluginPCA301" + "->" + String(__FUNCTION__) + " [>>>] " + "Sending " + String(devPtr) + "  - cmd: " + String(cmd), true);

    if (devPtr - 1 < _devices.size()) {        

        pBuf[0] = _devices[devPtr - 1]->channel;
        switch (cmd) {
            case    'p': 
                // Serial.println("Poll device:");
                pBuf[1] = 4;  
                break;   // poll
            case    'j': 
                // Serial.println("Pair device:");
                pBuf[1] = 17; 
                break;   // pair
            default    : 
                pBuf[1] = 5;   // switch
        }

        pBuf[2] = _devices[devPtr - 1]->devId >> 16;
        pBuf[3] = _devices[devPtr - 1]->devId >> 8;
        pBuf[4] = _devices[devPtr - 1]->devId;

        if (cmd == 'e') {            
            pBuf[5] = 1;  // turn "on" (with byte 1 set to 5)
        } else {
            pBuf[5] = 0;  // turn "off" (with byte 1 set to 5)
        }
        pBuf[6] = pBuf[7] = pBuf[8] = pBuf[9] = 0xFF;

        sendLen = 10;

        if (!quiet) {
            Serial.print("TX ");
            Serial.print(NODEID);
            for (byte i = 0; i < sendLen; i++) {
                Serial.print(' ');
                showByte(pBuf[i]);
            }
            Serial.println();
        }      
    }   
}

//- set next tx time for a given device ------------------------------------------------------------
//  index 0 based
void HAPPluginPCA301::setNextTX(uint32_t devId, uint8_t nextTX) {
    int devPtr = getDevice(devId);
    if (devPtr != -1)
        _devices[devPtr]->nextTX  = millis() / 100 + nextTX;
    return;
}

//- analyze packet ---------------------------------------------------------------------------------
void HAPPluginPCA301::analyzePacket(){

    uint32_t devId = mem2devId(rfm69_buf+2);

    int devPtr = getDevice(devId); // = vector index + 1 (1 based)

    bool confChanged = false;

    //- unknown device? add it to pcaConf ------------------------------------------------------------
    if (devPtr == -1) {
        // devPtr = ++pcaConf.numDev;
        // pcaConf.pcaDev[devPtr-1].devId = devId;
        // //- is this device already paired with a handheld display unit? --------------------------------
        // if (rfm69_buf[0]) {
        //     //- device is paired to handheld display unit, therefore use same channel --------------------
        //     pcaConf.pcaDev[devPtr-1].channel = rfm69_buf[0];
        // } else {
        //     //- device is not paired to an handheld display unit, assign a free channel ------------------
        //     pcaConf.pcaDev[devPtr-1].channel = pcaConf.numDev;            
        // }

        // configAccessory(devPtr-1);

        HAPPluginPCA301Device* newDevice = new HAPPluginPCA301Device();
        newDevice->devId = devId;
        if (rfm69_buf[0]) {
            //- device is paired to handheld display unit, therefore use same channel --------------------
            newDevice->channel = rfm69_buf[0];
        } else {
        //     //- device is not paired to an handheld display unit, assign a free channel ------------------
            newDevice->channel = _devices.size() + 1;            
        }

        // ToDo: Configure PCA 301 Accessory + Fakegato
        // configAccessory(devPtr-1);
        newDevice->setFakeGatoFactory(_fakeGatoFactory);
        newDevice->setEventManager(_eventManager);
        
        _devices.push_back(newDevice);

        _accessorySet->addAccessory(newDevice->initAccessory());

        
        confChanged = true;

    } else if (rfm69_buf[0] && _devices[devPtr]->channel != rfm69_data[0]) {
        //- known device, but used channel is different -> update config in memory -------------------
        _devices[devPtr]->channel = rfm69_buf[0];
        confChanged = true;
    }

    //- update dynamic values ------------------------------------------------------------------------
    if (mem2long(rfm69_buf+6) != 0xAAAAAAAA && mem2long(rfm69_data+6) != 0xFFFFFFFF) {
        _devices[devPtr]->pState  = (bool) rfm69_buf[5]; 
        _devices[devPtr]->pNow    = mem2word(rfm69_buf+6);
        _devices[devPtr]->pTtl    = mem2word(rfm69_buf+8);
        _devices[devPtr]->nextTX  = millis() / 100 + random(0,30) + pollIntv;
        _devices[devPtr]->retries = 0;
    } else if (rfm69_buf[1] == 5) {
        // switch command, trigger poll
        _devices[devPtr]->nextTX  = millis() / 100 + 5;
    }

    //- pairing request received? --------------------------------------------------------------------
    if (!rfm69_buf[0]) {
        if (!quiet) {
            Serial.print("#PREQ ");
            Serial.println(devId);
        }
        delay(70);                   // there's a timing issue while pairing, lose a bit of time
        sendDevice(devPtr + 1, 'j');
        cmd = 'j';
    }

    //- save config to EEPROM ------------------------------------------------------------------------
    if (confChanged) {
        // ToDo: Change config accessory ?
        saveConf();
    }
        

} // analyzePacket

//- lookup device ----------------------------------------------------------------------------------
// get index of device by devId 
//  0 based
int HAPPluginPCA301::getDevice(uint32_t devId) {
    
    for (int i=0; i < _devices.size(); i++){
        if (_devices[i]->devId == devId) {
             return i;    // device found
        }
    }
    
    return -1;
}

//- get devId --------------------------------------------------------------------------------------
uint32_t HAPPluginPCA301::mem2devId(volatile uint8_t * data) {
    return (uint32_t)data[0] << 16 | (uint32_t)data[1] << 8 | (uint32_t)data[2];
}

//- mem2word ---------------------------------------------------------------------------------------
uint16_t HAPPluginPCA301::mem2word(volatile uint8_t * data) {
    return data[0] << 8 | data[1];
}

//- mem2long ---------------------------------------------------------------------------------------
uint32_t HAPPluginPCA301::mem2long(volatile uint8_t * data) {
    return (uint32_t)mem2word(data+0) << 16 | mem2word(data+2);
}


//- showNibble -------------------------------------------------------------------------------------
void HAPPluginPCA301::showNibble(byte nibble) {
    char c = '0' + (nibble & 0x0F);
    if (c > '9')
        c += 7;
    Serial.print(c);
}

//- showByte ---------------------------------------------------------------------------------------
void HAPPluginPCA301::showByte (byte value) {
    Serial.print(value);
}


//- showHelp ---------------------------------------------------------------------------------------
void HAPPluginPCA301::showHelp() {
    // Serial.print("\n[");
    // Serial.print(PROGNAME);
    // Serial.print('.');
    // Serial.print(PROGVERS);
    // Serial.println(']');
    Serial.println("\n");
    Serial.println("Available commands:");
    Serial.println("     ..,.. s    - send data packet");
    Serial.println("           l    - list devices");
    Serial.println("       <n> c    - config (0=fill, 1=load, 2=save, 3=erase)");
    Serial.println("       <n> d    - turn off device <n>");
    Serial.println("       <n> e    - turn on device <n>");
    Serial.println("  0x<hhhh> h    - set center frequency offset (Example: 0x03B6 => 868.950MHz)");
    Serial.println("                  note: leading zeros must be entered");
    Serial.println("       <n> p    - poll device <n>");
    Serial.println("       <n> r    - list recordings");
    Serial.println("       <n> q    - quiet mode (1=suppress TX and bad packets)");
    // Serial.println("       <n> v    - version and configuration report");
    Serial.println();
}

//- handleInput ------------------------------------------------------------------------------------
void HAPPluginPCA301::handleInput (char c) {
    if (freq.startsWith("0x") && c != 'h') {
        if (('A' <= c && c <='F') || ('0' <= c && c <= '9')) {
            freq += c;
        }
    } else if ('0' <= c && c <= '9') {
        value = 10 * value + c - '0';
    } else if (c == ',') {
        if (top < sizeof stack) {
            stack[top++] = value;
        }
            
        value = 0;
    } else if (c == 'x') {
        freq = String("0x");
        value = 0;
    } else if ('a' <= c && c <='w') {      
        switch (c) {
            default:
                showHelp();
                break;
            case 'l':     // list known devices                
                reportConf(1);
                break;

            case 'q':     // turn quiet mode on or off (don't report TX and bad packets)
                Serial.print("Toggle quit mode");
                if (value == 1) {
                    Serial.println(" ON");
                } else {
                    Serial.println(" OFF");
                }
                quiet = value;
                break;

            case 'r':     // list recordings
                reportConf(2);
                break;

            case 's':     // send packet
                if (top < sizeof stack) {
                    stack[top++] = value;
                    sendLen      = top;
                    cmd          = c;
                    memcpy(pBuf, stack, top);
                    if (sendLen == 10 && pBuf[1] == 5)
                        setNextTX(mem2devId(pBuf+2), 10);
                    if (!quiet) {
                        Serial.print("TX ");
                        Serial.print(NODEID);
                        for (byte i = 0; i < sendLen; i++) {
                            Serial.print(' ');
                            showByte(pBuf[i]);
                        }
                        Serial.println();
                    }
                } else
                    top = 0;
                break;

            // case 'v':     // report version and configuration parameters
            //     displayVersion(1);
            //     break;

            case 'd':     // turn a device off (disable)
            case 'e':     // turn a device on (enable)
            case 'p':     // poll a device
                sendDevice(value,c);
                cmd = c;
                break;
            
            case 'c':     // config options
                modifyConf(value);
                break;
            
            case 'h': // modify and display RFM69 Frequency register
                Serial.print("> FREQ set to: ");
                rfm69_center_freq = RF_FREQ_BASE + hexToUInt16(freq);
                rfm69_freq_carrier_khz(rfm69_center_freq);
                Serial.println(rfm69_center_freq);
                freq = String("");
                break;
        }
        value = top = 0;
        memset(stack, 0, sizeof stack);
    } else if (c == '+' || c == '-' || c == '#') {
        switch (c) {
            case '+': // modify and display RFM69 Frequency register
            case '-': // modify and display RFM69 Frequency register
                Serial.print("> FREQ");
                if (c == '+') {
                    rfm69_center_freq += 1;
                } else {
                    rfm69_center_freq -= 1;
                }
                    
                rfm69_freq_carrier_khz(rfm69_center_freq);
                Serial.print(c);
                Serial.print(": "); 
                Serial.println(rfm69_center_freq);
                break;

            case '#': // test
                break;

        }

        value = 0;
        freq = String("");
    } else if (' ' < c && c < 'A') {
        showHelp();
    }
        
}

uint16_t HAPPluginPCA301::hexToUInt16(String hexString) {
  
    uint16_t UInt16Value = 0;
    int nextInt;
  
    for (int i = 0; i < hexString.length(); i++) {
        
        nextInt = int(hexString.charAt(i));
        if (nextInt >= 48 && nextInt <= 57) nextInt = map(nextInt, 48, 57, 0, 9);
        if (nextInt >= 65 && nextInt <= 70) nextInt = map(nextInt, 65, 70, 10, 15);
        if (nextInt >= 97 && nextInt <= 102) nextInt = map(nextInt, 97, 102, 10, 15);
        nextInt = constrain(nextInt, 0, 15);
        
        UInt16Value = (UInt16Value * 16) + nextInt;
    }
  
  return UInt16Value;
}

// void HAPPluginPCA301::displayVersion(uint8_t newline) {
//     Serial.print("\n[");
//     Serial.print(PROGNAME);
//     Serial.print('.');
//     Serial.print(PROGVERS);
//     Serial.print(']');  
//     if (newline!=0){
//         Serial.println();
//     }        
// }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// M A I N
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//- setup ------------------------------------------------------------------------------------------
void HAPPluginPCA301::pca301serial_setup() {

    // available cli options
#if HAP_DEBUG    
    showHelp();
#endif
    // try loading config from EEPROM. if CRC does not match, use blank default config
    // if (!loadConf()){
    //     fillConf();
    // }        


    // quiet is default
    quiet  = HAP_PCA301_QUIET;


}


//- loop -------------------------------------------------------------------------------------------
void HAPPluginPCA301::pca301serial_loop_pre() {
    uint16_t crc;

    if (rfm69_rx_avail()) {

        rfm69_crc = 0;
        while (true == rfm69_rx_avail()) {
            uint8_t in = rfm69_fifo_data();

            rfm69_buf[rxfill++] = in;
            if (rxfill <= 10) {
                rfm69_crc = crc16_pca301_update(rfm69_crc, in);
            }
        }

        /* compare CRC */
        if (rxfill) {
            crc = (rfm69_buf[10] << 8) | rfm69_buf[11];
            if (crc == rfm69_crc) {
                rfm69_crc = 0;
            }
        }
    }
}


//- loop -------------------------------------------------------------------------------------------
bool HAPPluginPCA301::pca301serial_loop() {
    uint8_t cnt;
    bool updated = false;

    pca301serial_loop_pre();

    if (Serial.available()) {
        handleInput(Serial.read());
    }

    if (!cmd) {
        pcaTask();                 // check tasks
    }

    if ((RFM69_OPMODE_RX == rfm69_opmode_get()) && rxfill) {

        /* clear fifo */
        rfm69_fifo_clear();

        if (rfm69_len > RFM69_MAXDATA) {
            rfm69_crc = 1;   // force bad crc if packet length is invalid
            Serial.println("bad CRC");
        }

        
        if (rfm69_crc == 0) {

            // in quiet mode, suppress as much packets as possible from non-PCA301 transmitters
            if (quiet && rfm69_buf[0] != 0) {
                // quiet mode and not a pairing request
                if (mem2long(rfm69_buf+6) == 0xFFFFFFFF) {
                    // originator is another JeeLink
                    rxfill = 0;
                    rfm69_crc = 0;
                    return false;
                }
                if (rfm69_buf[1] != 5 && mem2long(rfm69_data+6) == 0xAAAAAAAA) {
                    // originator is a hardware display unit
                    rxfill = 0;
                    rfm69_crc = 0;
                    return false;
                }
                // all non PCA301 packets filtered EXCEPT switch command from hardware display unit      
            }
        
            Serial.print("OK");
            updated = true;
        } else {
            if (quiet) {     // don't report bad packets in quiet mode
                rxfill = 0;
                rfm69_crc = 0;
                return false;
            }
            Serial.print(" ?");
        }

        Serial.print(' ');
        Serial.print(NODEID);

        // FHEM quick fix - unpaired devices get listed with channel 0
        Serial.print(' ');
        if (rfm69_buf[0] == 0) {
            showByte(pBuf[1]);
        } else {
            showByte(rfm69_buf[0]);
        }

        for (uint8_t i = 1; i < PCA301_PACKET_LENGTH; ++i) {
            Serial.print(' ');
            showByte(rfm69_buf[i]);
        }

        Serial.println();
        

        if (rfm69_crc == 0) {
            analyzePacket();
        }
            

        rxfill = 0;
        rfm69_crc = 0;
    }

    if (cmd) {        

        /* calculate CRC */
        rfm69_crc = 0;
        for (cnt = 0; cnt < sendLen; cnt++) {
            rfm69_crc = crc16_pca301_update(rfm69_crc, pBuf[cnt]);
        }

        /* add CRC to data stream */
        pBuf[sendLen++] = rfm69_crc >> 8;
        pBuf[sendLen++] = rfm69_crc & 0xff;

        rfm69_send(sendLen, pBuf);
        cmd = 0;
        sendLen = 0;        
    }

    return updated;
}


//- load config from EEPROM - returns 1 if valid config was found, otherwise 0
bool HAPPluginPCA301::loadConf() {
    LogD("Loading config", true);

    return false;

#if 0    
    Serial.println("Loading Config");
    uint16_t len   = sizeof(pcaConf);
    byte *pPtrByte = (byte*)&pcaConf;        // byte Ptr to pcaConf
    eeprom_crc     = 0;
    //   eeprom_read_block(&pcaConf, (void *) 0, len);

    for (int i=0; i < (len - 2); i++) {
        eeprom_crc = crc16_pca301_update(eeprom_crc, *pPtrByte);
        pPtrByte++;
    }

    // valid config in EEPROM?
    if (eeprom_crc == pcaConf.crc) {
        // valid config found, reset dynamic settings
        for (int i = 0; i < pcaConf.numDev; i++) {
            pcaConf.pcaDev[i].pNow    = 0;
            pcaConf.pcaDev[i].pTtl    = 0;
            pcaConf.pcaDev[i].nextTX  = 0;
            pcaConf.pcaDev[i].retries = 0;
        }
        return 1;
    } else {
        // invalid crc
        return 0;
    }

    return 0;
#endif
}

// save config to EEPROM
void HAPPluginPCA301::saveConf() {
    LogD(String(__PRETTY_FUNCTION__), true);

    // ToDo: Save config

    // uint16_t len = sizeof(pcaConf);
    // byte *pPtrByte = (byte*)&pcaConf;        // byte Ptr to pcaConf

    // eeprom_crc = 0;
    // for (int i=0; i < (len - 2); i++) {
    //     eeprom_crc = crc16_pca301_update(eeprom_crc, *pPtrByte);
    //     pPtrByte++;
    // }
    // pcaConf.crc = eeprom_crc;

#if HAP_DEBUG
    // serializeJsonPretty(getConfigImpl(), Serial);	
    struct HAPEvent event = HAPEvent();												
	_eventManager->queueEvent( EventManager::kEventUpdatedConfig, event);
#endif    

//   eeprom_write_block(&pcaConf, (void *) 0, len);
}

// erase config
void HAPPluginPCA301::eraseConf() {
    LogD(String(__PRETTY_FUNCTION__), true);
    
    //  ToDo : add erase config ?
    // pcaConf.numDev = 0;
}

//- fill config ------------------------------------------------------------------------------------
void HAPPluginPCA301::fillConf() {
    LogD(String(__PRETTY_FUNCTION__), true);

    // ToDo: Add Fill config ???
//     pcaConf.numDev     = 0;
//     pcaConf.pollIntv   = HAP_PCA301_POLL_INTERVAL;        // default poll interval in 1/10th seconds
//     pcaConf.deadIntv   = 3000;                            // dead device poll retry interval in 1/10th seconds

// #if HAP_DEBUG    
//     pcaConf.quiet      = 0;                               // quiet, 1=suppress TX and bad packets
// #else
//     pcaConf.quiet      = 1;                               // quiet, 1=suppress TX and bad packets
// #endif

//     pcaConf.pcaDev[0]  = (struct_pcaDev){1 ,0xAAAAA, 0, 0, 0, 0, 0, 0, 0, 0, 0, ""};    // device 1
//     pcaConf.pcaDev[1]  = (struct_pcaDev){2 ,0xBBBBB, 0, 0, 0, 0, 0, 0, 0, 0, 0, ""};    // device 2
}


uint16_t HAPPluginPCA301::crc16_pca301_update(uint16_t crc, uint8_t data) {
    int i;
    crc = crc ^ ((uint16_t)data << 8);
    for (i=0; i<8; i++) {
        if (crc & 0x8000)
        crc = (crc << 1) ^ 0x8005;
        else
        crc <<= 1;
    }
    return crc;
}




// void HAPPluginPCA301::handleRoot(HTTPRequest * req, HTTPResponse * res){    
//     req->discardRequestBody();
//     res->setStatusCode(200);
//     res->setStatusText("OK");
//     res->setHeader("Content-Type", "text/html");
//     res->printf("<p><strong>Hello from %s plugin</p>\n", _name.c_str());

//     res->printf("center frequency: %d\n", rfm69_center_freq);    
// }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// E N D
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -



/*****************************************************************************/
/** RFM69 Interrupt Enable 
 * -> declared in rfm69_funky.h
 */
void rfm69_int_enable(){
    attachInterrupt(digitalPinToInterrupt(PCA301_PIN_INT), rfm69_isr, RISING);
}


/*****************************************************************************/
/** RFM69 Interrupt Disable
 * -> declared in rfm69_funky.h
 */
void rfm69_int_disable(){
    detachInterrupt(digitalPinToInterrupt(PCA301_PIN_INT));
}



