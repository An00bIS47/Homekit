//
// HAPPluginNeoPixel.cpp
// Homekit
//
//  Created on: 11.09.2019
//      Author: michael
//
#include "HAPPluginNeoPixel.hpp"
#include "HAPServer.hpp"

#define HAP_BLINK_INTERVAL 1000	


#define VERSION_MAJOR       0
#define VERSION_MINOR       0
#define VERSION_REVISION    3	// 2 = FakeGato support
#define VERSION_BUILD       1





HAPPluginNeoPixel::HAPPluginNeoPixel(){
    _type = HAP_PLUGIN_TYPE_ACCESSORY;
    _name = "NeoPixel";
    _isEnabled = HAP_PLUGIN_USE_NEOPIXEL;
    _interval = HAP_BLINK_INTERVAL;
    _previousMillis = 0;
    _isOn = false;
    _gpio = DATA_PIN;

    _version.major      = VERSION_MAJOR;
    _version.minor      = VERSION_MINOR;
    _version.revision   = VERSION_REVISION;
    _version.build      = VERSION_BUILD;
}

void HAPPluginNeoPixel::changePower(bool oldValue, bool newValue) {
    LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Setting iid " + String(_powerState->iid) +  " oldValue: " + oldValue + " -> newValue: " + newValue, true);

    if (newValue == true) {
        // digitalWrite(_gpio, HIGH);     // dont know why to put low here, maybe because of SPI ?  
    } else {
        // digitalWrite(_gpio, LOW);
    }      
    _pixels->show();
}

#if HAP_PLUGIN_NEOPIXEL_ENABLE_BRIGHTNESS	

void HAPPluginNeoPixel::changeBrightness(int oldValue, int newValue){
    printf("New brightness state: %d\n", newValue);

    rgb_t rgb;

    hsi2rgb(_hue->value().toFloat(), newValue, _brightnessState->value().toFloat(), &rgb);

    Serial.printf("r: %d - g: %d - b: %d \n", rgb.r, rgb.g, rgb.b);


    _pixels->setPixelColor(0, _pixels->Color(rgb.r, rgb.b, rgb.g));
    _pixels->show();
}   

#endif

void HAPPluginNeoPixel::changeHue(float oldValue, float newValue){
    printf("New hue state: %.2f\n", newValue);
    rgb_t rgb;

    hsi2rgb(newValue, _saturation->value().toFloat(), _brightnessState->value().toFloat(), &rgb);
    
    Serial.printf("r: %d - g: %d - b: %d \n", rgb.r, rgb.g, rgb.b);
    
    _pixels->setPixelColor(0, _pixels->Color(rgb.r, rgb.b, rgb.g));
    // _hue->setValue(String(newValue));
    _pixels->show();
}



void HAPPluginNeoPixel::changeSaturation(float oldValue, float newValue){
    printf("New saturation state: %.2F\n", newValue);
    
    rgb_t rgb;
    hsi2rgb(_hue->value().toFloat(), _saturation->value().toFloat(), newValue, &rgb);

    Serial.printf("r: %d - g: %d - b: %d \n", rgb.r, rgb.g, rgb.b);
    _pixels->setPixelColor(0, _pixels->Color(rgb.r, rgb.b, rgb.g));

    // _brightnessState->setValue(String(newValue));
    _pixels->show();
}


void HAPPluginNeoPixel::handleImpl(bool forced){
    
    LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Handle plguin [" + String(_interval) + "]", true);

    // if (_isOn) {            
    //     _pixels->setPixelColor(0, _pixels->Color(0, 255, 0));
    //     setValue(_powerState->iid, "1", "0");
    // } else {
    //     _pixels->setPixelColor(0, _pixels->Color(255, 0, 0));          
    //     setValue(_powerState->iid, "0", "1");
    // }

    // _pixels->show();

    // Add event
    // struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _powerState->iid, _powerState->value());							
    // _eventManager->queueEvent( EventManager::kEventNotifyController, event);

#if HAP_PLUGIN_NEOPIXEL_ENABLE_BRIGHTNESS	
    // uint32_t freeMem = ESP.getFreeHeap();        
    // uint8_t percentage = ( freeMem * 100) / 245000;        
    
    // setValue(_brightnessState->iid, _brightnessState->value(), String(percentage));

    // Add event
    // struct HAPEvent eventB = HAPEvent(nullptr, _accessory->aid, _brightnessState->iid, _brightnessState->value());							
    // _eventManager->queueEvent( EventManager::kEventNotifyController, eventB);
#endif        

}

bool HAPPluginNeoPixel::begin(){

    _pixels = new Adafruit_NeoPixel(NUM_LEDS, DATA_PIN, HAP_PLUGIN_NEOPIXEL_FORMAT);
    _pixels->begin();
    return true;
}



