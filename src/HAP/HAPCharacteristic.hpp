// HAPCharacteristics.hpp
// Homekit
//
//  Created on: 31.03.2018
//      Author: michael
//

#ifndef HAPCHARACTERISTIC_HPP_
#define HAPCHARACTERISTIC_HPP_

#include <Arduino.h>
#include "HAPGlobals.hpp"
#include <functional>
#include <ArduinoJson.h>

#include "HAPCharacteristics.hpp"
#include "HAPCharacteristicsBluetooth.hpp"


#ifndef HAP_UUID
#define HAP_UUID        "%08X-0000-1000-8000-0026BB765291"
#endif

#ifndef HAP_UUID_LENGTH
#define HAP_UUID_LENGTH 36 + 1
#endif


#ifndef HAP_LONG_UUID
#define HAP_LONG_UUID   0
#endif

#define CHAR_TYPE_NULL 0x00


enum {
    permission_read     = 1,
    permission_write    = 1 << 1,
    permission_notify   = 1 << 2,     //Notify = Accessory will notice the controller
    permission_hidden   = 1 << 3      //Hidden 
};

typedef enum {
    unit_none = 0,          // none
    unit_celsius,           // °C
    unit_percentage,        // %
    unit_arcDegree,         // °
    unit_lux,               // lux
    unit_seconds,           // sec

    unit_hpa,               // hPa
    unit_watt,              // W
    unit_voltage,           // V
    unit_kwh,               // kWh
    unit_kmh,               // km/h
    unit_km,                // km
    unit_m,                 // m
    unit_mm,                // mm    
    unit_kelvin,            // K
    unit_DU,                // DU
    unit_mired,             // Mired
    unit_ppm,               // ppm    
} unit;

typedef enum {
    battery_level_normal    = 0,
    battery_level_low       = 1    
} battery_level;

typedef enum {
    NOT_CHARGING            = 0,
    CHARGING                = 1,
    NOT_CHARGEABLE          = 2
} charging_state;


class characteristics {
public:
    
    int type;
    const int permission;
    String typeString;
    int iid;
    String desc;
    
    // std::function<void(int, void*, void*)> genericValueChangeFunctionCall = NULL;
    std::function<void(void)> valueGetCallback = NULL;


    characteristics(int _type, int _permission): type(_type), permission(_permission), typeString(""), desc("") {}
    characteristics(String _typeString, int _permission): type(CHAR_TYPE_NULL), permission(_permission), typeString(_typeString), desc("") {}
    
    virtual String value() = 0;
    virtual void setValue(String str) = 0;
    virtual String describe() = 0;

    virtual void setDescription(String str){
        desc = str;
    }

    virtual void toJson(JsonObject root, bool type_ = false, bool perms = false, bool event = false, bool meta = false) = 0;
    // virtual String unitString() = 0;

    bool writable() { return permission&permission_write; }
    bool notifiable() { return permission&permission_notify; }
    bool readable() { return permission&permission_read; }
    bool hidden() { return permission&permission_hidden; }

    static String unitString(unit unitValue){
        switch (unitValue) {
            case unit_arcDegree:                
                return "°";                
            case unit_celsius:                
                return "°C";                    
            case unit_percentage:
                return "%";
            case unit_lux:
                return "lux";             
            case unit_seconds:
                return "seconds";
                
            case unit_hpa:
                return "hPa";
            case unit_watt:
                return "W";
            case unit_voltage:
                return "V";
            case unit_kwh:
                return "kWh";
            case unit_kmh:
                return "km/h";
            case unit_km:
                return "km";
            case unit_m:
                return "m";
            case unit_mm:
                return "mm";
            case unit_kelvin:
                return "K";
            case unit_DU:
                return "DU";            
            case unit_mired:
                return "Mired";
            case unit_ppm:
                return "ppm";

            case unit_none:
                return "";                
            default:
                return "";
        }
    }

