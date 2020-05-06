//
// HAPConfig.cpp
// Homekit
//
//  Created on: 01.08.2019
//      Author: michael
//
#include <Arduino.h>



#include "HAPConfig.hpp"
#include "HAPLogger.hpp"
#include "HAPPlugins.hpp"
#include "HAPHelper.hpp"

#if HAP_CONFIG_USE_SPIFFS
#include <FS.h>
#include <SPIFFS.h>
#endif

#include "../WiFiCredentials.hpp"
#include "HAPWiFiHelper.hpp"

DynamicJsonDocument HAPConfig::_config = DynamicJsonDocument(HAP_ARDUINOJSON_BUFFER_SIZE);

HAPConfig::HAPConfig(){
    
}

void HAPConfig::begin(){
    
    // homekit
    JsonObject homekit = _config.createNestedObject("homekit");
    homekit["loglevel"]         = (uint8_t)HAP_LOGLEVEL;

    // accessory
    JsonObject accessory = _config.createNestedObject("accessory");
    accessory["pincode"]        = HAP_PIN_CODE;
    accessory["hostname"]       = HAP_HOSTNAME_PREFIX;

    // wifi
    JsonObject wifi = _config.createNestedObject("wifi");
    wifi["mode"]                = HAP_WIFI_DEFAULT_MODE;

    if (HAP_WIFI_DEFAULT_MODE == HAPWiFiModeMulti) {
        JsonArray wifi_networks     = wifi.createNestedArray("networks");
        JsonObject wifi_networks_0  = wifi_networks.createNestedObject();
        
        wifi_networks_0["ssid"]     = WIFI_SSID;
        wifi_networks_0["password"] = WIFI_PASSWORD;
    }

    // webserver
    JsonObject webserver = _config.createNestedObject("webserver");
    webserver["enabled"]        = intToBool(HAP_ENABLE_WEBSERVER);
    
    JsonArray webserver_admins    = webserver.createNestedArray("admins");
    JsonObject webserver_admins_0 = webserver_admins.createNestedObject();
    webserver_admins_0["username"] = HAP_WEBSERVER_ADMIN_USERNAME;
    webserver_admins_0["password"] = HAP_WEBSERVER_ADMIN_PASSWORD;

    JsonArray webserver_apis      = webserver.createNestedArray("apis");
    JsonObject webserver_apis_0   = webserver_apis.createNestedObject();
    webserver_apis_0["username"]  = HAP_WEBSERVER_API_USERNAME;
    webserver_apis_0["password"]  = HAP_WEBSERVER_API_PASSWORD;

    // update
    JsonObject update           = _config.createNestedObject("update");
    JsonObject update_web       = update.createNestedObject("web");
    update_web["enabled"]       = intToBool(HAP_UPDATE_ENABLE_FROM_WEB);
    JsonObject update_ota       = update.createNestedObject("ota");
    update_ota["enabled"]       = intToBool(HAP_UPDATE_ENABLE_OTA);
    update_ota["port"]          = HAP_UPDATE_OTA_PORT;
    update_ota["password"]      = HAP_UPDATE_OTA_PASSWORD;

    // plugins
    JsonObject plugins = _config.createNestedObject("plugins");

    auto &factory = HAPPluginFactory::Instance();        
    std::vector<String> names = factory.names();    

    for(std::vector<String>::iterator it = names.begin(); it != names.end(); ++it) {
    	//Serial.println(*it);
    	auto plugin = factory.getPlugin(*it);
                
        plugins[plugin->name()] = plugin->getConfig();               
    }    
}

JsonObject HAPConfig::config(){
    return _config.as<JsonObject>();
}

bool HAPConfig::intToBool(int i){
    if (i) return true;

    return false;
}


void HAPConfig::mergeConfig(JsonObject object){    
    HAPHelper::mergeJson(_config, object);
}

void HAPConfig::update(){    
    // updatePluginConfig();


    _callbackUpdate();
}

