//
// HAPPlugins.hpp
// Homekit
//
//  Created on: 18.08.2017
//      Author: michael
//

#ifndef HAPPLUGINS_HPP_
#define HAPPLUGINS_HPP_

#include <Arduino.h>
#include <list>
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <functional>

#include <ArduinoJson.h>
#include <MD5Builder.h>


#include "HAPAccessory.hpp"
#include "HAPAccessorySet.hpp"
#include "HAPLogger.hpp"
#include "HAPVersion.hpp"
#include "HAPHelper.hpp"

#include "EventManager.h"

#include "HAPConfig.hpp"
#include "HAPCharacteristic.hpp"
#include "HAPCharacteristics.hpp"
#include "HAPServices.hpp"

#include "HAPFakeGato.hpp"
#include "HAPFakeGatoFactory.hpp"
#include "HAPDeviceID.hpp"

#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>


using namespace httpsserver;

enum HAP_PLUGIN_TYPE {
	HAP_PLUGIN_TYPE_OTHER = 0,
	HAP_PLUGIN_TYPE_ACCESSORY,
	HAP_PLUGIN_TYPE_STORAGE,
	HAP_PLUGIN_TYPE_DISPLAY
};

struct HAPWebServerPluginNode {			
	std::string	name;
	std::string	path;
	std::string method;
	std::function<void(HTTPRequest*, HTTPResponse*)> callback;

	HAPWebServerPluginNode(const std::string &name_, const std::string &path_, const std::string &method_, std::function<void(HTTPRequest*, HTTPResponse*)> callback_) : name(name_), path(path_), method(method_), callback(callback_) {};
};

/* Base class for plugins */
class HAPPlugin {

public:
	
	//virtual HAPPlugin() = 0;
	virtual HAPAccessory* initAccessory() = 0;
	virtual bool begin() = 0;

	virtual JsonObject getConfigImpl() = 0;
	virtual void setConfigImpl(JsonObject root) = 0;

	virtual void handleImpl(bool forced = false) = 0;

	virtual std::vector<HAPWebServerPluginNode*> getResourceNodes() {
		return {};
	}

	virtual void stop()  { _isEnabled = false; };
	virtual void start() { _isEnabled = true;  };
	

	virtual void identify(bool oldValue, bool newValue) {
		LogE("Handle identify from plugins", true);
	}
	
	inline void handle(bool forced = false) {
		if (shouldHandle() || forced) {	
			handleImpl(forced);
		}
	}
	virtual void handleEvents(int eventCode, struct HAPEvent eventParam) {}	

	// virtual HAPConfigValidationResult validateConfigImpl(JsonObject object) = 0;

	virtual HAPConfigValidationResult validateConfig(JsonObject object){	

		// LogD(String(__PRETTY_FUNCTION__), true);

		HAPConfigValidationResult result;
		result.valid = false;

		// ToDo: Should validation be only for enabled plugins?		
		// plugin._name.enabled
		if (!object.containsKey("enabled")){
			result.reason = "plugins." + _name + ".enabled is required";
			LogW("Config validation failed: " + result.reason, true);
			return result;
		}
		if (object.containsKey("enabled") && !object["enabled"].is<bool>()) {
			result.reason = "plugins." + _name + ".enabled is not a bool";
			LogW("Config validation failed: " + result.reason, true);
			return result;
		}
		
		// plugin._name.interval
		if (!object.containsKey("interval")){
			result.reason = "plugins." + _name + ".interval is required";
			LogW("Config validation failed: " + result.reason, true);
			return result;
		}

		if (object.containsKey("interval") && !object["interval"].is<uint32_t>()) {
			result.reason = "plugins." + _name + ".interval is not an integer";
			LogW("Config validation failed: " + result.reason, true);
			return result;
		}
		
		
		result.valid = true;
		return result;		
	}



	virtual void setConfig(JsonObject root){

		LogD(String(__PRETTY_FUNCTION__), true);

		// LogD("Setting configuration for plugin " + String(name()) + " ...", false);
		if (root.containsKey("enabled")){
			LogD(" -- enabled: " + root["enabled"].as<String>(), true);
        	enable(root["enabled"].as<bool>());
    	}

		if (root.containsKey("interval")){
			LogD(" -- interval: " + root["interval"].as<String>(), true);
        	setInterval(root["interval"].as<unsigned long>());
    	}

		setConfigImpl(root);
		// LogD("OK", true);
	}

	virtual JsonObject getConfig(){	
		LogD(String(__PRETTY_FUNCTION__), true);

		const size_t capacity = HAP_ARDUINOJSON_BUFFER_SIZE / 2;
    	DynamicJsonDocument doc(capacity);
    	
    	doc["enabled"] = isEnabled();
    	doc["interval"] = interval();	

		DynamicJsonDocument pluginDoc(capacity/ 2);
		pluginDoc = getConfigImpl();		

		HAPHelper::mergeJson(doc, pluginDoc.as<JsonObject>());
		
		return doc.as<JsonObject>();
	}

