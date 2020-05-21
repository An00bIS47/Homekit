//
// HAPPluginBME280.cpp
// Homekit
//
//  Created on: 01.08.2019
//      Author: michael
//

#include "HAPPluginBME280.hpp"
#include "HAPServer.hpp"


#define BME280_BASE_ADDRESS        	0x76
#define HAP_PLUGIN_BME280_INTERVAL	5000

#define SDA_PIN				SDA	
#define SCL_PIN				SCL

#define VERSION_MAJOR       0
#define VERSION_MINOR       3	// 2 = FakeGato support
#define VERSION_REVISION    1
#define VERSION_BUILD       2

HAPPluginBME280::HAPPluginBME280(){
	_type = HAP_PLUGIN_TYPE_ACCESSORY;
	_name = "BME280";
	_isEnabled = HAP_PLUGIN_USE_BME280;
	_interval = 0;	// will be set according to selected mode
	_previousMillis = 0;
	_mode = HAP_PLUGIN_BME280_WEATHER;

    _version.major      = VERSION_MAJOR;
    _version.minor      = VERSION_MINOR;
    _version.revision   = VERSION_REVISION;
    _version.build      = VERSION_BUILD;	
}

void HAPPluginBME280::identify(bool oldValue, bool newValue) {
	Serial.printf("Start Identify %s\n", _name.c_str());
}

void HAPPluginBME280::changeTemp(float oldValue, float newValue) {
	Serial.printf("[%s] New temperature: %f\n", _name.c_str(), newValue);
}

void HAPPluginBME280::changeHum(float oldValue, float newValue) {
	Serial.printf("[%s] New humidity: %f\n", _name.c_str(), newValue);
}

void HAPPluginBME280::changePressure(uint16_t oldValue, uint16_t newValue) {
	Serial.printf("[%s] New pressure: %d\n", _name.c_str(), newValue);
}

void HAPPluginBME280::handleImpl(bool forced){	
	// if (shouldHandle() || forced) {		
		LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Handle plguin [" + String(_interval) + "]", true);

		if (_accessory->aid == 0){
			return;
		}

#if HAP_PLUGIN_BME280_USE_DUMMY
		float temperature 			= random(200, 401) / 10.0;
		float relative_humidity 	= random(200, 401) / 10.0;
		float pressure 				= random(30, 110) * 10;
		setValue(_temperatureValue->iid, _temperatureValue->value(), String(temperature) );
#else

		// Only needed in forced mode! In normal mode, you can remove the next line.
    	_bme->takeForcedMeasurement(); // has no effect in normal mode
        
		setValue(_temperatureValue->iid, _temperatureValue->value(), String(_bme->readTemperature()) );
#endif		
		// sensors_event_t sensorEvent;		
		// _dht->temperature().getEvent(&sensorEvent);
  		// if (!isnan(sensorEvent.temperature)) {
    	// 	setValue(charType_currentTemperature, getValue(charType_currentTemperature), String(sensorEvent.temperature) );        
  		// }

#if HAP_PLUGIN_BME280_USE_DUMMY
		setValue(_humidityValue->iid, _humidityValue->value(), String(relative_humidity) );
#else
        setValue(_humidityValue->iid, _humidityValue->value(), String(_bme->readHumidity()) );
		//  _dht->humidity().getEvent(&sensorEvent);  
		//  if (!isnan(sensorEvent.relative_humidity)) {
    	// 	setValue(charType_currentHumidity, getValue(charType_currentHumidity), String(sensorEvent.relative_humidity) );
  		// }
#endif


#if HAP_PLUGIN_BME280_USE_DUMMY
		uint16_t pres = pressure;
		setValue(_pressureValue->iid, _pressureValue->value(), String(pres) );
#else		  
		float pressure = _bme->readPressure() / 100.0F;
		// Serial.print("pressure: ");
		// Serial.println(pressure);
		// uint16_t pres = (uint16_t)pressure;
		setValue(_pressureValue->iid, _pressureValue->value(), String(pressure) );
	// }
#endif

}


// void HAPPluginDHT::handleEvents(int eventCode, struct HAPEvent eventParam){
// 	LogE("!!!!!!!!!!! HANDLE PLUGIN EVENT !!!!!!!!!!!!!!!", true);
// }



void HAPPluginBME280::setValue(int iid, String oldValue, String newValue){
	if (iid == _temperatureValue->iid) {		
		_temperatureValue->setValue(newValue);
	} else if (iid == _humidityValue->iid) {
		_humidityValue->setValue(newValue);
	} else if (iid == _pressureValue->iid) {
		_pressureValue->setValue(newValue);
	}

	// Add event
	struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, iid, newValue);							
	_eventManager->queueEvent( EventManager::kEventNotifyController, event);
}