// void HAPConfig::updatePluginConfig(){
//     auto &factory = HAPPluginFactory::Instance();        
//     std::vector<String> names = factory.names();    

//     for(std::vector<String>::iterator it = names.begin(); it != names.end(); ++it) {
//     	//Serial.println(*it);
//     	auto plugin = factory.getPlugin(*it);
//         plugin->setConfig(_config["plugins"][plugin->name()]);            
//     }
// }

HAPConfigValidationResult HAPConfig::parse(String jsonString, bool dryRun){
    return parse(jsonString.c_str(), dryRun);
}


HAPConfigValidationResult HAPConfig::parse(const char* jsonString, bool dryRun){
    return parse((uint8_t*) jsonString, strlen(jsonString), dryRun);
}

HAPConfigValidationResult HAPConfig::parse(const uint8_t* jsonString, size_t length, bool dryRun){

    DynamicJsonDocument doc(HAP_ARDUINOJSON_BUFFER_SIZE);
    HAPConfigValidationResult result;
    result.valid = false;

    DeserializationError error = deserializeJson(doc, jsonString, length);
    if (error){
        result.reason = "Invalid json";
        LogW("Config validation failed: " + result.reason, true);        
        return result;
    }

    result = validateConfig(doc.as<JsonObject>());
    if (result.valid == false){
        LogW("Config validation failed: " + result.reason, true);
        return result;
    }

    if (!dryRun){        
        _config =  doc;
        _callbackUpdate();        
    }

    return result;
}
    
HAPConfigValidationResult HAPConfig::validateConfig(const JsonObject object){
    
    HAPConfigValidationResult result;
    result.valid = false;

    // homekit
    result = validateConfigHomekit(object);
    if (!result.valid) return result;
    
    // accessory
    result = validateConfigAccessory(object);
    if (!result.valid) return result;

    // wifi
    result = validateConfigWifi(object);
    if (!result.valid) return result;

    // webserver
    result = validateConfigWebserver(object);
    if (!result.valid) return result;

    // update
    result = validateConfigUpdate(object);
    if (!result.valid) return result;

    // plugins
    result = validateConfigPlugins(object);
    if (!result.valid) return result;


    result.valid = true;
    return result;
}

HAPConfigValidationResult HAPConfig::validateConfigHomekit(const JsonObject object){
    HAPConfigValidationResult result;
    result.valid = false;

    // homekit.loglevel - int
    if (!object["homekit"].as<JsonObject>().containsKey("loglevel") || !object["homekit"]["loglevel"].is<uint8_t>()) {
        result.reason = "homekit.loglevel is not an integer";        
        return result;
    }

    result.valid = true;
    return result;
}


HAPConfigValidationResult HAPConfig::validateConfigAccessory(const JsonObject object){
    HAPConfigValidationResult result;
    result.valid = false;
    
    /*
        "accessory": {
            "pin-code": "031-45-712",
            "hostname": "esp32-"
        }
     */

    // accessory
    if (object.containsKey("accessory") && !object["accessory"].is<JsonObject>()) {
        result.reason = "accessory is not an object";
        return result;
    }

    // accessory.hostname - string
    if (!object["accessory"].as<JsonObject>().containsKey("hostname") || !object["accessory"]["hostname"].is<const char*>()) {
        result.reason = "accessory.hostname is not a string";
        return result;
    }
    // accessory.hostname - length
    if (strlen(object["accessory"]["hostname"]) + 1 > HAP_STRING_LENGTH_MAX) {
        result.reason = "accessory.hostname is too long";
        return result;
    }

    // accessory.pincode - string
    if (!object["accessory"].as<JsonObject>().containsKey("pincode") || !object["accessory"]["pincode"].is<const char*>()) {
        result.reason = "accessory.pincode is not a string";
        return result;
    }

    // accessory.pincode - length max 10
    if (strlen(object["accessory"]["pincode"]) != 10) {
        result.reason = "accessory.pincode is not 10 characters long";
        return result;
    }

    result.valid = true;
    return result;
}