    static String unitJson(unit unitValue){
        switch (unitValue) {
            case unit_arcDegree:                
                return "arcdegrees";                
            case unit_celsius:                
                return "celsius";                    
            case unit_percentage:
                return "percentage";
            case unit_lux:
                return "lux";             
            case unit_seconds:
                return "seconds";

            case unit_hpa:
                return "hPa";
            case unit_watt:
                return "W";
            case unit_voltage:
                return "V";
            case unit_kwh:
                return "kWh";
            case unit_kmh:
                return "km/h";
            case unit_km:
                return "km";
            case unit_m:
                return "m";
            case unit_mm:
                return "mm";
            case unit_kelvin:
                return "K";
            case unit_DU:
                return "DU";            
            case unit_mired:
                return "Mired";
            case unit_ppm:
                return "ppm";

            case unit_none:
                return "";                
            default:
                return "";
        }
    }

    
};



//To store value of device state, subclass the following type
class boolCharacteristics: public characteristics {
public:
    bool _value;
    
    //void (*valueChangeFunctionCall)(bool oldValue, bool newValue) = NULL;
    std::function<void(bool, bool)> valueChangeFunctionCall = NULL;

    boolCharacteristics(int _type, int _permission): characteristics(_type, _permission) {}
    boolCharacteristics(String _typeString, int _permission): characteristics(_typeString, _permission) {}    

    virtual String value() {
        if (_value)
            return "1";
        return "0";
    }

    virtual void setValue(String str) {
        bool newValue = false;
        if ( str == "1") {
            newValue = true;
        } 
        
        if (valueChangeFunctionCall)
            valueChangeFunctionCall(_value, newValue);

        // if (genericValueChangeFunctionCall)
        //     genericValueChangeFunctionCall(iid, (void*)&_value, (void*)&newValue);
    
        _value = newValue;
    }
    

    virtual void toJson(JsonObject root, bool type_ = false, bool perms = false, bool event = false, bool meta = false){
        root["value"] = _value;
        root["iid"] = iid;
        if (perms){
            JsonArray perms = root.createNestedArray("perms");
            if (writable())
                perms.add("pw");
            if (readable())
                perms.add("pr");
            if (notifiable())
                perms.add("ev");   
            if (hidden())
                perms.add("hd");     
        }
        if (event)
            root["ev"] = notifiable();
        if (type_){
            if (type == CHAR_TYPE_NULL) {                
                root["type"] = typeString;
            } else {
                root["type"] = type;
            }            
        }
        if (meta){
            root["format"] = "bool";
        }
        
        if (desc != ""){
            root["description"] = desc;
        }
            
    }

    virtual String describe();
};

class floatCharacteristics: public characteristics {
public:
    float _value;
    const float _minVal, _maxVal, _step;
    const unit _unit;
    
    // void (*valueChangeFunctionCall)(float oldValue, float newValue) = NULL;
    std::function<void(float, float)> valueChangeFunctionCall = NULL;

    floatCharacteristics(int _type, int _permission, float minVal, float maxVal, float step, unit charUnit): characteristics(_type, _permission), _minVal(minVal), _maxVal(maxVal), _step(step), _unit(charUnit) {}
    floatCharacteristics(String _type, int _permission, float minVal, float maxVal, float step, unit charUnit): characteristics(_type, _permission), _minVal(minVal), _maxVal(maxVal), _step(step), _unit(charUnit) {}
    
    
    virtual String value() {
        char temp[16];        
        snprintf(temp, 16, "%.1f", _value);
        return String(temp);
    }
    
    virtual void setValue(String str) {
        float temp = atof(str.c_str());    
        if (temp == temp) {
            if (valueChangeFunctionCall)
                valueChangeFunctionCall(_value, temp);
            // if (genericValueChangeFunctionCall)
            //     genericValueChangeFunctionCall(iid, (void*)&_value, (void*)&temp);
                // //dereferencing void pointer with float typecasting
                // printf("fData = %f\n\n",*((float *)pvData));

            _value = temp;
        }
    }


    

    virtual void toJson(JsonObject root, bool type_ = false, bool perms = false, bool event = false, bool meta = false){
        
        root["value"] = _value * 1.0;
        root["iid"] = iid;

        if (perms){
            JsonArray perms = root.createNestedArray("perms");
            if (writable())
                perms.add("pw");
            if (readable())
                perms.add("pr");
            if (notifiable())
                perms.add("ev"); 
            if (hidden())
                perms.add("hd");       
        }
        if (event)
            root["ev"] = notifiable();
        if (type_){
            if (type == CHAR_TYPE_NULL) {                
                root["type"] = typeString;
            } else {
                root["type"] = type;
            }            
        }
                         
        if (meta){
            root["minValue"] = _minVal;
            root["maxValue"] = _maxVal;
            root["step"] = _step;
            root["unit"] = unitJson(_unit);                
            root["format"] = "float";
        }

        if (desc != ""){
            root["description"] = desc;
        }
    }