HAPAccessory* HAPPluginBME280::initAccessory(){
	LogD("\nInitializing plugin: " + _name + " ...", false);

	
    char hex[5];
#if HAP_PLUGIN_BME280_USE_DUMMY
	sprintf(hex, "%x", 0x00);
#else
    sprintf(hex, "%x", _bme->sensorID());
#endif
	_accessory = new HAPAccessory();

	String sn = md5(HAPDeviceID::deviceID() + _name);

	auto callbackIdentify = std::bind(&HAPPlugin::identify, this, std::placeholders::_1, std::placeholders::_2);
   	_accessory->addInfoService("Weather", "ACME", "BME280" + String(hex), sn, callbackIdentify, version());

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
	auto callbackChangeTemp = std::bind(&HAPPluginBME280::changeTemp, this, std::placeholders::_1, std::placeholders::_2);
	//_temperatureValue->valueChangeFunctionCall = std::bind(&changeTemp);
	_temperatureValue->valueChangeFunctionCall = callbackChangeTemp;
	_accessory->addCharacteristics(temperatureService, _temperatureValue);

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

	auto callbackChangeHum = std::bind(&HAPPluginBME280::changeHum, this, std::placeholders::_1, std::placeholders::_2);
	//_humidityValue->valueChangeFunctionCall = std::bind(&changeHum);
	_humidityValue->valueChangeFunctionCall = callbackChangeHum;
	_accessory->addCharacteristics(humidityService, _humidityValue);


	//
	// AirPressure
	//
	HAPService* pressureService = new HAPService(HAP_SERVICE_FAKEGATO_AIR_PRESSURE_SENSOR);
	_accessory->addService(pressureService);

	stringCharacteristics *pressureServiceName = new stringCharacteristics(HAP_CHARACTERISTIC_NAME, permission_read, 0);
	pressureServiceName->setValue("Pressure Sensor");
	_accessory->addCharacteristics(pressureService, pressureServiceName);
	
	_pressureValue = new uint16Characteristics(HAP_CHARACTERISTIC_FAKEGATO_AIR_PRESSURE, permission_read|permission_notify, 0, 1100, 1, unit_hpa);
	_pressureValue->setValue("320");

	auto callbackChangePressure = std::bind(&HAPPluginBME280::changePressure, this, std::placeholders::_1, std::placeholders::_2);
	//_humidityValue->valueChangeFunctionCall = std::bind(&changeHum);
	_pressureValue->valueChangeFunctionCall = callbackChangePressure;
	_accessory->addCharacteristics(pressureService, _pressureValue);


	//
	// FakeGato
	// 	
	// _fakegato.begin();	
	_fakegato.registerFakeGatoService(_accessory, _name);
	auto callbackAddEntry = std::bind(&HAPPluginBME280::fakeGatoCallback, this);
	// _fakegato.registerCallback(callbackAddEntry);
	registerFakeGato(&_fakegato, _name, callbackAddEntry);

	LogD("OK", true);

	return _accessory;
}

HAPConfigValidationResult HAPPluginBME280::validateConfig(JsonObject object){
    HAPConfigValidationResult result;
    
    result = HAPPlugin::validateConfig(object);
    if (result.valid == false) {
        return result;
    }

	/*
        "HAPPluginBME280": {
            "enabled": true,	
            "interval": 1000,
            "mode": 1,
        }
     */

	result.valid = false;

	// plugin._name.mode
    if (object.containsKey("mode") && !object["mode"].is<uint8_t>()) {
        result.reason = "plugins." + _name + ".mode is not a integer";
        return result;
    }

	if (object["mode"].as<uint8_t>() > 3) {
        result.reason = "plugins." + _name + ".mode is not a valid value";
        return result;
    }

	result.valid = true;
    return result;
}

JsonObject HAPPluginBME280::getConfigImpl(){    
    DynamicJsonDocument doc(1);
	doc["mode"] = (uint8_t) _mode;
	return doc.as<JsonObject>();
}

void HAPPluginBME280::setConfigImpl(JsonObject root){
	if (root.containsKey("mode")){
        // LogD(" -- database: " + String(root["database"]), true);
        _mode = (enum HAP_PLUGIN_BME280_MODE) root["mode"].as<uint8_t>();
    }
}