HAPConfigValidationResult HAPConfig::validateConfigWifi(const JsonObject object){
    HAPConfigValidationResult result;
    result.valid = false;
    
    /*
        "wifi": {
            "mode": 0,
            "networks" [
                {
                    "ssid": "XXX",
                    "password": "XXX"
                }
            ]
        }
     */

    // wifi
    if (object.containsKey("wifi") && !object["wifi"].is<JsonObject>()) {
        result.reason = "wifi is not an object";
        return result;
    }

    // wifi.mode - uint8_t
    if (!object["wifi"].as<JsonObject>().containsKey("mode") || !object["wifi"]["mode"].is<uint8_t>()) {
        if (object["wifi"]["mode"] >= 0 && object["wifi"]["mode"] <= 3)
        result.reason = "wifi.mode is not a valid value";
        return result;
    }

    // if ( object["wifi"]["mode"].as<uint8_t>() == HAPWiFiModeMulti) {
        // wifi.networks - array
        if (object["wifi"].as<JsonObject>().containsKey("networks") && !object["wifi"]["networks"].is<JsonArray>()) {
            result.reason = "wifi.networks is not an array";
            return result;
        }


        uint8_t count = 0;
        for( const auto& value : object["wifi"]["networks"].as<JsonArray>() ) {
                
            
            // wifi.networks.count.username
            if (!value.containsKey("ssid") || !value["ssid"].is<const char*>()) {
                result.reason = "wifi.networks." + String(count) + ".ssid is not a string";
                return result;
            }   

            // wifi.networks.count.username - length -> MAX 31    
            if (strlen(value["ssid"]) > 31) {
                result.reason = "wifi.networks." + String(count) + ".ssid is too long";
                return result;
            } 


            // wifi.networks.count.password
            if (!value.containsKey("password") || !value["password"].is<const char*>()) {
                result.reason = "wifi.networks." + String(count) + ".password is not a string";
                return result;
            }   

            // wifi.networks.count.password - length -> MAX 63      
            if (strlen(value["password"]) > 63) {
                result.reason = "wifi.networks." + String(count) + ".password is too long";
                return result;
            } 
                
            count++;
        }
    // }

    result.valid = true;
    return result;
}


HAPConfigValidationResult HAPConfig::validateConfigWebserver(const JsonObject object){
    HAPConfigValidationResult result;
    result.valid = false;
    
    /*
        "webserver": {
            "enabled": true,
            "admins": [
                {
                    "username": "admin",
                    "password": "secret"
                }
            ],
            "apis": [
                {
                    "username": "api",
                    "password": "secret"
                }
            ]
        }
     */

    // webserver
    if (object.containsKey("webserver") && !object["webserver"].is<JsonObject>()) {
        result.reason = "webserver is not an object";
        return result;
    }

    // webserver.enabled - bool
    if (!object["webserver"].as<JsonObject>().containsKey("enabled") || !object["webserver"]["enabled"].is<bool>()) {
        result.reason = "webserver.enabled is not a bool";
        return result;
}

    // webserver.admins
    if (!object["webserver"].as<JsonObject>().containsKey("admins") || !object["webserver"]["admins"].is<JsonArray>()) {
        result.reason = "webserver.admins is not an array";
        return result;
    }

    // webserver.admins array
    uint8_t count = 0;
    for( const auto& value : object["webserver"]["admins"].as<JsonArray>() ) {
            
        
        // webserver.admins.count.username
        if (!value.containsKey("username") || !value["username"].is<const char*>()) {
            result.reason = "webserver.admins." + String(count) + ".username is not a string";
            return result;
        }   

        // webserver.admins.count.username - length        
        if (strlen(value["username"]) + 1 > HAP_STRING_LENGTH_MAX) {
            result.reason = "webserver.admins." + String(count) + ".username is too long";
            return result;
        } 


        // webserver.admins.count.password
        if (!value.containsKey("password") || !value["password"].is<const char*>()) {
            result.reason = "webserver.admins." + String(count) + ".password is not a string";
            return result;
        }   

        // webserver.admins.count.password - length        
        if (strlen(value["password"]) + 1 > HAP_STRING_LENGTH_MAX) {
            result.reason = "webserver.admins." + String(count) + ".password is too long";
            return result;
        } 
              
        count++;
    }

    // webserver.apis
    if (!object["webserver"].as<JsonObject>().containsKey("apis") || !object["webserver"]["apis"].is<JsonArray>()) {
        result.reason = "webserver.apis is not an array";
        return result;
    }

    // webserver.apis array
    count = 0;
    for( const auto& value : object["webserver"]["apis"].as<JsonArray>() ) {
            
        
        // webserver.apis.count.username
        if (!value.containsKey("username") || !value["username"].is<const char*>()) {
            result.reason = "webserver.apis." + String(count) + ".username is not a string";
            return result;
        }   

        // webserver.apis.count.username - length        
        if (strlen(value["username"]) + 1 > HAP_STRING_LENGTH_MAX) {
            result.reason = "webserver.apis." + String(count) + ".username is too long";
            return result;
        } 


        // webserver.apis.count.password
        if (!value.containsKey("password") || !value["password"].is<const char*>()) {
            result.reason = "webserver.apis." + String(count) + ".password is not a string";
            return result;
        }   

        // webserver.apis.count.password - length        
        if (strlen(value["password"]) + 1 > HAP_STRING_LENGTH_MAX) {
            result.reason = "webserver.apis." + String(count) + ".password is too long";
            return result;
        }
              
        count++;
    }

    result.valid = true;
    return result;
}


