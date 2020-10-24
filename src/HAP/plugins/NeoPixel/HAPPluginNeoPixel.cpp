//
// HAPPluginNeoPixel.cpp
// Homekit
//
//  Created on: 11.09.2019
//      Author: michael
//
#include "HAPPluginNeoPixel.hpp"
#include "HAPServer.hpp"

#define HAP_PLUGIN_NEOPIXEL_INTERVAL 1000	

#define VERSION_MAJOR       0
#define VERSION_MINOR       0
#define VERSION_REVISION    5	// 2 = FakeGato support
#define VERSION_BUILD       1


HAPPluginNeoPixel::HAPPluginNeoPixel(){
    _type = HAP_PLUGIN_TYPE_ACCESSORY;
    _name = "NeoPixel";
    _isEnabled = HAP_PLUGIN_USE_NEOPIXEL;
    _interval = HAP_PLUGIN_NEOPIXEL_INTERVAL;
    _previousMillis = 0;
    _isOn = false;
    _gpio = HAP_PLUGIN_NEOPIXEL_DATA_PIN;

    _version.major      = VERSION_MAJOR;
    _version.minor      = VERSION_MINOR;
    _version.revision   = VERSION_REVISION;
    _version.build      = VERSION_BUILD;

    _hue                = nullptr;
    _saturation         = nullptr; 
    _brightnessState    = nullptr;
    _powerState         = nullptr;
}

