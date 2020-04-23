//
// HAPPluginRCSwitch.cpp
// Homekit
//
//  Created on: 20.12.2019
//      Author: michael
//

#include "HAPPluginRCSwitch.hpp"
#include "HAPServer.hpp"
#include "HAPWebServerBodyParserMultipart.hpp"
#include "HAPWebServerBodyParserURLEncoded.hpp"

#if !HAP_WEBSERVER_USE_SPIFFS  
#include "HAPWebServerFiles.hpp"
#endif

#define VERSION_MAJOR       0
#define VERSION_MINOR       0
#define VERSION_REVISION    3
#define VERSION_BUILD       0

#define RCSWITCH_PIN    33
#define HAP_PLUGIN_RCSWITCH_INTERVAL 1000

HAPPluginRCSwitch::HAPPluginRCSwitch(){
    _type           = HAP_PLUGIN_TYPE_ACCESSORY;
	_name           = "RCSwitch";
	_isEnabled      = HAP_PLUGIN_USE_RCSWITCH;
	_interval       = HAP_PLUGIN_RCSWITCH_INTERVAL;
	_previousMillis = 0;

    _version.major      = VERSION_MAJOR;
    _version.minor      = VERSION_MINOR;
    _version.revision   = VERSION_REVISION;
    _version.build      = VERSION_BUILD;


    _newDevice  = nullptr;
}


HAPPluginRCSwitch::HAPPluginRCSwitch(String name_){
    _name            = name_;
}


bool HAPPluginRCSwitch::begin(){

    // Transmitter is connected to Arduino Pin #10  
    _rcSwitch.enableTransmit(RCSWITCH_PIN);

    return true;
}

uint32_t HAPPluginRCSwitch::bitStringToUInt32(char* input_binary_string){
    return strtol(input_binary_string, NULL, 2);
}

String HAPPluginRCSwitch::uint32ToBitString(uint32_t dec){
    return String(dec, BIN);
}

HAPAccessory* HAPPluginRCSwitch::initAccessory() {
	LogD("\nInitializing plugin: HAPPluginRCSwitch ...", false);

    // HAPPluginRCSwitchDevice* newDevice = new HAPPluginRCSwitchDevice(
    //     17,
    //     16,
    //     "Intertechno 1"
    // );
    // _devices.push_back(newDevice);

    // newDevice = new HAPPluginRCSwitchDevice(
    //     17,
    //     8,
    //     "Intertechno 2"
    // );
    // _devices.push_back(newDevice);

    for (auto& dev : _devices){           
        dev->setFakeGatoFactory(_fakeGatoFactory);
        dev->setEventManager(_eventManager);
        
        auto callbackSend = std::bind(&HAPPluginRCSwitch::sendDeviceCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);        
        dev->setRCSwitchSendCallback(callbackSend);

        _accessorySet->addAccessory(dev->initAccessory());
    }

    return nullptr;
}

void HAPPluginRCSwitch::setValue(int iid, String oldValue, String newValue){

}

void HAPPluginRCSwitch::identify(bool oldValue, bool newValue){

}

void HAPPluginRCSwitch::handleImpl(bool forced){

}	