HAPConfigValidationResult HAPConfig::validateConfigUpdate(const JsonObject object){
    HAPConfigValidationResult result;
    result.valid = false;
    
    /*
        "update": {
            "web": {
                "enabled": true    	
            },
            "ota": {
                "enabled": true,
                "port": 3232,
                "password": "admin"
            }
        }
     */

    // update
    if (object.containsKey("update") && !object["update"].is<JsonObject>()) {
        result.reason = "update is not an object";
        return result;
    }

    // update.web - JsonObject
    if (!object["update"].as<JsonObject>().containsKey("web") || !object["update"]["web"].is<JsonObject>()) {
        result.reason = "update.web is not a object";
        return result;
    }

    // update.web.enabled - bool
    if (!object["update"]["web"].as<JsonObject>().containsKey("enabled") || !object["update"]["web"]["enabled"].is<bool>()) {
        result.reason = "update.web.enabled is not a bool";
        return result;
    }


    // update.ota - JsonObject
    if (!object["update"].as<JsonObject>().containsKey("ota") || !object["update"]["ota"].is<JsonObject>()) {
        result.reason = "update.ota is not a object";
        return result;
    }

    // update.ota.enabled - bool
    if (!object["update"]["ota"].as<JsonObject>().containsKey("enabled") || !object["update"]["ota"]["enabled"].is<bool>()) {
        result.reason = "update.ota.enabled is not a bool";
        return result;
    }

    // update.ota.password
    if (object["update"]["ota"].as<JsonObject>().containsKey("password") && !object["update"]["ota"]["password"].is<const char*>()) {
        result.reason = "update.ota.password is not a string";
        return result;
    }   

    // update.ota.password - length        
    if ((object["update"]["ota"].as<JsonObject>().containsKey("password")) && (strlen(object["update"]["ota"]["password"].as<const char*>()) + 1 > HAP_STRING_LENGTH_MAX)) {
        result.reason = "update.ota.password is too long";
        return result;
    }

    // update.ota.port
    if (object["update"]["ota"].as<JsonObject>().containsKey("port") && !object["update"]["ota"]["port"].is<uint16_t>()) {
        result.reason = "update.ota.port is not a integer";
        return result;
    } 

    result.valid = true;
    return result;
}


