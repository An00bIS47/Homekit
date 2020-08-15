//
// HAPPluginInfluxDB.cpp
// Homekit
//
//  Created on: 29.04.2018
//      Author: michael
//

#include "HAPPluginInfluxDB.hpp"
#include "HAPHelper.hpp"
#include "HAPLogger.hpp"
#include "HAPCharacteristic.hpp"
#include "HAPServer.hpp"

#include "HAPGlobals.hpp"

#define HAP_PLUGIN_INFLUXDB_BUFFER_SIZE 3192

#define VERSION_MAJOR       0
#define VERSION_MINOR       0
#define VERSION_REVISION    1
#define VERSION_BUILD       1

#if HAP_USE_INFLUXDB_SECURE
extern const uint8_t localCA_pem_start[] asm("_binary_localCA_pem_start");
extern const uint8_t localCA_pem_end[]   asm("_binary_localCA_pem_end");
#endif


// ToDo: Use HAPGlobals for this ?
const char*     INFLUXDB_HOST = "homebridge";
const uint16_t  INFLUXDB_PORT = 8086;

const char*     INFLUXDB_DATABASE = "homekit";
const char*     INFLUXDB_USER = "admin";
const char*     INFLUXDB_PASS = "password";


// ToDo: ?
// const uint8_t INFLUXDB_IGNORED_SERVICE_TYPES[] = {
// 	serviceType_accessoryInfo
// };



HAPPluginInfluxDB::HAPPluginInfluxDB() {
    
    _type = HAP_PLUGIN_TYPE_STORAGE;
    _name = "InfluxDB";
    _isEnabled = HAP_PLUGIN_USE_INFLUXDB;
    _interval = HAP_PLUGIN_INFLUXDB_INTERVAL;
    _previousMillis = 0;	
    
#if HAP_USE_INFLUXDB_SECURE
    // _influxdb->setCACert((const char*)localCA_pem_start);	// does not work with provided root CA cert :(
                                                                // dont know why
#endif

    _version.major      = VERSION_MAJOR;
    _version.minor      = VERSION_MINOR;
    _version.revision   = VERSION_REVISION;
    _version.build      = VERSION_BUILD;

    _username = INFLUXDB_USER;
    _password = INFLUXDB_PASS;
    _hostname = INFLUXDB_HOST;
	_database = INFLUXDB_DATABASE;
    _port = INFLUXDB_PORT;
 
    _eventManager		= nullptr;

    _usedSize = 0;
}

HAPAccessory* HAPPluginInfluxDB::initAccessory(){

    return nullptr;
}

bool HAPPluginInfluxDB::begin(){

    _influxdb = new Influxdb(_hostname, _port);	
    _influxdb->setDbAuth(_database, _username, _password);

    return true;
}



void HAPPluginInfluxDB::handleImpl(bool forced){	

    LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Handle plguin [" + String(_interval) + "]", true);
    
    // first accessory is bridge -> don't write
    for (int i=1; i < _accessorySet->numberOfAccessory(); i++){
        HAPAccessory* curAcc = _accessorySet->accessoryAtIndex(i);
        // Serial.print("Acc: " + curAcc->getName() + " - ");			
        // Serial.println(curAcc->numberOfService());

        String accName = curAcc->name();
        accName.replace(" ", "");
        
        String name;
        // first service is info service -> don't write
        for (int j=1; j < curAcc->numberOfService(); j++){
            HAPService* curSer = curAcc->serviceAtIndex(j);                                

            for (int k=0; k < curSer->numberOfCharacteristics(); k++){
                characteristics *curChar = curSer->characteristicsAtIndex(k);

                if (!curChar->hidden()){
                    if (curChar->type == HAP_CHARACTERISTIC_NAME){
                        // serviceName is first -> don't write into db
                        // Serial.println("-- " + curChar->value());

                        name = curChar->value();	
                        name.replace(" ", "");
                        name.replace("\"", "");
                        //Serial.println(name);					
                    } else if (curChar->typeString == HAP_SERVICE_FAKEGATO_HISTORY) {
                        // Exclude fakegato history service (all others are hidden)
                        //
                    } else {
                        // all other chars -> write to db
                        // Serial.println("-- " + curChar->value());

                        if (name == ""){
                            name = characteristicsName(curChar->type);
                        }

                        if (name != "") {
                            InfluxData row = InfluxData(name);
                            row.addTag("hostname", _accessorySet->modelName());
                            row.addTag("accessory", accName);
                            // row.addTag("sensor", name);
                            row.addTag("type", String(curChar->type));

                            row.setTimestamp(HAPServer::timestamp());
                            
                            // Serial.print("Handling " + String(name) + " - value: " + String(curChar->value()) + " type: ");
                            if ( HAPHelper::isValidFloat(curChar->value()) ){
                                // Serial.println(" float");
                                row.addValue("value", curChar->value().toFloat(), 2);					
                            } else if ( HAPHelper::isValidNumber(curChar->value()) ) {
                                // Serial.println(" integer");
                                row.addValue("value", curChar->value().toFloat());
                            } else if (curChar->value() == "true") {
                                // Serial.println(" bool -> 1");
                                row.addValue("value", 1);					
                            } else if (curChar->value() == "false") {
                                // Serial.println(" bool -> 0");
                                row.addValue("value", 0);					
                            } else {
                                row.addValueString("value", curChar->value() );					
                            }

                            // LogD(row.toString() + " - size: " + String(row.serializedSize()), true);
                    
                            if (_usedSize + row.serializedSize() > HAP_PLUGIN_INFLUXDB_BUFFER_SIZE){
                                LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Sending data to InfluxDB server [" + String(_usedSize) + " bytes] ...", false);
                                _influxdb->write();
                                _usedSize = 0;
                                LogD("OK", true);
                            }

                            name = "";

                            _influxdb->prepare(row);
                            _usedSize += row.serializedSize();                            
                            // LogD("usesdSize: " + String(_usedSize), true);
                        }
                        

                    }   
                }                    					

            }
            
        }
    }
}

