//
// HAPPluginDHT.cpp
// Homekit
//
//  Created on: 29.04.2018
//      Author: michael
//
#include "HAPPluginDHT.hpp"
#include "HAPServer.hpp"

#define HAP_PLUGIN_INTERVAL 5000

#define VERSION_MAJOR       0
#define VERSION_MINOR       0
#define VERSION_REVISION    3
#define VERSION_BUILD       1


HAPPluginDHT::HAPPluginDHT(){
	_type = HAP_PLUGIN_TYPE_ACCESSORY;
	_name = "DHT";
	_isEnabled = HAP_PLUGIN_USE_DHT;
	_interval = HAP_PLUGIN_INTERVAL;
	_previousMillis = 0;

    _version.major      = VERSION_MAJOR;
    _version.minor      = VERSION_MINOR;
    _version.revision   = VERSION_REVISION;
    _version.build      = VERSION_BUILD;
}


bool HAPPluginDHT::begin(){
	LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Begin()", true);
#if HAP_PLUGIN_DHT_USE_DUMMY
	LogI("Using DHT dummy!", true);
	randomSeed(analogRead(0));
#else
	_dht = new DHT_Unified(DHTPIN, DHTTYPE);
	_dht->begin();
#endif
	return true;
}


void HAPPluginDHT::identify(bool oldValue, bool newValue) {
	Serial.printf("Start Identify %s\n", _name.c_str());
}

void HAPPluginDHT::changeTemp(float oldValue, float newValue) {
	Serial.printf("[%s] New temperature: %f\n", _name.c_str(), newValue);
}

void HAPPluginDHT::changeHum(float oldValue, float newValue) {
	Serial.printf("[%s] New humidity: %f\n", _name.c_str(), newValue);
}

#if HAP_PLUGIN_DHT_USE_PRESSURE
void HAPPluginDHT::changePressure(uint16_t oldValue, uint16_t newValue) {
	Serial.printf("New pressure: %d\n", newValue);
}
#endif


void HAPPluginDHT::handleImpl(bool forced){	
	
		LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Handle plguin [" + String(_interval) + "]", true);

		sensors_event_t sensorEventTemp;
		sensors_event_t sensorEventHum;
#if HAP_PLUGIN_DHT_USE_PRESSURE		
		sensors_event_t sensorEventPressure;
#endif

#if HAP_PLUGIN_DHT_USE_DUMMY

		sensorEventTemp.temperature 		= random(200, 401) / 10.0;
		sensorEventHum.relative_humidity 	= random(200, 401) / 10.0;		
		// setValue(_temperatureValue->iid, getValue(_temperatureValue->iid), String(sensorEvent.temperature) );
		// setValue(_humidityValue->iid, getValue(_humidityValue->iid), String(sensorEvent.relative_humidity) );
#else

		_dht->temperature().getEvent(&sensorEventTemp);
		_dht->humidity().getEvent(&sensorEventHum);  	
#endif

  		if (!isnan(sensorEventTemp.temperature)) {
    		setValue(_temperatureValue->iid, _temperatureValue->value(), String(sensorEventTemp.temperature) );
  		}

		if (!isnan(sensorEventHum.relative_humidity)) {
    		setValue(_humidityValue->iid, _humidityValue->value(), String(sensorEventHum.relative_humidity) );
  		}

#if HAP_PLUGIN_DHT_USE_PRESSURE

		sensorEventPressure.pressure 		= random(30, 110) * 10;

		if (!isnan(sensorEventPressure.pressure)) {
    		setValue(_pressureValue->iid, getValue(_pressureValue->iid), String(sensorEventPressure.pressure) );
  		}
#endif


}

// void HAPPluginDHT::handleEvents(int eventCode, struct HAPEvent eventParam){
// 	LogE("!!!!!!!!!!! HANDLE PLUGIN EVENT !!!!!!!!!!!!!!!", true);
// }


void HAPPluginDHT::setValue(int iid, String oldValue, String newValue){
	if (iid == _temperatureValue->iid) {		
		_temperatureValue->setValue(newValue);
	} else if (iid == _humidityValue->iid) {
		//LogW("Setting DHT HUMIDITY oldValue: " + oldValue + " -> newValue: " + newValue, true);
		_humidityValue->setValue(newValue);
	} 
#if HAP_PLUGIN_DHT_USE_PRESSURE	
	else if (iid == _pressureValue->iid) {
		//LogW("Setting DHT HUMIDITY oldValue: " + oldValue + " -> newValue: " + newValue, true);
		_pressureValue->setValue(newValue);
	}
#endif
	struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, iid, newValue);							
	_eventManager->queueEvent( EventManager::kEventNotifyController, event);	
}



