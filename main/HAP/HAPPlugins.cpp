//
// HAPPlugins.cpp
// Homekit
//
//  Created on: 18.08.2017
//      Author: michael
//

#include "HAPPlugins.hpp"
#include "HAPWebServer.hpp"
#include "HAPCallbackTemplate.hpp"

//namespace HAPPluginSystem {

// HAPAccessory* HAPPlugin::_accessory = nullptr;
// String HAPPlugin::_name;
// bool HAPPlugin::_isEnabled;
// unsigned long HAPPlugin::_interval;


HAPPluginFactory& HAPPluginFactory::Instance() {
	static HAPPluginFactory instance;
	return instance;
}

void HAPPluginFactory::registerPlugin(IPluginRegistrar* registrar, String name) {
	_registry[name] = registrar;
}

std::unique_ptr<HAPPlugin> HAPPluginFactory::getPlugin(String name) {
	/* throws out_of_range if plugin unknown */
	IPluginRegistrar* registrar = _registry.at(name);
	return registrar->getPlugin();
}

std::vector<String> HAPPluginFactory::names(){
	std::vector<String> v;
	for(std::map<String, IPluginRegistrar*>::iterator it = _registry.begin(); it != _registry.end(); ++it) {
		v.push_back(it->first);	  
	}
	return v;
}


//}