    virtual String describe();
};

class intCharacteristics: public characteristics {
public:
    int _value;
    const int _minVal, _maxVal, _step;
    const unit _unit;
    
    // void (*valueChangeFunctionCall)(int oldValue, int newValue) = NULL;
    std::function<void(int, int)> valueChangeFunctionCall = NULL;

    intCharacteristics(int _type, int _permission, int minVal, int maxVal, int step, unit charUnit): characteristics(_type, _permission), _minVal(minVal), _maxVal(maxVal), _step(step), _unit(charUnit) {
        _value = minVal;
    }

    intCharacteristics(String _type, int _permission, int minVal, int maxVal, int step, unit charUnit): characteristics(_type, _permission), _minVal(minVal), _maxVal(maxVal), _step(step), _unit(charUnit) {
        _value = minVal;
    }


    virtual String value() {
        char temp[16];
        snprintf(temp, 16, "%d", _value);
        return String(temp);
    }

    virtual void setValue(String str) {
        float temp = atoi(str.c_str());
        if (temp == temp) {
            if (valueChangeFunctionCall)
                valueChangeFunctionCall(_value, temp);
            // if (genericValueChangeFunctionCall)
            //     genericValueChangeFunctionCall(iid, (void*)&_value, (void*)&temp);
                // //dereferencing void pointer with int typecasting
                // printf("fData = %f\n\n",*((int *)pvData));
            _value = temp;
        }
    }

    virtual void toJson(JsonObject root, bool type_ = false, bool perms = false, bool event = false, bool meta = false){
        
        root["value"] = _value;
        root["iid"] = iid;

        if (perms){
            JsonArray perms = root.createNestedArray("perms");
            if (writable())
                perms.add("pw");
            if (readable())
                perms.add("pr");
            if (notifiable())
                perms.add("ev");   
            if (hidden())
                perms.add("hd");      
        }
        if (event)
            root["ev"] = notifiable();

        if (type_){
            if (type == CHAR_TYPE_NULL) {                
                root["type"] = typeString;
            } else {
                root["type"] = type;
            }            
        }
        
        if (meta){
            root["minValue"] = _minVal;
            root["maxValue"] = _maxVal;
            root["step"] = _step;
            root["unit"] = unitJson(_unit);  
            root["format"] = "int";
        }

        if (desc != ""){
            root["description"] = desc;
        }  
    }


    virtual String describe();
};

class stringCharacteristics: public characteristics {
public:
    String _value;
    const uint8_t maxLen;
    
    
    // void (*valueChangeFunctionCall)(String oldValue, String newValue) = NULL;
    std::function<void(String, String)> valueChangeFunctionCall = NULL;

    stringCharacteristics(int _type, int _permission, uint8_t _maxLen, String _desc = ""): characteristics(_type, _permission), maxLen(_maxLen) {}
    stringCharacteristics(String _type, int _permission, uint8_t _maxLen, String _desc = ""): characteristics(_type, _permission), maxLen(_maxLen) {}

    virtual String value() {
        return "\""+_value+"\"";
    }



    virtual void setValue(String str) {
        if (valueChangeFunctionCall)
            valueChangeFunctionCall(_value, str);
        // if (genericValueChangeFunctionCall)
        //     genericValueChangeFunctionCall(iid, (void*)&_value, (void*)&str);
            // //dereferencing void pointer with String typecasting
            // printf("fData = %f\n\n",*((String *)pvData));
        _value = str;
    }