HAPAccessory* HAPPluginNeoPixel::initAccessory(){
 

	_accessory = new HAPAccessory();
	//HAPAccessory::addInfoServiceToAccessory(_accessory, "Builtin LED", "ACME", "LED", "123123123", &identify);
    auto callbackIdentify = std::bind(&HAPPluginNeoPixel::identify, this, std::placeholders::_1, std::placeholders::_2);
    _accessory->addInfoService("NeoPixel", "ACME", "NeoPix", "321123", callbackIdentify, version());

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

#if HAP_PLUGIN_NEOPIXEL_ENABLE_BRIGHTNESS	    
    _brightnessState = new intCharacteristics(HAP_CHARACTERISTIC_BRIGHTNESS, permission_read|permission_write|permission_notify, 0, 100, 1, unit_percentage);
        //_brightnessState->valueChangeFunctionCall = &changeBrightness;

    _brightnessState->setValue("50");
    auto callbackBrightness = std::bind(&HAPPluginNeoPixel::changeBrightness, this, std::placeholders::_1, std::placeholders::_2);        
    _brightnessState->valueChangeFunctionCall = callbackBrightness;
    _accessory->addCharacteristics(_service, _brightnessState);    
#endif


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


void HAPPluginNeoPixel::setValue(int iid, String oldValue, String newValue){
    LogD(HAPServer::timeString() + " " + "HAPPluginNeoPixel" + "->" + String(__FUNCTION__) + " [   ] " + "Setting iid " + String(iid) +  " oldValue: " + oldValue + " -> newValue: " + newValue, true);

     if (iid == _powerState->iid) {
        
        // if (newValue == "1"){
        //     _isOn = true;
        // } else {
        //     _isOn = false;
        // }    

        // _powerState->setValue(newValue);
    }

    else if (iid == _hue->iid) {    

    }

    else if (iid == _saturation->iid) { 

    }

#if HAP_PLUGIN_NEOPIXEL_ENABLE_BRIGHTNESS	    
     else if (iid == _brightnessState->iid) {

    }
#endif

    else {
        // not a known iid 
        //  return w/o event
        return;
    }

    

    struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, iid, newValue);							
    _eventManager->queueEvent( EventManager::kEventNotifyController, event);
}



String HAPPluginNeoPixel::getValue(int iid){
    LogE(HAPServer::timeString() + " " + "HAPPluginNeoPixel" + "->" + String(__FUNCTION__) + " [   ] " + "Getting iid " + String(iid), true);
    if (iid == _powerState->iid) {
        return _powerState->value();
    } 
    else if (iid == _brightnessState->iid) {
        return _brightnessState->value();
    }
    else if (iid == _hue->iid) {
        return _hue->value();
    }
    else if (iid == _saturation->iid) {
        return _saturation->value();
    }
    return "";
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
    result.valid = false;
    
    // plugin._name.gpio
    if (object.containsKey("gpio") && !object["gpio"].is<uint8_t>()) {
        result.reason = "plugins." + _name + ".gpio is not an integer";
        return result;
    }

    result.valid = true;
    return result;
}

JsonObject HAPPluginNeoPixel::getConfigImpl(){
    DynamicJsonDocument doc(128);
    doc["gpio"] = _gpio;

	return doc.as<JsonObject>();
}

void HAPPluginNeoPixel::setConfigImpl(JsonObject root){
    if (root.containsKey("gpio")){
        // LogD(" -- password: " + String(root["password"]), true);
        _gpio = root["gpio"].as<uint8_t>();
    }
}


//http://blog.saikoled.com/post/44677718712/how-to-convert-from-hsi-to-rgb-white
void HAPPluginNeoPixel::hsi2rgb(float H, float S, float I, rgb_t* rgbw) {    
    
    uint8_t r, g, b;
    
    H = fmod(H,360); // cycle H around to 0-360 degrees
    H = 3.14159*H/(float)180; // Convert to radians.
    S = S>0?(S<1?S:1):0; // clamp S and I to interval [0,1]
    I = I>0?(I<1?I:1):0;
        
    // Math! Thanks in part to Kyle Miller.
    if(H < 2.09439) {
        r = 255*I/3*(1+S*cos(H)/cos(1.047196667-H));
        g = 255*I/3*(1+S*(1-cos(H)/cos(1.047196667-H)));
        b = 255*I/3*(1-S);
    } else if(H < 4.188787) {
        H = H - 2.09439;
        g = 255*I/3*(1+S*cos(H)/cos(1.047196667-H));
        b = 255*I/3*(1+S*(1-cos(H)/cos(1.047196667-H)));
        r = 255*I/3*(1-S);
    } else {
        H = H - 4.188787;
        b = 255*I/3*(1+S*cos(H)/cos(1.047196667-H));
        r = 255*I/3*(1+S*(1-cos(H)/cos(1.047196667-H)));
        g = 255*I/3*(1-S);
    }
    rgbw->r = r;
    rgbw->g = g;
    rgbw->b = b;
	rgbw->w = 0;           // white channel is not used
}