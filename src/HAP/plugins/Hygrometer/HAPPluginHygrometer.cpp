//
// HAPPluginHygrometer.cpp
// Homekit
//
//  Created on: 29.04.2018
//      Author: michael
//
// Pinout:
// The YL-39 module has 4 pins: 
// - VCC: 3.3-5V
// - GND
// - D0 : digital pin that goes LOW or HIGH depending on a preset value
// - A0 : analog output that can be easily read by Arduino
// 
// On ESP32s when using WiFi only ADC1 pins can be used! ADC2 is disabled when using WIFI
// 
// The issue with such sensors is that the probe itself work by trying to measure the current 
// that goes from one side of it to the other. Because of this electrolysis occurs so it can destroy 
// the probe (YL-69) pretty fast in high-moisture soils. To bypass this, 
// instead of directly linking the VCC to the Arduino's VCC/5V we simply link it to a digital pin 
// and power it (digital pin goes HIGH) only before we do a readout (see the code for this).
// 
// 
// ESP32 Pinout:
// 14 -> VCC
// A0 -> GPIO 32
// 
// 

#include "HAPPluginHygrometer.hpp"
#include "HAPServer.hpp"

#define HAP_PLUGIN_INTERVAL 5000

#define VERSION_MAJOR       0
#define VERSION_MINOR       0
#define VERSION_REVISION    5
#define VERSION_BUILD       1

// 
// Set these values in the HAPGlobals.hpp
// 
#ifndef HAP_PLUGIN_HYGROMETER_USE_DUMMY
#define HAP_PLUGIN_HYGROMETER_USE_DUMMY 	0
#endif


#ifndef HAP_PLUGIN_HYGROMTER_REFERENCE
#define HAP_PLUGIN_HYGROMTER_REFERENCE      		2550    // value if put in a glass of water
#endif

#ifndef HAP_PLUGIN_HYGROMETER_PIN_VCC
#define HAP_PLUGIN_HYGROMETER_PIN_VCC       		A6	// 14
#endif

#ifndef HAP_PLUGIN_HYGROMETER_PIN_ADC
#define HAP_PLUGIN_HYGROMETER_PIN_ADC       		A7	// 32
#endif

#ifndef HAP_HYGROMETER_LEAK_SENSOR_ENABLED
#define HAP_HYGROMETER_LEAK_SENSOR_ENABLED  		1		// enable leak sensor for hygrometer
#endif

#ifndef HAP_HYGROMETER_LEAK_PERCENTAGE
#define HAP_HYGROMETER_LEAK_PERCENTAGE				35		// Level when a "leak" notification is triggered
#endif

#ifndef HAP_DEBUG_HYGROMETER
#define HAP_DEBUG_HYGROMETER 0
#endif

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

	_humidityValue 		= nullptr;

#if HAP_HYGROMETER_LEAK_SENSOR_ENABLED		
	_leakSensor			= nullptr;
	_leakSensorEnabled  = true;
#else
	_leakSensorEnabled  = false;
#endif

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
#else
	LogI("Using Hygrometer dummy!", true);
	randomSeed(analogRead(HAP_PLUGIN_HYGROMETER_PIN_ADC));	
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

#if HAP_DEBUG_HYGROMETER
    Serial.print("[" + _name + "] Moisture: ");
    Serial.print(moisture);
    Serial.print(" = ");
    Serial.print(percentage);
    Serial.println(" %");
#endif

#endif

#if HAP_HYGROMETER_LEAK_SENSOR_ENABLED
	
	if (_leakSensorEnabled) {
		if (percentage < HAP_HYGROMETER_LEAK_PERCENTAGE) {
			_leakSensor->setValue("1");
		} else {
			_leakSensor->setValue("0");
		}
	}


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

	String sn = HAPDeviceID::serialNumber("HY", String(HAP_PLUGIN_HYGROMETER_PIN_VCC) + String(HAP_PLUGIN_HYGROMETER_PIN_VCC));    

	_accessory = new HAPAccessory();	
	auto callbackIdentify = std::bind(&HAPPlugin::identify, this, std::placeholders::_1, std::placeholders::_2);	
	_accessory->addInfoService("Hygrometer", "ACME", "YL-69", sn, callbackIdentify, version());


	// 
	// Leak Sensor
	// 