HAPConfigValidationResult HAPConfig::validateConfigPlugins(const JsonObject object){
    HAPConfigValidationResult result;
    result.valid = false;

    auto &factory = HAPPluginFactory::Instance();        
    std::vector<String> names = factory.names();    

    if (!object.containsKey("plugins")) {
        result.reason = "plugins is required";
        return result;
    }

    if (object.containsKey("plugins") && !object["plugins"].is<JsonObject>()) {
        result.reason = "plugins is not an object";
        return result;
    }

    for (std::vector<String>::iterator it = names.begin(); it != names.end(); ++it) {
    	//Serial.println(*it);
    	auto plugin = factory.getPlugin(*it);

                
        if (object["plugins"].containsKey(plugin->name()) && !object["plugins"][plugin->name()].is<JsonObject>()) {
            result.reason = "plugins." + plugin->name() + " is not an object";
            return result;
        }

        result = plugin->validateConfig(object["plugins"][plugin->name()]);
        if (!result.valid) return result;
        

    }

    result.valid = true;
    return result;
}

void HAPConfig::prettyPrintTo(Print& output){
    serializeJsonPretty(_config, output);    
}

void HAPConfig::prettyPrintTo(String& output){
    serializeJsonPretty(_config, output);
}

bool HAPConfig::save(){
    // Open Preferences with my-app namespace. Each application module, library, etc
    // has to use a namespace name to prevent key name collisions. We will open storage in
    // RW-mode (second parameter has to be false).
    // Note: Namespace name is limited to 15 chars.
    _prefs.begin("config", false);

    String cfg;        
#if HAP_DEBUG  
    LogD("Saving config to", true); 
    serializeJsonPretty(_config, cfg);     
    LogD(cfg, true); 
    cfg = "";       
#endif

    serializeJson(_config, cfg); 

    size_t writtenBytes = _prefs.putString("json", cfg);

    _prefs.end();

    LogD("Bytes written: " + String(writtenBytes) + "/" + String(measureJson(_config)), true);
    if (writtenBytes != measureJson(_config)){
        return false;
    }

    return true;        
}

bool HAPConfig::load(){
    _prefs.begin("config", true);
    
    String cfg = _prefs.getString("json", "");
    
    HAPConfigValidationResult result = parse(cfg);

    _prefs.end();

    if (result.valid == false){
        LogE("Could not load: " + result.reason, true);
        return false;
    }

#if HAP_DEBUG  
    LogD("Loaded config", true);      
    prettyPrintTo(Serial);
#endif

    _callbackUpdate();
    
    return true;
}



void HAPConfig::setRefTime(uint32_t refTime, bool force) {
    
    if (!_config.containsKey("fakegato")) {
        _config.createNestedObject("fakegato");
    }
    
    if (!_config["fakegato"].containsKey("reftime") || force){
        LogD("Setting reference time to :" + String(refTime), true );
        _config["fakegato"]["reftime"] = refTime;   
        save();
    } else {
        LogD("Reference time is :" + _config["fakegato"]["reftime"].as<String>(), true );
    }
}

uint32_t HAPConfig::getRefTime(){
    if (_config["fakegato"].containsKey("reftime")){
        return _config["fakegato"]["reftime"].as<uint32_t>();
    } 
    return 0;
}

void HAPConfig::addNetwork(String ssid, String password){
    if (!_config["wifi"].containsKey("networks")) {
        JsonArray wifi_networks     = _config["wifi"].createNestedArray("networks");
        JsonObject wifi_networks_0  = wifi_networks.createNestedObject();
        wifi_networks_0["ssid"]     = ssid;
        wifi_networks_0["password"] = password;
    } else {
        
        // change password if found
        for (JsonObject network : _config["wifi"]["networks"].as<JsonArray>()){
            if ( network["ssid"].as<String>() == ssid){
                network["ssid"] = password;                
                return;
            }
        }

        // Add ssid and password
        JsonObject wifi_networks_0  = _config["wifi"]["networks"].createNestedObject();
        wifi_networks_0["ssid"]     = ssid;
        wifi_networks_0["password"] = password;                
    }  
}

size_t HAPConfig::measureLength(){
    return measureJson(_config);
}