	inline String md5(String str) {
		_md5.begin();
		_md5.add(String(str));
		_md5.calculate();
		return _md5.toString();
	}

	inline enum HAP_PLUGIN_TYPE type(){
		return _type;
	}

	inline String version(){
		return _version.toString();
	}

	inline String name(){
		return _name;
	}

	inline bool isEnabled(){
		return _isEnabled;
	}

	inline void enable(bool mode){
		_isEnabled = mode;
	}

	inline unsigned long interval(){
		return _interval;
	}	

	inline void setInterval(unsigned long interval){
		_interval = interval;
	}

	inline bool shouldHandle(){

		if (_isEnabled) {
			unsigned long currentMillis = millis(); // grab current time

			if ((unsigned long)(currentMillis - _previousMillis) >= _interval) {

				// save the last time you blinked the LED
				_previousMillis = currentMillis;

				//LogD("Handle plugin: " + String(_name), true);			
				return true;			
			}
		}

		return false;
	}

	virtual inline void addEventListener(EventManager* eventManager){
		_listenerMemberFunctionPlugin.mObj = this;
		_listenerMemberFunctionPlugin.mf = &HAPPlugin::handleEvents;
	
		// Add listener to event manager
		_eventManager = eventManager;

		// for accessory notifications and values			
		_eventManager->addListener( EventManager::kEventPairingComplete, &_listenerMemberFunctionPlugin );
		_eventManager->addListener( EventManager::kEventNotifyController, &_listenerMemberFunctionPlugin );	
		
	}

	inline void setAccessorySet(HAPAccessorySet* accessorySet){
		_accessorySet = accessorySet;
	}	



	inline void setFakeGatoFactory(HAPFakeGatoFactory* fakeGatoFactory){
		_fakeGatoFactory = fakeGatoFactory;
	}

	inline void registerFakeGato(HAPFakeGato* fakegato, String name, std::function<bool()> callback, uint32_t interval = HAP_FAKEGATO_INTERVAL){
		_fakeGatoFactory->registerFakeGato(fakegato, name, callback, interval);
	}

protected:
	enum HAP_PLUGIN_TYPE 		_type;
	unsigned long 				_previousMillis;
	HAPVersion 					_version;

	String 						_name;
	bool 						_isEnabled;
	unsigned long 				_interval;
	
	HAPAccessory*				_accessory;

	EventManager*				_eventManager;
	HAPAccessorySet*			_accessorySet;
	
	MemberFunctionCallable<HAPPlugin> _listenerMemberFunctionPlugin;
	
	HAPFakeGatoFactory*			_fakeGatoFactory;

	MD5Builder 					_md5;
};

	/* 
	 * Base class for PluginRegistrar
	 * See PluginRegistrar below for explanations
	 */
class IPluginRegistrar {
public:
	virtual std::unique_ptr<HAPPlugin> getPlugin() = 0;
};

	/* 
	 * This is the factory, the common interface to "plugins".
	 * Plugins registers themselves here and the factory can serve them on
	 * demand.
	 * It is a Singleton
	 */
class HAPPluginFactory {
public:
		/* Get Singleton instance */
	static HAPPluginFactory& Instance();
		/* Register a new plugin */
	void registerPlugin(IPluginRegistrar* registrar, String name);
		/* Get an instance of a plugin based on its name */
		/* throws out_of_range if plugin not found */
	std::unique_ptr<HAPPlugin> getPlugin(String name);
	std::vector<String> names();

	void loadPlugins();

private:
		/* Holds pointers to plugin registrars */
	std::map<String, IPluginRegistrar*> _registry;
		/* Make constructors private and forbid cloning */
	HAPPluginFactory(): _registry() {};
	HAPPluginFactory(HAPPluginFactory const&) = delete;
	void operator=(HAPPluginFactory const&) = delete;
};

	/* 
	 * Helper class that registers a plugin upon construction.
	 * Actually, the registrar registers itself, and the proxied plugin is only
	 * created on-demand. This mechanism can be shortened by directly 
	 * registering and instance of the plugin, but the assumption here is that
	 * instanciating the plugin can be heavy and not necessary.
	 */
	template<class TPlugin>
class PluginRegistrar: public IPluginRegistrar {
public:
	PluginRegistrar(String classname);
	std::unique_ptr<HAPPlugin> getPlugin();
private:
		/* That is not really used there, but could be useful */
	String _classname;
};

/* template functions in header */
template<class TPlugin>
PluginRegistrar<TPlugin>::PluginRegistrar(String classname): _classname(classname) {
	HAPPluginFactory &factory = HAPPluginFactory::Instance();
	factory.registerPlugin(this, classname);
}

template<class TPlugin>
std::unique_ptr<HAPPlugin>
PluginRegistrar<TPlugin>::getPlugin() {
	std::unique_ptr<HAPPlugin> plugin(new TPlugin());
	return plugin;
}


#define REGISTER_PLUGIN(CLASSNAME) \
static PluginRegistrar<CLASSNAME> \
CLASSNAME##_registrar( #CLASSNAME ); 


#endif /* HAPPLUGINS_HPP_ */    