void HAPPluginNeoPixel::changePower(bool oldValue, bool newValue) {
    LogE(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Setting iid " + String(_powerState->iid) +  " oldValue: " + oldValue + " -> newValue: " + newValue, true);

    _isOn = newValue;
    if (!newValue) {
        _pixels[0] = CRGB::Black;
        FastLED.show();             
    } else {
        setPixelColor(_hue->value().toInt(), _saturation->value().toInt(), _brightnessState->value().toInt());
    }    
}


void HAPPluginNeoPixel::changeBrightness(int oldValue, int newValue){
    printf("New brightness state: %d\n", newValue);

    setPixelColor(_hue->value().toInt(), _saturation->value().toInt(), newValue);    
}   


void HAPPluginNeoPixel::changeHue(float oldValue, float newValue){
    printf("New hue state: %.2f\n", newValue);    

    setPixelColor((uint16_t)newValue, _saturation->value().toInt(), _brightnessState->value().toInt());
}



void HAPPluginNeoPixel::changeSaturation(float oldValue, float newValue){
    printf("New saturation state: %.2F\n", newValue);

    setPixelColor(_hue->value().toInt(), (uint8_t)newValue, _brightnessState->value().toInt());
}


void HAPPluginNeoPixel::handleImpl(bool forced){       

}

void HAPPluginNeoPixel::setPixelColor(uint16_t hueDegree, uint8_t satPercent, uint8_t briPercent){
        uint8_t hue = map(hueDegree << 7, 0, 46080, 0, 255);
        uint8_t sat = map(satPercent << 2, 0, 400, 0, 255);
        uint8_t bri = map(briPercent << 2, 0, 400, 0, 255);

        CHSV hvsColor = CHSV(hue, sat, bri);
        _pixels[0] = hvsColor;
        FastLED.show();
}

bool HAPPluginNeoPixel::begin(){    
    FastLED.addLeds<NEOPIXEL, HAP_PLUGIN_NEOPIXEL_DATA_PIN>(_pixels, HAP_PLUGIN_NEOPIXEL_NUM_LEDS);  // GRB ordering is assumed
    _pixels[0] = CRGB::Black;
    FastLED.show();   

    return true;
}



HAPAccessory* HAPPluginNeoPixel::initAccessory(){
 
    String sn = HAPDeviceID::serialNumber("NEO", String(_gpio));

	_accessory = new HAPAccessory();
	//HAPAccessory::addInfoServiceToAccessory(_accessory, "Builtin LED", "ACME", "LED", "123123123", &identify);
    auto callbackIdentify = std::bind(&HAPPluginNeoPixel::identify, this, std::placeholders::_1, std::placeholders::_2);
    _accessory->addInfoService("NeoPixel", "ACME", "NeoPix", sn, callbackIdentify, version());

    HAPService* _service = new HAPService(HAP_SERVICE_LIGHTBULB);
    _accessory->addService(_service);

    stringCharacteristics *lightServiceName = new stringCharacteristics(HAP_CHARACTERISTIC_NAME, permission_read, 32);
    lightServiceName->setValue("NeoPixel");
    _accessory->addCharacteristics(_service, lightServiceName);

    _powerState = new boolCharacteristics(HAP_CHARACTERISTIC_ON, permission_read|permission_write|permission_notify);
    if (_isOn)
        _powerState->setValue("1");
    else
        _powerState->setValue("0");
    
    auto callbackPowerState = std::bind(&HAPPluginNeoPixel::changePower, this, std::placeholders::_1, std::placeholders::_2);        
    _powerState->valueChangeFunctionCall = callbackPowerState;
    _accessory->addCharacteristics(_service, _powerState);


    _brightnessState = new intCharacteristics(HAP_CHARACTERISTIC_BRIGHTNESS, permission_read|permission_write|permission_notify, 0, 100, 1, unit_percentage);        
    _brightnessState->setValue("50");
    auto callbackBrightness = std::bind(&HAPPluginNeoPixel::changeBrightness, this, std::placeholders::_1, std::placeholders::_2);        
    _brightnessState->valueChangeFunctionCall = callbackBrightness;
    _accessory->addCharacteristics(_service, _brightnessState);    


    //
    // Hue 
    //
    _hue = new floatCharacteristics(HAP_CHARACTERISTIC_HUE, permission_read|permission_write|permission_notify, 0.0, 360.0, 1.0, unit_arcDegree);
    _hue->setValue("0.0");
    auto callbackHue = std::bind(&HAPPluginNeoPixel::changeHue, this, std::placeholders::_1, std::placeholders::_2);            
    _hue->valueChangeFunctionCall = callbackHue;
    _accessory->addCharacteristics(_service, _hue);


    //
    // Saturation 
    //
    _saturation = new floatCharacteristics(HAP_CHARACTERISTIC_SATURATION, permission_read|permission_write|permission_notify, 0.0, 100.0, 1.0, unit_percentage);
    _saturation->setValue("0.0");
    auto callbackSaturation = std::bind(&HAPPluginNeoPixel::changeSaturation, this, std::placeholders::_1, std::placeholders::_2);        
    _saturation->valueChangeFunctionCall = callbackSaturation;
    _accessory->addCharacteristics(_service, _saturation);


	LogD("OK", true);

	return _accessory;
}


void HAPPluginNeoPixel::identify(bool oldValue, bool newValue) {
    printf("Start Identify Light from member\n");
}

HAPConfigValidationResult HAPPluginNeoPixel::validateConfig(JsonObject object){
    HAPConfigValidationResult result;
    
    result = HAPPlugin::validateConfig(object);
    if (result.valid == false) {
        return result;
    }
    // result.valid = false;
    
    // plugin._name.gpio
    // if (object.containsKey("gpio") && !object["gpio"].is<uint8_t>()) {
    //     result.reason = "plugins." + _name + ".gpio is not an integer";
    //     return result;
    // }

    result.valid = true;
    return result;
}

JsonObject HAPPluginNeoPixel::getConfigImpl(){
    LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Get config implementation", true);

    DynamicJsonDocument doc(128);
    // doc["gpio"] = _gpio;

#if HAP_DEBUG_CONFIG
    serializeJson(doc, Serial);
    Serial.println();
#endif

    doc.shrinkToFit();
	return doc.as<JsonObject>();
}

void HAPPluginNeoPixel::setConfigImpl(JsonObject root){


    // if (root.containsKey("gpio")){
    //     // LogD(" -- password: " + String(root["password"]), true);
    //     _gpio = root["gpio"].as<uint8_t>();
    // }
}