void HAPPluginInfluxDB::handleEvents(int eventCode, struct HAPEvent eventParam){
    LogD("Handle event: [" + String(__PRETTY_FUNCTION__) + "]", true);	



    //characteristics *character = _accessorySet->getCharacteristics(aid, iid);

}

void HAPPluginInfluxDB::addEventListener(EventManager* eventManager){

    if (_isEnabled) {
        LogD("Add Listener: [" + String(__PRETTY_FUNCTION__) + "]", true);	

        _listenerMemberFunctionPlugin.mObj = this;
        _listenerMemberFunctionPlugin.mf = &HAPPlugin::handleEvents;
        
        // Add listener to event manager
        _eventManager = eventManager;

        
        // for accessory notifications and values
        // _eventManager->addListener( EventManager::kEventNotifyAccessory, &_listenerMemberFunctionPlugin );
        // _eventManager->addListener( EventManager::kEventPairingComplete, &_listenerMemberFunctionPlugin );
        // _eventManager->addListener( EventManager::kEventHomekitStarted, &_listenerMemberFunctionPlugin );
        
        // _eventManager->addListener( EventManager::kEventNotifyController, &_listenerMemberFunctionPlugin );		
    }
    
}

/*
 * Config validation
 */
HAPConfigValidationResult HAPPluginInfluxDB::validateConfig(JsonObject object){

    LogD("Validation config " + String(__PRETTY_FUNCTION__), true);
    

    HAPConfigValidationResult result;
    
    result = HAPPlugin::validateConfig(object);
    if (result.valid == false) {
        return result;
    }
    result.valid = false;

    /*
        "HAPPluginInfluxDB": {
            "enabled": true,
            "interval": 1000,
            "username": "admin",
            "password": "admin",
            "database": "homekit",
            "port": 8086,
            "host": "192.168.178.137"
        }
     */

    
    // plugin._name.username
    if (object.containsKey("username") && !object["username"].is<const char*>()) {
        result.reason = "plugins." + _name + ".username is not a string";
        return result;
    }

    // plugin._name.password
    if (object.containsKey("password") && !object["password"].is<const char*>()) {
        result.reason = "plugins." + _name + ".password is not a string";
        return result;
    }

    // plugin._name.database - required
    if (!object.containsKey("database") ) {
        result.reason = "plugins." + _name + ".database is required";
        return result;
    }

    if (object.containsKey("database") && !object["database"].is<const char*>()) {
        result.reason = "plugins." + _name + ".database is not a string";
        return result;
    }

    // plugin._name.hostname - required
    if (!object.containsKey("hostname") ) {
        result.reason = "plugins." + _name + ".hostname is required";
        return result;
    }

    if (object.containsKey("hostname") && !object["hostname"].is<const char*>()) {
        result.reason = "plugins." + _name + ".hostname is not a string";
        return result;
    }

    // plugin._name.port - required
    if (!object.containsKey("port") ) {
        result.reason = "plugins." + _name + ".port is required";
        return result;
    }
    if (object.containsKey("port") && !object["port"].is<uint16_t>()) {
        result.reason = "plugins. " + _name + ".port is not a unsigned integer";
        return result;
    }

    result.valid = true;
    return result;
}

/*
 * getConfig
 * 
 * @param: root :   the root object of the plugin config
 *                  enabled and interval is already present 
 *                  add all other config values except "_enabled" and "_interval"
 */
JsonObject HAPPluginInfluxDB::getConfigImpl(){   

    LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Get config implementation", true);     
    
    DynamicJsonDocument doc(512);
    doc["username"] = _username;
    doc["password"] = _password;
    doc["database"] = _database;
    doc["port"]     = _port;
    doc["hostname"] = _hostname;

#if HAP_DEBUG_CONFIG
    serializeJson(doc, Serial);
    Serial.println();
#endif

    doc.shrinkToFit();
    return doc.as<JsonObject>();
}

void HAPPluginInfluxDB::setConfigImpl(JsonObject root){
    
    if (root.containsKey("username")){
        // LogD(" -- username: " + String(root["username"]), true);
        _username = root["username"].as<String>();
    }

    if (root.containsKey("password")){
        // LogD(" -- password: " + String(root["password"]), true);
        _password = root["password"].as<String>();
    }

    if (root.containsKey("hostname")){
        // LogD(" -- hostname: " + String(root["hostname"]), true);
        _hostname =root["hostname"].as<String>();
    }

    if (root.containsKey("database")){
        // LogD(" -- database: " + String(root["database"]), true);
        _database = root["database"].as<String>();
    }

    if (root.containsKey("port")){
        // LogD(" -- port: " + String(root["port"]), true);
        _port = root["port"].as<uint16_t>();
    }
}