HAPConfigValidationResult HAPPluginRCSwitch::validateConfig(JsonObject object){

    /*
        {
            "enabled": true,
            "interval": 1000,
            "devices": [
                {
                    "houseAddress": 17,
                    "deviceAddress": 16,
                    "name": "test1"
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

       // plugin._name.devices
    if (object.containsKey("devices") && !object["devices"].is<JsonArray>()) {
        result.reason = "plugins." + _name + ".devices is not an array";
        return result;
    }

    // plugin._name.devices array
    uint8_t count = 0;
    for( const auto& value : object["devices"].as<JsonArray>() ) {
        
        // plugin._name.devices.count.house
        if (!value.containsKey("houseAddress") ) {
            result.reason = "plugins." + _name + ".devices." + String(count) + ".houseAddress is required";
            return result;
        }
        if (value.containsKey("houseAddress") && !value["houseAddress"].is<uint8_t>()) {
            result.reason = "plugins." + _name + ".devices." + String(count) + ".houseAddress is not an integer";
            return result;
        }

        // plugin._name.devices.count.device
        if (!value.containsKey("deviceAddress") ) {
            result.reason = "plugins." + _name + ".devices." + String(count) + ".deviceAddress is required";
            return result;
        }
        if (value.containsKey("deviceAddress") && !value["deviceAddress"].is<uint8_t>()) {
            result.reason = "plugins." + _name + ".devices." + String(count) + ".deviceAddress is not an integer";
            return result;
        }    

        // optional
        // plugin._name.devices.count.name
        if (value.containsKey("name") && !value["name"].is<const char*>()) {
            result.reason = "plugins." + _name + ".devices." + String(count) + ".name is not a string";
            return result;
        }   

        // plugin._name.devices.count.name - length
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

JsonObject HAPPluginRCSwitch::getConfigImpl(){
    DynamicJsonDocument doc(HAP_ARDUINOJSON_BUFFER_SIZE / 8);
    JsonArray devices = doc.createNestedArray("devices");

    for (auto& dev : _devices){
        JsonObject devices_ = devices.createNestedObject();
        devices_["houseAddress"]   = dev->houseAddress;
        devices_["deviceAddress"]  = dev->deviceAddress;
        devices_["name"]    = dev->name;
    }

    return doc.as<JsonObject>();
}

void HAPPluginRCSwitch::setConfigImpl(JsonObject root){
#if HAP_DEBUG    
    int count = 0;

#endif
    if (root.containsKey("devices")){        
        for (JsonVariant dev : root["devices"].as<JsonArray>()) {

#if HAP_DEBUG                                    
            LogD(" -- device " + String(count) + ": house "     + dev["houseAddress"].as<String>()   , true);                    
            LogD(" -- device " + String(count) + ": device "    + dev["deviceAddress"].as<String>()        , true);            
            LogD(" -- device " + String(count) + ": name "      + dev["name"].as<String>()      , true);                        

            count++;
#endif
            
            HAPPluginRCSwitchDevice* newDevice = new HAPPluginRCSwitchDevice(
                dev["houseAddress"].as<uint8_t>(),
                dev["deviceAddress"].as<uint8_t>(),
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

int HAPPluginRCSwitch::indexOfDevice(HAPPluginRCSwitchDevice* device){
    // Check if element 22 exists in vector
	std::vector<HAPPluginRCSwitchDevice*>::iterator it = std::find(_devices.begin(), _devices.end(), device);
 
	if (it != _devices.end())
	{		
		// Get index of element from iterator
		return std::distance(_devices.begin(), it);		
	} else {
        return -1;
    }
}


void HAPPluginRCSwitch::prependZeros(char *dest, String src, uint8_t width) {
    size_t len = src.length();

    if (len >= width) {
        strcpy(dest, src.c_str());
    } else {
        sprintf(dest, "%0*d%s", (int) (width - len), 0, src.c_str());
    }
    
}

void HAPPluginRCSwitch::sendDeviceCallback(uint8_t houseAddress_, uint8_t deviceAddress_, bool on_){
    
    char houseCode[6];
    char deviceCode[6];

    prependZeros(houseCode, String(houseAddress_, BIN), 5);
    prependZeros(deviceCode, String(deviceAddress_, BIN), 5);


    LogD(HAPServer::timeString() + " " + "HAPPluginRCSwitch" + "->" + String(__FUNCTION__) + " [>>>] " + "Callback send device " + String(houseCode) + ":" + String(deviceCode) + "  - state: " + String(on_), true);

    if (on_ == true){
        _rcSwitch.switchOn(houseCode, deviceCode);
    } else {
        _rcSwitch.switchOff(houseCode, deviceCode);
    }
}


void HAPPluginRCSwitch::handleHTTPGet(HTTPRequest * req, HTTPResponse * res){        
    // template processing    
    auto callback = std::bind(&HAPPluginRCSwitch::handleHTTPGetKeyProcessor, this, std::placeholders::_1, std::placeholders::_2);
    
    
#if HAP_WEBSERVER_USE_SPIFFS        
    HAPWebServerTemplateProcessor::processAndSend(res, "/index.html", callback);
#else
    HAPWebServerTemplateProcessor::processAndSendEmbedded(res, html_template_index_start, html_template_index_end, callback);
#endif  

}


void HAPPluginRCSwitch::handleHTTPGetKeyProcessor(const String& key, HTTPResponse * res){
    if (key == "VAR_TITLE") {
        res->print("Plugins - " + _name);
    } else if (key == "VAR_NAVIGATION") {
        res->print(HAPWebServer::buildNavigation());
    } else if (key == "VAR_CONTENT") {        
        res->print("<div class=\"pure-u-1 pure-u-md-1\"><p>Paired Devices:</p>");
        res->print("<table class=\"pure-table pure-table-horizontal\">");
        res->print("<thead> <tr> <th>Name</th> <th>House Code</th> <th>Device Code</th> </tr> </thead>");
        res->print("<tbody>");
        res->print("");
        for (auto& dev : _devices){ 

            char houseCode[6];
            char deviceCode[6];

            prependZeros(houseCode, String(dev->houseAddress, BIN), 5);
            prependZeros(deviceCode, String(dev->deviceAddress, BIN), 5);

            res->printf("<tr><td>%s</td><td>%s</td><td>%s</td></tr>\n", dev->name.c_str(),houseCode ,deviceCode);
        }
        res->print("</tbody>");
        res->print("</table>");
        res->print("</div>");
        
        res->print("<br>");
        res->print("<br>");


        // Add new device        
        res->print("<div class=\"pure-u-1 pure-u-md-1\">");
        res->print("<form class=\"pure-form\" action=\"/plugin/rcswitch\" method=\"POST\">");
        res->print(" <fieldset>");
        res->print("<legend>Add another outlet</legend>");
        res->print("<input type=\"text\" id=\"name\" placeholder=\"Name\">");
        res->print("<input type=\"text\" id=\"houseAddress\" placeholder=\"System Address\">");
        res->print("<input type=\"text\" id=\"deviceAddress\" placeholder=\"Device Address\">");
        res->print("<button type=\"submit\" class=\"pure-button pure-button-primary\">Add</button>");
        res->print("</fieldset>");
        res->print("</form>");
        res->print("</div>");

    } else {
        res->print("");
    }
}

HAPConfigValidationResult HAPPluginRCSwitch::validateName(const char* name){
    HAPConfigValidationResult result;
    result.valid = true;

    if (name == 0) {                    
        result.reason = "The length of the name is 0";
        result.valid = false;
    }

    return result;
}

HAPConfigValidationResult HAPPluginRCSwitch::validateHouseAddress(const char* houseAddress){
    HAPConfigValidationResult result;
    result.valid = true;

    if (strlen(houseAddress) < 5){
        result.reason = "houseAddress must be 5 digits long!";
        result.valid = false;
        return result;
    }

    for (int i = 0; i < strlen(houseAddress); i++){
        if ( (houseAddress[i] == '0' ) || ( houseAddress[i] == '1') ) {

        } else {
            result.reason = "houseAddress must be a binary string!";
            result.valid = false;
            return result;
        }
    }

    return result;
}


HAPConfigValidationResult HAPPluginRCSwitch::validateDeviceAddress(const char* deviceAddress){
    HAPConfigValidationResult result;
    result.valid = true;

    if (strlen(deviceAddress) < 5){
        result.reason = "deviceAddress must 5 digits long!";
        result.valid = false;
        return result;
    }

    for (int i = 0; i < strlen(deviceAddress); i++){
        if ( (deviceAddress[i] == '0' ) || ( deviceAddress[i] == '1') ) {

        } else {
            result.reason = "deviceAddress must be a binary string!";
            result.valid = false;
            return result;
        }
    }

    
    return result;
}

void HAPPluginRCSwitch::handleHTTPPost(HTTPRequest * req, HTTPResponse * res){    


    if (req->getHeader("Content-Type") == "application/x-www-form-urlencoded") {
        HAPWebServerBodyParserURLEncoded urlParser;
        std::vector<std::pair<std::string, std::string>> parameters = urlParser.processAndParse(req);

        if (_newDevice == nullptr) {
            _newDevice = new HAPPluginRCSwitchDevice();
        }
        
        HAPConfigValidationResult result;
        result.valid = true;
        
        for (auto param : parameters) {

            if (param.first == "name") {
                result = validateName(param.second.c_str());

                if (result.valid){
                    _newDevice->name = param.second.c_str();
                } else {
                    break;
                }
                
            } else if (param.first == "deviceAddress") {
                result = validateDeviceAddress(param.second.c_str());

                if (result.valid){
                    _newDevice->deviceAddress = strtol(param.second.c_str(), (char**) NULL, 2);
                } else {
                    break;
                }              
                
            } else if (param.first == "houseAddress") {
                result = validateHouseAddress(param.second.c_str());
                
                if (result.valid){
                    _newDevice->houseAddress = strtol(param.second.c_str(), (char**) NULL, 2);
                } else {
                    break;
                }
            }
        }

        if (result.valid) {

            std::vector<HAPPluginRCSwitchDevice*>::iterator it = find(_devices.begin(), _devices.end(), _newDevice);
            if(it == _devices.end()) {
                LogD("Adding new rcswitch device with name: " + _newDevice->name + " hA: " + String(_newDevice->houseAddress) + " dA: " + String(_newDevice->deviceAddress), true);

                _newDevice->setFakeGatoFactory(_fakeGatoFactory);
                _newDevice->setEventManager(_eventManager);
                
                auto callbackSend = std::bind(&HAPPluginRCSwitch::sendDeviceCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);        
                _newDevice->setRCSwitchSendCallback(callbackSend);

                _accessorySet->addAccessory(_newDevice->initAccessory());
                
                _devices.push_back(_newDevice);

                struct HAPEvent event = HAPEvent(nullptr, 0, 0, "");							
                _eventManager->queueEvent( EventManager::kEventUpdatedConfig, event, EventManager::kLowPriority);        
            }

        } else {
            LogE("Could not add new rcswitch device. Reason: " + result.reason, true);
        }

        _newDevice = nullptr;
    } else {
        auto callbackFormField = std::bind(&HAPPluginRCSwitch::handleHTTPFormField, this, std::placeholders::_1, std::placeholders::_2);
        HAPWebServerBodyParserMultipart parser;
        parser.processAndParse(req, callbackFormField);
        
        if (_newDevice != nullptr) {                    

            std::vector<HAPPluginRCSwitchDevice*>::iterator it = find(_devices.begin(), _devices.end(), _newDevice);
            if(it == _devices.end()) {

                LogD("Adding new rcswitch device with name: " + _newDevice->name + " hA: " + String(_newDevice->houseAddress) + " dA: " + String(_newDevice->deviceAddress), true);

                _newDevice->setFakeGatoFactory(_fakeGatoFactory);
                _newDevice->setEventManager(_eventManager);
                
                auto callbackSend = std::bind(&HAPPluginRCSwitch::sendDeviceCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);        
                _newDevice->setRCSwitchSendCallback(callbackSend);

                _accessorySet->addAccessory(_newDevice->initAccessory());
                
                _devices.push_back(_newDevice);

            }
    
            _newDevice = nullptr;

            struct HAPEvent event = HAPEvent(nullptr, 0, 0, "");							
            _eventManager->queueEvent( EventManager::kEventUpdatedConfig, event, EventManager::kLowPriority);                    
        }
    }

    auto callbackGet = std::bind(&HAPPluginRCSwitch::handleHTTPGetKeyProcessor, this, std::placeholders::_1, std::placeholders::_2);
#if HAP_WEBSERVER_USE_SPIFFS        
    // template processing    
    HAPWebServerTemplateProcessor::processAndSend(res, "/index.html", callbackGet);
#else
    
    HAPWebServerTemplateProcessor::processAndSendEmbedded(res, html_template_index_start, html_template_index_end, callbackGet);
#endif 

    
}

void HAPPluginRCSwitch::handleHTTPFormField(const std::string& fieldName, const std::string& fieldValue) { 

    if (_newDevice == nullptr) {
        _newDevice = new HAPPluginRCSwitchDevice();
    }

    HAPConfigValidationResult result;
    result.valid = true;
    
    if (fieldName == "name") {
        result = validateName(fieldValue.c_str());
        if (result.valid) {
            _newDevice->name = fieldValue.c_str();
        } else {
            _newDevice = nullptr;
            return;
        }            
    } else if (fieldName == "houseAddress") {  

        result = validateName(fieldValue.c_str());
        if (result.valid) {
            _newDevice->houseAddress = strtol(fieldValue.c_str(), (char**) NULL, 2);
        } else {
            _newDevice = nullptr;
            return;
        }
    } else if (fieldName == "deviceAddress") {
        result = validateName(fieldValue.c_str());
        if (result.valid) {
            _newDevice->deviceAddress = strtol(fieldValue.c_str(), (char**) NULL, 2);        
        } else {
            _newDevice = nullptr;
            return;
        }
    }
}


std::vector<HAPWebServerPluginNode*> HAPPluginRCSwitch::getResourceNodes(){
    std::vector<HAPWebServerPluginNode*> vector;

    auto callbackGet = std::bind(&HAPPluginRCSwitch::handleHTTPGet, this, std::placeholders::_1, std::placeholders::_2);
    HAPWebServerPluginNode* getRequest = new HAPWebServerPluginNode("RCSwitch", "rcswitch", "GET", callbackGet);    
    vector.push_back(getRequest);


    auto callbackPost = std::bind(&HAPPluginRCSwitch::handleHTTPPost, this, std::placeholders::_1, std::placeholders::_2);
    HAPWebServerPluginNode* postRequest = new HAPWebServerPluginNode("RCSwitch", "rcswitch", "POST", callbackPost);    
    vector.push_back(postRequest);

    return vector;
}