#if HAP_HYGROMETER_LEAK_SENSOR_ENABLED	
	HAPService* leakService = new HAPService(HAP_SERVICE_LEAK_SENSOR);
	_accessory->addService(leakService);

	
	stringCharacteristics *leakServiceName = new stringCharacteristics(HAP_CHARACTERISTIC_NAME, permission_read, 0);
	leakServiceName->setValue("Hygrometer Sensor");
	_accessory->addCharacteristics(leakService, leakServiceName);

	uint8_t validValues[] = {0, 1};	// 0 = no Leak detected , 1 = leak detected
	_leakSensor = new uint8Characteristics(HAP_CHARACTERISTIC_LEAK_DETECTED, permission_read|permission_notify, 0, 1, 1, unit_none, 2, validValues);
	_leakSensor->setValue("0");
	_accessory->addCharacteristics(leakService, _leakSensor);

	// stringCharacteristics *humServiceName = new stringCharacteristics(HAP_CHARACTERISTIC_NAME, permission_read, 0);
	// humServiceName->setValue("Soil Moisture Sensor");
	// _accessory->addCharacteristics(humidityService, humServiceName);

	_humidityValue = new floatCharacteristics(HAP_CHARACTERISTIC_CURRENT_RELATIVE_HUMIDITY, permission_read|permission_notify, 0, 100, 0.1, unit_percentage);
	_humidityValue->setValue("0.0");

	auto callbackChangeHum = std::bind(&HAPPluginHygrometer::changeHum, this, std::placeholders::_1, std::placeholders::_2);
	//_humidityValue->valueChangeFunctionCall = std::bind(&changeHum);
	_humidityValue->valueChangeFunctionCall = callbackChangeHum;
	_accessory->addCharacteristics(leakService, _humidityValue);

#else

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

#endif


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

#if HAP_HYGROMETER_LEAK_SENSOR_ENABLED	
    HAPConfigValidationResult result;


    result = HAPPlugin::validateConfig(object);
    if (result.valid == false) {
        return result;
    }
    result.valid = false;
    
    // plugin._name.leakEnabled
    if (object.containsKey("leakEnabled") && !object["leakEnabled"].is<bool>()) {
        result.reason = "plugins." + _name + ".leakEnabled is not a bool";
        return result;
    }

    result.valid = true;
    return result;
#else	
	return HAPPlugin::validateConfig(object);
#endif

}

JsonObject HAPPluginHygrometer::getConfigImpl(){
	LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Get config implementation", true);

    DynamicJsonDocument doc(128);

#if HAP_HYGROMETER_LEAK_SENSOR_ENABLED		
    doc["leakEnabled"] = _leakSensorEnabled;
#endif

#if HAP_DEBUG_CONFIG
    serializeJson(doc, Serial);
    Serial.println();
#endif

    doc.shrinkToFit();
	return doc.as<JsonObject>();
}

void HAPPluginHygrometer::setConfigImpl(JsonObject root){
    
#if HAP_HYGROMETER_LEAK_SENSOR_ENABLED	
	if (root.containsKey("leakEnabled")){
        // LogD(" -- password: " + String(root["password"]), true);
        _leakSensorEnabled = root["leakEnabled"].as<bool>();
    }
#endif

}


bool HAPPluginHygrometer::fakeGatoCallback(){		
	// return _fakegato.addEntry(0x02, "0", _humidityValue->value(), "0");
	return _fakegato.addEntry(_humidityValue->value());
}


uint16_t HAPPluginHygrometer::readSensor() {

    digitalWrite(HAP_PLUGIN_HYGROMETER_PIN_VCC, HIGH);
    delay(100);

    uint16_t value = analogRead(HAP_PLUGIN_HYGROMETER_PIN_ADC);
    digitalWrite(HAP_PLUGIN_HYGROMETER_PIN_VCC, LOW);

    return 4095 - value;
}