    virtual void toJson(JsonObject root, bool type_ = false, bool perms = false, bool event = false, bool meta = false){
        
        root["value"] = _value;
        root["iid"] = iid;
        
        if (perms){
            JsonArray perms = root.createNestedArray("perms");
            if (writable())
                perms.add("pw");
            if (readable())
                perms.add("pr");
            if (notifiable())
                perms.add("ev");
            if (hidden())
                perms.add("hd");        
        }
        if (event)
            root["ev"] = notifiable();
        if (type_){
            if (type == CHAR_TYPE_NULL) {                
                root["type"] = typeString;
            } else {
                root["type"] = type;
            }            
        }
        if (meta){
            root["maxLen"] = maxLen;   
            root["format"] = "string";         
        }  

        if (desc != ""){
            root["description"] = desc;
        }  
    }

    virtual String describe();
};

class dataCharacteristics: public characteristics {
public:    
    const size_t maxLen;
    String _value;
    
    // void (*valueChangeFunctionCall)(String oldValue, String newValue) = NULL;
    std::function<void(String, String)> valueChangeFunctionCall = NULL;
    

    dataCharacteristics(int _type, int _permission, size_t _maxLen, String _desc = ""): characteristics(_type, _permission), maxLen(_maxLen) {}
    dataCharacteristics(String _type, int _permission, size_t _maxLen, String _desc = ""): characteristics(_type, _permission), maxLen(_maxLen) {}

    virtual String value() {        
        return "\""+_value+"\"";
    }

    virtual void setValue(String str) {
        if (valueChangeFunctionCall)
            valueChangeFunctionCall(_value, str);
        // if (genericValueChangeFunctionCall)
        //     genericValueChangeFunctionCall(iid, (void*)&_value, (void*)&str);
            // //dereferencing void pointer with String typecasting
            // printf("fData = %f\n\n",*((String *)pvData));
        _value = str;
    }


    virtual void toJson(JsonObject root, bool type_ = false, bool perms = false, bool event = false, bool meta = false){
        
        root["iid"] = iid;
        if (_value == (char*)NULL){
            root["value"] = (char*)NULL;
        } else {
            root["value"] = _value;
        }        
        
        if (perms){
            JsonArray perms = root.createNestedArray("perms");
            if (writable())
                perms.add("pw");
            if (readable())
                perms.add("pr");
            if (notifiable())
                perms.add("ev");
            if (hidden())
                perms.add("hd");        
        }
        if (event)
            root["ev"] = notifiable();
        if (type_){
            if (type == CHAR_TYPE_NULL) {                
                root["type"] = typeString;
            } else {
                root["type"] = type;
            }            
        }
        if (meta){
            root["maxLen"] = maxLen;
            root["format"] = "data";     
        }    

        if (desc != ""){
            root["description"] = desc;
        } 
    }

    virtual String describe();
};



class uint16Characteristics: public characteristics {
public:
    uint16_t _value;
    const uint16_t _minVal, _maxVal, _step;
    const unit _unit;
    
    // void (*valueChangeFunctionCall)(int oldValue, int newValue) = NULL;
    std::function<void(uint16_t, uint16_t)> valueChangeFunctionCall = NULL;

    uint16Characteristics(int _type, int _permission, uint16_t minVal, uint16_t maxVal, uint16_t step, unit charUnit): characteristics(_type, _permission), _minVal(minVal), _maxVal(maxVal), _step(step), _unit(charUnit) {
        _value = minVal;
    }

    uint16Characteristics(String _type, int _permission, uint16_t minVal, uint16_t maxVal, uint16_t step, unit charUnit): characteristics(_type, _permission), _minVal(minVal), _maxVal(maxVal), _step(step), _unit(charUnit) {
        _value = minVal;
    }


    virtual String value() {
        char temp[16];
        snprintf(temp, 16, "%d", _value);
        return String(temp);
    }

    virtual void setValue(String str) {
        uint16_t temp = atoi(str.c_str());
        
        if (valueChangeFunctionCall)
            valueChangeFunctionCall(_value, temp);
        // if (genericValueChangeFunctionCall)
        //     genericValueChangeFunctionCall(iid, (void*)&_value, (void*)&temp);
            // //dereferencing void pointer with int typecasting
            // printf("fData = %f\n\n",*((int *)pvData));
        _value = temp;
        
    }

