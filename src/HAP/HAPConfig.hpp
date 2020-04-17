//
// HAPConfig.hpp
// Homekit
//
//  Created on: 14.12.2018
//      Author: michael
//

#ifndef HAPCONFIG_HPP_
#define HAPCONFIG_HPP_

#include <Arduino.h>
#include <ArduinoJson.h>
#include <functional>

#include "HAPGlobals.hpp"
#include "HTTPResponse.hpp"

#include <Preferences.h>


/* Example settings in json
    {
    "accessory": {
        "pincode": "031-45-712",
        "hostname": ""
    },
    "webserver": {
        "enabled": true
    },
    "update": {
        "web": {
        "enabled": true
        },
        "ota": {
        "enabled": true
        }
    },
    "plugins": {
        "HAPPluginInfluxDB": {
        "enabled": true,
        "interval": 2000,
        "username": "admin",
        "password": "admin",
        "database": "homekit",
        "port": 8086,
        "host": "192.168.178.137"
        }
    }
    }

    Remarks to json:
        interval    -> always uint32_t
        pin         -> always uin8_t    
        
        enabled     -> always bool

        host        -> ip or hostname -> check for ip string
    */

struct HAPConfigValidationResult
{
    bool valid;
    String reason;
};

class HAPConfig
{
public:
    HAPConfig();

    static HAPConfigValidationResult validateConfig(const JsonObject object);

    void begin();
    JsonObject config();

    inline void registerCallback(std::function<void(void)> callback){
        _callbackUpdate = callback;
    }

    HAPConfigValidationResult parse(const char *jsonString, bool dryRun = false);
    HAPConfigValidationResult parse(String jsonString, bool dryRun = false);
    HAPConfigValidationResult parse(const uint8_t* jsonString, size_t length, bool dryRun);


    void addNetwork(String ssid, String password);


    void mergeConfig(JsonObject object);

    void prettyPrintTo(Print &output);
    void prettyPrintTo(String &output);
    // void prettyPrintTo(std::string& output);

    void update();
    bool save();
    bool load();

    size_t measureLength();

    void setRefTime(uint32_t refTime, bool force = false);
    uint32_t getRefTime();

private:

    Preferences _prefs;


    // void updatePluginConfig();

    static DynamicJsonDocument _config;

    static HAPConfigValidationResult validateConfigHomekit(const JsonObject object);
    static HAPConfigValidationResult validateConfigAccessory(const JsonObject object);
    static HAPConfigValidationResult validateConfigWifi(const JsonObject object);
    static HAPConfigValidationResult validateConfigWebserver(const JsonObject object);
    static HAPConfigValidationResult validateConfigUpdate(const JsonObject object);
    static HAPConfigValidationResult validateConfigPlugins(const JsonObject object);

    static bool intToBool(int i);

    std::function<void(void)> _callbackUpdate;
};

#endif /* HAPSETTINGS_HPP_ */