HAPAccessory* HAPPluginDHT::initAccessory(){
	LogD("\nInitializing plugin: " + _name + " ...", false);

#if HAP_PLUGIN_DHT_USE_DUMMY
#else


	sensor_t sensor;

	_dht->temperature().getSensor(&sensor);	
#endif


	_accessory = new HAPAccessory();
	

	auto callbackIdentify = std::bind(&HAPPlugin::identify, this, std::placeholders::_1, std::placeholders::_2);
#if HAP_PLUGIN_DHT_USE_DUMMY
	String sn = md5(HAPDeviceID::deviceID() + _name);
	_accessory->addInfoService("DHT Sensor", "ACME", "DHT", sn, callbackIdentify, version());
#else    
	String sn = md5(HAPDeviceID::deviceID() + _name + String(sensor.version));
	_accessory->addInfoService("DHT Sensor", "ACME", sensor.name, sn, callbackIdentify, version());
#endif


	// 
	// Weather
	// 
	// HAPService* weatherService = new HAPService(service_FG_weather);
	// _accessory->addService(weatherService);
	// floatCharacteristics* curTemperatureValue = new floatCharacteristics(charType_currentTemperature, permission_read|permission_notify, -50, 100, 0.1, unit_celsius);
	// _accessory->addCharacteristics(weatherService, curTemperatureValue);

	//
	// Temperature
	//
	HAPService* temperatureService = new HAPService(HAP_SERVICE_TEMPERATURE_SENSOR);
	_accessory->addService(temperatureService);

	stringCharacteristics *tempServiceName = new stringCharacteristics(HAP_CHARACTERISTIC_NAME, permission_read, 0);
	tempServiceName->setValue("Temperature Sensor");

	_accessory->addCharacteristics(temperatureService, tempServiceName);

	//floatCharacteristics(uint8_t _type, int _permission, float minVal, float maxVal, float step, unit charUnit): characteristics(_type, _permission), _minVal(minVal), _maxVal(maxVal), _step(step), _unit(charUnit)
	_temperatureValue = new floatCharacteristics(HAP_CHARACTERISTIC_CURRENT_TEMPERATURE, permission_read|permission_notify, -50, 100, 0.1, unit_celsius);
	_temperatureValue->setValue("0.0");
	auto callbackChangeTemp = std::bind(&HAPPluginDHT::changeTemp, this, std::placeholders::_1, std::placeholders::_2);
	//_temperatureValue->valueChangeFunctionCall = std::bind(&changeTemp);
	_temperatureValue->valueChangeFunctionCall = callbackChangeTemp;
	_accessory->addCharacteristics(temperatureService, _temperatureValue);


#if HAP_PLUGIN_DHT_USE_DUMMY
#else
	_dht->humidity().getSensor(&sensor);	
#endif

	//
	// Humidity
	//
	HAPService* humidityService = new HAPService(HAP_SERVICE_HUMIDITY_SENSOR);
	_accessory->addService(humidityService);

	stringCharacteristics *humServiceName = new stringCharacteristics(HAP_CHARACTERISTIC_NAME, permission_read, 0);
	humServiceName->setValue("Humidity Sensor");
	_accessory->addCharacteristics(humidityService, humServiceName);

	_humidityValue = new floatCharacteristics(HAP_CHARACTERISTIC_CURRENT_RELATIVE_HUMIDITY, permission_read|permission_notify, 0, 100, 0.1, unit_percentage);
	_humidityValue->setValue("0.0");

	auto callbackChangeHum = std::bind(&HAPPluginDHT::changeHum, this, std::placeholders::_1, std::placeholders::_2);
	//_humidityValue->valueChangeFunctionCall = std::bind(&changeHum);
	_humidityValue->valueChangeFunctionCall = callbackChangeHum;
	_accessory->addCharacteristics(humidityService, _humidityValue);

#if HAP_PLUGIN_DHT_USE_PRESSURE
	//
	// AirPressure
	//
	HAPService* pressureService = new HAPService(serviceType_FG_pressure);
	_accessory->addService(pressureService);

	stringCharacteristics *pressureServiceName = new stringCharacteristics(charType_serviceName, permission_read, 0);
	pressureServiceName->setValue("Pressure Sensor");
	_accessory->addCharacteristics(pressureService, pressureServiceName);

	_pressureValue = new uint16Characteristics(charType_FG_airPressure, permission_read|permission_notify, 300, 1100, 1, unit_hpa);
	_pressureValue->setValue("700");

	auto callbackChangePressure = std::bind(&HAPPluginDHT::changePressure, this, std::placeholders::_1, std::placeholders::_2);
	//_humidityValue->valueChangeFunctionCall = std::bind(&changeHum);
	_pressureValue->valueChangeFunctionCall = callbackChangePressure;
	_accessory->addCharacteristics(pressureService, _pressureValue);
#endif



	//
	// FakeGato
	// 	
	// _fakegato.begin();	
	_fakegato.registerFakeGatoService(_accessory, _name);
	auto callbackAddEntry = std::bind(&HAPPluginDHT::fakeGatoCallback, this);
	// _fakegato.registerCallback(callbackAddEntry);
	registerFakeGato(&_fakegato, _name, callbackAddEntry);



	LogD("OK", true);

	return _accessory;
}

HAPConfigValidationResult HAPPluginDHT::validateConfig(JsonObject object){
    return HAPPlugin::validateConfig(object);
}

JsonObject HAPPluginDHT::getConfigImpl(){
    DynamicJsonDocument doc(1);
	return doc.as<JsonObject>();
}

void HAPPluginDHT::setConfigImpl(JsonObject root){

}


bool HAPPluginDHT::fakeGatoCallback(){	
	// return _fakegato.addEntry(_temperatureValue->value(), _humidityValue->value(), _pressureValue->value());
#if HAP_PLUGIN_DHT_USE_PRESSURE	
	return _fakegato.addEntry(_temperatureValue->value(), _humidityValue->value(), _pressureValue->value());
#else
	return _fakegato.addEntry(_temperatureValue->value(), _humidityValue->value(), "0");
#endif
}