bool HAPPluginBME280::begin(){

	_bme = new Adafruit_BME280();

#if HAP_PLUGIN_BME280_USE_DUMMY

	LogI("Using BME280 dummy!", true);
	_interval = HAP_PLUGIN_BME280_INTERVAL;

#else
	
	Wire.begin(SDA_PIN, SCL_PIN);
	
    uint8_t status = _bme->begin(BME280_BASE_ADDRESS, &Wire);  
    char hex[5];
    sprintf(hex, "%x", _bme->sensorID());

    if (!status) {        
        LogE("\nCould not find a valid BME280 sensor, check wiring, address, sensor ID!", true);
        LogE("SensorID was: 0x" + String(hex), true);
        LogE("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085", true);
        LogE("   ID of 0x56-0x58 represents a BMP 280", true);
        LogE("        ID of 0x60 represents a BME 280", true);
        LogE("        ID of 0x61 represents a BME 680", true);
        LogE("Disabling BME280 Sensor Plugin", true);
		_isEnabled = false;

		_eventManager->removeListener(&_listenerMemberFunctionPlugin);

		return false;
    }


	if (_mode == HAP_PLUGIN_BME280_WEATHER) {
		// Table 7: Settings and performance for weather monitoring
		// Suggested settings for weather monitoring
		// 		Sensor mode: forced mode, 1 sample / minute
		// 		Oversampling settings: pressure *1, temperature *1, humidity *1
		// 		IIR filter settings: filter off
		// 	Performance for suggested settings
		// 		Current consumption 0.16 µA
		// 		RMS Noise 3.3 Pa / 30 cm, 0.07 %RH
		// 		Data output rate: 1/60 Hz
		_bme->setSampling(Adafruit_BME280::MODE_FORCED,
						Adafruit_BME280::SAMPLING_X1,  // temperature
						Adafruit_BME280::SAMPLING_X1, // pressure
						Adafruit_BME280::SAMPLING_X1,  // humidity
						Adafruit_BME280::FILTER_OFF );
		// suggested rate is 1/60Hz (1m)
		_interval = 60000; // in milliseconds

	} else if (_mode == HAP_PLUGIN_BME280_HUMIDITY) {
		// Serial.println("-- Humidity Sensing Scenario --");
		// Serial.println("forced mode, 1x temperature / 1x humidity / 0x pressure oversampling");
		// Serial.println("= pressure off, filter off");
    	_bme->setSampling(Adafruit_BME280::MODE_FORCED,
                    Adafruit_BME280::SAMPLING_X1,   // temperature
                    Adafruit_BME280::SAMPLING_NONE, // pressure
                    Adafruit_BME280::SAMPLING_X1,   // humidity
                    Adafruit_BME280::FILTER_OFF );
                      
    	// suggested rate is 1Hz (1s)
    	_interval = 1000;  // in milliseconds
	
	} else if (_mode == HAP_PLUGIN_BME280_INDOOR) {
		// Serial.println("-- Indoor Navigation Scenario --");
		// Serial.println("normal mode, 16x pressure / 2x temperature / 1x humidity oversampling,");
		// Serial.println("0.5ms standby period, filter 16x");
	    _bme->setSampling(Adafruit_BME280::MODE_NORMAL,
                    Adafruit_BME280::SAMPLING_X2,  // temperature
                    Adafruit_BME280::SAMPLING_X16, // pressure
                    Adafruit_BME280::SAMPLING_X1,  // humidity
                    Adafruit_BME280::FILTER_X16,
                    Adafruit_BME280::STANDBY_MS_0_5 );
    
		// suggested rate is 25Hz
		// 1 + (2 * T_ovs) + (2 * P_ovs + 0.5) + (2 * H_ovs + 0.5)
		// T_ovs = 2
		// P_ovs = 16
		// H_ovs = 1
		// = 40ms (25Hz)
		// with standby time that should really be 24.16913... Hz
		_interval = 41;

	} else if (_mode == HAP_PLUGIN_BME280_GAMING) {
		// gaming
		// Serial.println("-- Gaming Scenario --");
		// Serial.println("normal mode, 4x pressure / 1x temperature / 0x humidity oversampling,");
		// Serial.println("= humidity off, 0.5ms standby period, filter 16x");
		_bme->setSampling(Adafruit_BME280::MODE_NORMAL,
						Adafruit_BME280::SAMPLING_X1,   // temperature
						Adafruit_BME280::SAMPLING_X4,   // pressure
						Adafruit_BME280::SAMPLING_NONE, // humidity
						Adafruit_BME280::FILTER_X16,
						Adafruit_BME280::STANDBY_MS_0_5 );
						
		// Suggested rate is 83Hz
		// 1 + (2 * T_ovs) + (2 * P_ovs + 0.5)
		// T_ovs = 1
		// P_ovs = 4
		// = 11.5ms + 0.5ms standby
		_interval = 12;
	} else {
		_interval = HAP_PLUGIN_BME280_INTERVAL;
	}
  

#endif

	return true;
}

bool HAPPluginBME280::fakeGatoCallback(){	
	// return _fakegato.addEntry(_temperatureValue->value(), _humidityValue->value(), _pressureValue->value());
	return _fakegato.addEntry(_temperatureValue->value(), _humidityValue->value(), _pressureValue->value());
}