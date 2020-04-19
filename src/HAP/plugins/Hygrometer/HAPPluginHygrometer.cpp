//
// HAPPluginHygrometer.cpp
// Homekit
//
//  Created on: 29.04.2018
//      Author: michael
//
#include "HAPPluginHygrometer.hpp"
#include "HAPServer.hpp"

#define HAP_PLUGIN_INTERVAL 5000

#define VERSION_MAJOR       0
#define VERSION_MINOR       0
#define VERSION_REVISION    3
#define VERSION_BUILD       1




HAPPluginHygrometer::HAPPluginHygrometer(){
	_type = HAP_PLUGIN_TYPE_ACCESSORY;
	_name = "Hygrometer";
	_isEnabled = HAP_PLUGIN_USE_HYGROMETER;
	_interval = HAP_PLUGIN_INTERVAL;
	_previousMillis = 0;

    _version.major      = VERSION_MAJOR;
    _version.minor      = VERSION_MINOR;
    _version.revision   = VERSION_REVISION;
    _version.build      = VERSION_BUILD;


}


bool HAPPluginHygrometer::begin(){
	LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Begin()", true);


#if !HAP_PLUGIN_HYGROMETER_USE_DUMMY	
    // Init the soil moisture sensor board
    //  VCC
    pinMode(HAP_PLUGIN_HYGROMETER_PIN_VCC, OUTPUT);
    digitalWrite(HAP_PLUGIN_HYGROMETER_PIN_VCC, LOW);
    
    // ADC
    pinMode(HAP_PLUGIN_HYGROMETER_PIN_ADC, INPUT);
    // analogReadResolution(12);    // 12 is already the default value 
#endif
	return true;
}


void HAPPluginHygrometer::identify(bool oldValue, bool newValue) {
	Serial.printf("Start Identify %s\n", _name.c_str());
}


void HAPPluginHygrometer::changeHum(float oldValue, float newValue) {
	Serial.printf("[%s] New soil moisture: %f\n", _name.c_str(), newValue);
}



void HAPPluginHygrometer::handleImpl(bool forced){	
	
    LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Handle plguin [" + String(_interval) + "]", true);

#if HAP_PLUGIN_HYGROMETER_USE_DUMMY	
	float percentage = random(0, 100);
#else

    uint16_t moisture = readSensor();
    float percentage = map(moisture, HAP_PLUGIN_HYGROMTER_REFERENCE, 0, 100, 0) * 1.0;

#if 0
    Serial.print("[" + _name + "] Moisture: ");
    Serial.print(moisture);
    Serial.print(" = ");
    Serial.print(percentage);
    Serial.println(" %");
#endif

#endif




    setValue(_humidityValue->iid, _humidityValue->value(), String(percentage));
}

// void HAPPluginDHT::handleEvents(int eventCode, struct HAPEvent eventParam){
// 	LogE("!!!!!!!!!!! HANDLE PLUGIN EVENT !!!!!!!!!!!!!!!", true);
// }


void HAPPluginHygrometer::setValue(int iid, String oldValue, String newValue){
	if (iid == _humidityValue->iid) {
		// LogW("Setting soil moisture oldValue: " + oldValue + " -> newValue: " + newValue, true);
		_humidityValue->setValue(newValue);
	}

	struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, iid, newValue);							
	_eventManager->queueEvent( EventManager::kEventNotifyController, event);	
}



HAPAccessory* HAPPluginHygrometer::initAccessory(){
	LogD("\nInitializing plugin: " + _name + " ...", false);

	_accessory = new HAPAccessory();
	
	auto callbackIdentify = std::bind(&HAPPlugin::identify, this, std::placeholders::_1, std::placeholders::_2);
	String sn = md5(HAPDeviceID::deviceID() + _name);
	_accessory->addInfoService("Hygrometer", "ACME", "YL-69", sn, callbackIdentify, version());

	//
	// Soil Moisture
	//
	HAPService* humidityService = new HAPService(HAP_SERVICE_HUMIDITY_SENSOR);
	_accessory->addService(humidityService);

	stringCharacteristics *humServiceName = new stringCharacteristics(HAP_CHARACTERISTIC_NAME, permission_read, 0);
	humServiceName->setValue("Soil Moisture Sensor");
	_accessory->addCharacteristics(humidityService, humServiceName);

	_humidityValue = new floatCharacteristics(HAP_CHARACTERISTIC_CURRENT_RELATIVE_HUMIDITY, permission_read|permission_notify, 0, 100, 0.1, unit_percentage);
	_humidityValue->setValue("0.0");

	auto callbackChangeHum = std::bind(&HAPPluginHygrometer::changeHum, this, std::placeholders::_1, std::placeholders::_2);
	//_humidityValue->valueChangeFunctionCall = std::bind(&changeHum);
	_humidityValue->valueChangeFunctionCall = callbackChangeHum;
	_accessory->addCharacteristics(humidityService, _humidityValue);

	//
	// FakeGato
	// 	
	// _fakegato.begin();	
	_fakegato.registerFakeGatoService(_accessory, _name);
	auto callbackAddEntry = std::bind(&HAPPluginHygrometer::fakeGatoCallback, this);
	// _fakegato.registerCallback(callbackAddEntry);
	registerFakeGato(&_fakegato, _name, callbackAddEntry);

	LogD("OK", true);

	return _accessory;
}

HAPConfigValidationResult HAPPluginHygrometer::validateConfig(JsonObject object){
    return HAPPlugin::validateConfig(object);
}

JsonObject HAPPluginHygrometer::getConfigImpl(){
    DynamicJsonDocument doc(1);
	return doc.as<JsonObject>();
}

void HAPPluginHygrometer::setConfigImpl(JsonObject root){

}


bool HAPPluginHygrometer::fakeGatoCallback(){		
	return _fakegato.addEntry("0", _humidityValue->value(), "0");
}


uint16_t HAPPluginHygrometer::readSensor() {

    digitalWrite(HAP_PLUGIN_HYGROMETER_PIN_VCC, HIGH);
    delay(100);

    uint16_t value = analogRead(HAP_PLUGIN_HYGROMETER_PIN_ADC);
    digitalWrite(HAP_PLUGIN_HYGROMETER_PIN_VCC, LOW);

    return 4095 - value;
}