    virtual void toJson(JsonObject root, bool type_ = false, bool perms = false, bool event = false, bool meta = false){
        
        root["value"] = _value;
        root["iid"] = iid;

        if (perms){
            JsonArray perms = root.createNestedArray("perms");
            if (writable())
                perms.add("pw");
            if (readable())
                perms.add("pr");
            if (notifiable())
                perms.add("ev");   
            if (hidden())
                perms.add("hd");      
        }
        if (event)
            root["ev"] = notifiable();

        if (type_){
            if (type == CHAR_TYPE_NULL) {                
                root["type"] = typeString;
            } else {
                root["type"] = type;
            }            
        }
        
        if (meta){
            root["minValue"] = _minVal;
            root["maxValue"] = _maxVal;
            root["step"] = _step;
            root["unit"] = unitJson(_unit);  
            root["format"] = "uint16"; 
        }

        if (desc != ""){
            root["description"] = desc;
        }
    }


    virtual String describe();
};



class uint8Characteristics: public characteristics {
public:
    uint8_t _value;
    const uint16_t _minVal, _maxVal, _step;
    const unit _unit;
    uint8_t* _validValues;
    uint8_t _validValuesSize;
    
    // void (*valueChangeFunctionCall)(int oldValue, int newValue) = NULL;
    std::function<void(uint16_t, uint16_t)> valueChangeFunctionCall = NULL;

    uint8Characteristics(int _type, int _permission, uint16_t minVal, uint16_t maxVal, uint16_t step, unit charUnit, uint8_t validValuesSize, uint8_t validValues[]): characteristics(_type, _permission), _minVal(minVal), _maxVal(maxVal), _step(step), _unit(charUnit) {
        _value = minVal;

        if (validValuesSize > 0){
            _validValues = (uint8_t*) malloc (validValuesSize * sizeof(uint8_t));
            memcpy(_validValues, validValues, validValuesSize);
            _validValuesSize = validValuesSize;
        } else {
            _validValues = NULL;
            _validValuesSize = 0;
        }           
    }

    uint8Characteristics(String _type, int _permission, uint16_t minVal, uint16_t maxVal, uint16_t step, unit charUnit, uint8_t validValuesSize, uint8_t validValues[]): characteristics(_type, _permission), _minVal(minVal), _maxVal(maxVal), _step(step), _unit(charUnit) {
        _value = minVal;

        if (validValuesSize > 0){
            _validValues = (uint8_t*) malloc (validValuesSize * sizeof(uint8_t));
            memcpy(_validValues, validValues, validValuesSize);
            _validValuesSize = validValuesSize;
        } else {
            _validValues = NULL;
            _validValuesSize = 0;
        } 
    }


    virtual String value() {
        char temp[16];
        snprintf(temp, 16, "%d", _value);
        return String(temp);
    }

    virtual void setValue(String str) {
        uint8_t temp = atoi(str.c_str());
        
        if (valueChangeFunctionCall)
            valueChangeFunctionCall(_value, temp);
        // if (genericValueChangeFunctionCall)
        //     genericValueChangeFunctionCall(iid, (void*)&_value, (void*)&temp);
            // //dereferencing void pointer with int typecasting
            // printf("fData = %f\n\n",*((int *)pvData));
        _value = temp;
        
    }

    virtual void toJson(JsonObject root, bool type_ = false, bool perms = false, bool event = false, bool meta = false){
        
        root["value"] = _value;
        root["iid"] = iid;

        if (perms){
            JsonArray perms = root.createNestedArray("perms");
            if (writable())
                perms.add("pw");
            if (readable())
                perms.add("pr");
            if (notifiable())
                perms.add("ev");   
            if (hidden())
                perms.add("hd");      
        }
        if (event)
            root["ev"] = notifiable();

        if (type_){
            if (type == CHAR_TYPE_NULL) {                
                root["type"] = typeString;
            } else {
                root["type"] = type;
            }            
        }
        
        if (meta){
            root["minValue"] = _minVal;
            root["maxValue"] = _maxVal;
            root["step"] = _step;
            root["unit"] = unitJson(_unit);  
            root["format"] = "uint8"; 

            if (_validValues != NULL){
                JsonArray validValuesJson = root.createNestedArray("valid-values");
                for (int i = 0; i < _validValuesSize; i++){
                    validValuesJson.add(_validValues[i]);
                }
            }
        }

        if (desc != ""){
            root["description"] = desc;
        }
    }


    virtual String describe();
};

#endif /* HAPCHARACTERISTIC_HPP_ */