// HAPCharacteristics.cpp
// Homekit
//
//  Created on: 31.03.2018
//      Author: michael
//

#include "HAPCharacteristic.hpp"
#include "HAPHelper.hpp"


// 
// bool char describe
// 
inline String attribute(unsigned short type, unsigned short acclaim, int p, bool value, String desc) {
    String result;
    if (p & permission_read) {
        result += HAPHelper::wrap("value")+":";

        // boolean can be 0, 1 and false, true (without quotes!)
        // 
        if (value) result += 1;
        else result += 0;
        result += ",";
    }
    
    result += HAPHelper::wrap("perms")+":";
    result += "[";
    if (p & permission_read) result += HAPHelper::wrap("pr")+",";
    if (p & permission_write) result += HAPHelper::wrap("pw")+",";
    if (p & permission_notify) result += HAPHelper::wrap("ev")+",";
    result = result.substring(0, result.length()-1);
    result += "]";
    result += ",";
    
    
    char uuidStr[8];
    snprintf(uuidStr, 8, "%X", type);    
    result += HAPHelper::wrap("type") + ":" + HAPHelper::wrap(uuidStr);
    result += ",";
    
    char tempStr[4];
    snprintf(tempStr, 4, "%hd", acclaim);
    result += HAPHelper::wrap("iid")+":"+tempStr;
    result += ",";
    
    result += "\"format\":\"bool\"";
    
    return "{"+result+"}";
}


// 
// bool char describe
// 
inline String attribute(String typeString, unsigned short acclaim, int p, bool value, String desc) {
    String result;
    if (p & permission_read) {
        result += HAPHelper::wrap("value")+":";

        // boolean can be 0, 1 and false, true (without quotes!)
        // 
        if (value) result += 1;
        else result += 0;
        result += ",";
    }
    
    result += HAPHelper::wrap("perms")+":";
    result += "[";
    if (p & permission_read) result += HAPHelper::wrap("pr")+",";
    if (p & permission_write) result += HAPHelper::wrap("pw")+",";
    if (p & permission_notify) result += HAPHelper::wrap("ev")+",";
    if (p & permission_hidden) result += HAPHelper::wrap("hd")+",";
    result = result.substring(0, result.length()-1);
    result += "]";
    result += ",";

    result += HAPHelper::wrap("type") + ":" + HAPHelper::wrap(typeString);
    result += ",";
    
    char tempStr[4];
    snprintf(tempStr, 4, "%hd", acclaim);
    result += HAPHelper::wrap("iid")+":"+tempStr;
    result += ",";
    

    if (desc != "") {
        result += "\"description\":" + HAPHelper::wrap(desc.c_str());
        result += ",";        
    }

    result += "\"format\":\"bool\"";
    
    return "{"+result+"}";
}


// 
// int char describe
// 
inline String attribute(unsigned short type, unsigned short acclaim, int p, int value, int minVal, int maxVal, int step, unit valueUnit, String desc) {
    String result;
    char tempStr[4];
    
    snprintf(tempStr, 4, "%d", value);
    
    if (p & permission_read) {
        result += HAPHelper::wrap("value")+":"+tempStr;
        result += ",";
    }
    
    snprintf(tempStr, 4, "%d", minVal);
    if (minVal != INT32_MIN)
        result += HAPHelper::wrap("minValue")+":"+tempStr+",";
    
    snprintf(tempStr, 4, "%d", maxVal);
    if (maxVal != INT32_MAX)
        result += HAPHelper::wrap("maxValue")+":"+tempStr+",";
    
    snprintf(tempStr, 4, "%d", step);
    if (step > 0)
        result += HAPHelper::wrap("minStep")+":"+tempStr+",";
    
    result += HAPHelper::wrap("perms")+":";
    result += "[";
    if (p & permission_read) result += HAPHelper::wrap("pr")+",";
    if (p & permission_write) result += HAPHelper::wrap("pw")+",";
    if (p & permission_notify) result += HAPHelper::wrap("ev")+",";
    if (p & permission_hidden) result += HAPHelper::wrap("hd")+",";
    result = result.substring(0, result.length()-1);
    result += "]";
    result += ",";

  
    char uuidStr[8];
    snprintf(uuidStr, 8, "%X", type);    


    result += HAPHelper::wrap("type") + ":" + HAPHelper::wrap(uuidStr);
    result += ",";
    
    snprintf(tempStr, 4, "%hd", acclaim);
    result += HAPHelper::wrap("iid")+":"+tempStr;
    result += ",";
    result += HAPHelper::wrap("unit")+":"+HAPHelper::wrap(characteristics::unitJson(valueUnit))+",";

    

    if (desc != "") {
        result += "\"description\":" + HAPHelper::wrap(desc.c_str());
        result += ",";        
    }

    result += "\"format\":\"int\"";
    
    return "{"+result+"}";
}

// 
// int char describe
// 
inline String attribute(String typeString, unsigned short acclaim, int p, int value, int minVal, int maxVal, int step, unit valueUnit, String desc) {
    String result;
    char tempStr[4];
    
    snprintf(tempStr, 4, "%d", value);
    
    if (p & permission_read) {
        result += HAPHelper::wrap("value")+":"+tempStr;
        result += ",";
    }
    
    snprintf(tempStr, 4, "%d", minVal);
    if (minVal != INT32_MIN)
        result += HAPHelper::wrap("minValue")+":"+tempStr+",";
    
    snprintf(tempStr, 4, "%d", maxVal);
    if (maxVal != INT32_MAX)
        result += HAPHelper::wrap("maxValue")+":"+tempStr+",";
    
    snprintf(tempStr, 4, "%d", step);
    if (step > 0)
        result += HAPHelper::wrap("minStep")+":"+tempStr+",";
    
    result += HAPHelper::wrap("perms")+":";
    result += "[";
    if (p & permission_read) result += HAPHelper::wrap("pr")+",";
    if (p & permission_write) result += HAPHelper::wrap("pw")+",";
    if (p & permission_notify) result += HAPHelper::wrap("ev")+",";
    if (p & permission_hidden) result += HAPHelper::wrap("hd")+",";
    result = result.substring(0, result.length()-1);
    result += "]";
    result += ",";


    result += HAPHelper::wrap("type") + ":" + HAPHelper::wrap(typeString);
    result += ",";
    
    snprintf(tempStr, 4, "%hd", acclaim);
    result += HAPHelper::wrap("iid")+":"+tempStr;
    result += ",";
    
    result += HAPHelper::wrap("unit")+":"+HAPHelper::wrap(characteristics::unitJson(valueUnit))+",";

    

    if (desc != "") {
        result += "\"description\":" + HAPHelper::wrap(desc.c_str());
        result += ",";        
    }

    result += "\"format\":\"int\"";
    
    return "{"+result+"}";
}


// 
// uint16 char describe
// 
inline String attribute(unsigned short type, unsigned short acclaim, int p, uint16_t value, uint16_t minVal, uint16_t maxVal, uint16_t step, unit valueUnit, String desc) {
    String result;
    char tempStr[16];
    
    snprintf(tempStr, 16, "%d", value);
    
    if (p & permission_read) {
        result += HAPHelper::wrap("value")+":"+tempStr;
        result += ",";
    }
    
    snprintf(tempStr, 16, "%d", minVal);
    if (minVal != INT16_MIN)
        result += HAPHelper::wrap("minValue")+":"+tempStr+",";
    
    snprintf(tempStr, 16, "%d", maxVal);
    if (maxVal != UINT16_MAX)
        result += HAPHelper::wrap("maxValue")+":"+tempStr+",";
    
    snprintf(tempStr, 16, "%d", step);
    if (step > 0)
        result += HAPHelper::wrap("minStep")+":"+tempStr+",";
    
    result += HAPHelper::wrap("perms")+":";
    result += "[";
    if (p & permission_read) result += HAPHelper::wrap("pr")+",";
    if (p & permission_write) result += HAPHelper::wrap("pw")+",";
    if (p & permission_notify) result += HAPHelper::wrap("ev")+",";
    if (p & permission_hidden) result += HAPHelper::wrap("hd")+",";
    result = result.substring(0, result.length()-1);
    result += "]";
    result += ",";

// #if HAP_LONG_UUID    
//     char uuidStr[HAP_UUID_LENGTH];
//     snprintf(uuidStr, HAP_UUID_LENGTH, HAP_UUID, type);
// #else    
    char uuidStr[8];
    snprintf(uuidStr, 8, "%X", type);    
// #endif

    result += HAPHelper::wrap("type") + ":" + HAPHelper::wrap(uuidStr);
    result += ",";
    
    snprintf(tempStr, 4, "%hd", acclaim);
    result += HAPHelper::wrap("iid")+":"+tempStr;
    result += ",";
    result += HAPHelper::wrap("unit")+":"+HAPHelper::wrap(characteristics::unitJson(valueUnit))+",";

    

    if (desc != "") {
        result += "\"description\":" + HAPHelper::wrap(desc.c_str());
        result += ",";        
    }

    result += "\"format\":\"uint16\"";
    
    return "{"+result+"}";
}


// 
// uint16 char describe
// 
inline String attribute(String typeString, unsigned short acclaim, int p, uint16_t value, uint16_t minVal, uint16_t maxVal, uint16_t step, unit valueUnit, String desc) {
    String result;
    char tempStr[16];
    
    snprintf(tempStr, 16, "%d", value);
    
    if (p & permission_read) {
        result += HAPHelper::wrap("value")+":"+tempStr;
        result += ",";
    }
    
    snprintf(tempStr, 16, "%d", minVal);
    if (minVal != INT16_MIN)
        result += HAPHelper::wrap("minValue")+":"+tempStr+",";
    
    snprintf(tempStr, 16, "%d", maxVal);
    if (maxVal != UINT16_MAX)
        result += HAPHelper::wrap("maxValue")+":"+tempStr+",";
    
    snprintf(tempStr, 16, "%d", step);
    if (step > 0)
        result += HAPHelper::wrap("minStep")+":"+tempStr+",";
    
    result += HAPHelper::wrap("perms")+":";
    result += "[";
    if (p & permission_read) result += HAPHelper::wrap("pr")+",";
    if (p & permission_write) result += HAPHelper::wrap("pw")+",";
    if (p & permission_notify) result += HAPHelper::wrap("ev")+",";
    if (p & permission_hidden) result += HAPHelper::wrap("hd")+",";
    result = result.substring(0, result.length()-1);
    result += "]";
    result += ",";


    result += HAPHelper::wrap("type") + ":" + HAPHelper::wrap(typeString);
    result += ",";
    
    snprintf(tempStr, 4, "%hd", acclaim);
    result += HAPHelper::wrap("iid")+":"+tempStr;
    result += ",";
    
    result += HAPHelper::wrap("unit")+":"+HAPHelper::wrap(characteristics::unitJson(valueUnit))+",";
    

    if (desc != "") {
        result += "\"description\":" + HAPHelper::wrap(desc.c_str());
        result += ",";        
    }

    result += "\"format\":\"uint16\"";
    
    return "{"+result+"}";
}



// 
// float char describe
// 
inline String attribute(unsigned short type, unsigned short acclaim, int p, float value, float minVal, float maxVal, float step, unit valueUnit, String desc) {
    String result;
    
    char tempStr[16];
    //snprintf(tempStr, 16, "%.1f", value);
    dtostrf(value, 2, 1, tempStr); //2 is mininum width, 1 is precision
    
    if (p & permission_read) {
        result += HAPHelper::wrap("value")+":"+tempStr;
        result += ",";
    }
    
    snprintf(tempStr, 16, "%f", minVal);
    if (minVal != INT32_MIN)
        result += HAPHelper::wrap("minValue")+":"+tempStr+",";
    
    snprintf(tempStr, 16, "%f", maxVal);
    if (maxVal != INT32_MAX)
        result += HAPHelper::wrap("maxValue")+":"+tempStr+",";
    
    snprintf(tempStr, 16, "%f", step);
    if (step > 0)
        result += HAPHelper::wrap("minStep")+":"+tempStr+",";
    
    result += HAPHelper::wrap("perms")+":";
    result += "[";
    if (p & permission_read) result += HAPHelper::wrap("pr")+",";
    if (p & permission_write) result += HAPHelper::wrap("pw")+",";
    if (p & permission_notify) result += HAPHelper::wrap("ev")+",";
    if (p & permission_hidden) result += HAPHelper::wrap("hd")+",";
    result = result.substring(0, result.length()-1);
    result += "]";
    result += ",";
    
// #if HAP_LONG_UUID    
//     char uuidStr[HAP_UUID_LENGTH];
//     snprintf(uuidStr, HAP_UUID_LENGTH, HAP_UUID, type);
// #else    
    char uuidStr[8];
    snprintf(uuidStr, 8, "%X", type);    
// #endif

    result += HAPHelper::wrap("type") + ":" + HAPHelper::wrap(uuidStr);
    result += ",";
    
    snprintf(tempStr, 4, "%hd", acclaim);
    result += HAPHelper::wrap("iid")+":"+tempStr;
    result += ",";
    
    result += HAPHelper::wrap("unit")+":"+HAPHelper::wrap(characteristics::unitJson(valueUnit))+",";
    

    if (desc != "") {
        result += "\"description\":" + HAPHelper::wrap(desc.c_str());
        result += ",";        
    }

    result += "\"format\":\"float\"";
    
    return "{"+result+"}";
}



// 
// float char describe
// 
inline String attribute(String typeString, unsigned short acclaim, int p, float value, float minVal, float maxVal, float step, unit valueUnit, String desc) {
    String result;
    
    char tempStr[16];
    //snprintf(tempStr, 16, "%.1f", value);
    dtostrf(value, 2, 1, tempStr); //2 is mininum width, 1 is precision
    
    if (p & permission_read) {
        result += HAPHelper::wrap("value")+":"+tempStr;
        result += ",";
    }
    
    snprintf(tempStr, 16, "%f", minVal);
    if (minVal != INT32_MIN)
        result += HAPHelper::wrap("minValue")+":"+tempStr+",";
    
    snprintf(tempStr, 16, "%f", maxVal);
    if (maxVal != INT32_MAX)
        result += HAPHelper::wrap("maxValue")+":"+tempStr+",";
    
    snprintf(tempStr, 16, "%f", step);
    if (step > 0)
        result += HAPHelper::wrap("minStep")+":"+tempStr+",";
    
    result += HAPHelper::wrap("perms")+":";
    result += "[";
    if (p & permission_read) result += HAPHelper::wrap("pr")+",";
    if (p & permission_write) result += HAPHelper::wrap("pw")+",";
    if (p & permission_notify) result += HAPHelper::wrap("ev")+",";
    if (p & permission_hidden) result += HAPHelper::wrap("hd")+",";
    result = result.substring(0, result.length()-1);
    result += "]";
    result += ",";
    

    result += HAPHelper::wrap("type") + ":" + HAPHelper::wrap(typeString);
    result += ",";
    
    snprintf(tempStr, 4, "%hd", acclaim);
    result += HAPHelper::wrap("iid")+":"+tempStr;
    result += ",";
    
    result += HAPHelper::wrap("unit")+":"+HAPHelper::wrap(characteristics::unitJson(valueUnit))+",";
    
    if (desc != "") {
        result += "\"description\":" + HAPHelper::wrap(desc.c_str());
        result += ",";
    }
    
    result += "\"format\":\"float\"";
    
    return "{"+result+"}";
}



// 
// string / data char describe
// 
inline String attribute(unsigned short type, unsigned short acclaim, int p, String value, unsigned short len, String desc, bool isData) {
    String result;
    char tempStr[4];
    
    if (p & permission_read) {
        if (isData && value == (char*)NULL ){
            result += HAPHelper::wrap("value") + ":" + "null";
        } else {
            result += HAPHelper::wrap("value")+":"+HAPHelper::wrap(value.c_str());
        }        
        result += ",";
    }
    
    result += HAPHelper::wrap("perms")+":";
    result += "[";
    if (p & permission_read) result += HAPHelper::wrap("pr")+",";
    if (p & permission_write) result += HAPHelper::wrap("pw")+",";
    if (p & permission_notify) result += HAPHelper::wrap("ev")+",";
    if (p & permission_hidden) result += HAPHelper::wrap("hd")+",";
    result = result.substring(0, result.length()-1);
    result += "]";
    result += ",";
    

// #if HAP_LONG_UUID    
//     char uuidStr[HAP_UUID_LENGTH];
//     snprintf(uuidStr, HAP_UUID_LENGTH, HAP_UUID, type);
// #else    
    char uuidStr[8];
    snprintf(uuidStr, 8, "%X", type);    
// #endif

    result += HAPHelper::wrap("type") + ":" + HAPHelper::wrap(uuidStr);
    result += ",";
    
    snprintf(tempStr, 4, "%hd", acclaim);
    result += HAPHelper::wrap("iid")+":"+tempStr;
    result += ",";
    
    if (len > 0) {
        snprintf(tempStr, 4, "%hd", len);
        result += HAPHelper::wrap("maxLen")+":"+tempStr;
        result += ",";
    }

    if (desc != "") {
        result += "\"description\":" + HAPHelper::wrap(desc.c_str());
        result += ",";        
    }
    
    if (isData) {
        result += "\"format\":\"data\"";
    } else {
        result += "\"format\":\"string\"";
    }
    
    return "{"+result+"}";
}

// 
// string / data char describe
// 
inline String attribute(String typeString, unsigned short acclaim, int p, String value, unsigned short len, String desc, bool isData) {
    String result;
    char tempStr[4];
    
    if (p & permission_read) {
        if (isData && value == (char*)NULL ){
            result += HAPHelper::wrap("value") + ":" + "null";
        } else {
            result += HAPHelper::wrap("value")+":"+HAPHelper::wrap(value.c_str());
        }        
        result += ",";
    }
    
    result += HAPHelper::wrap("perms")+":";
    result += "[";
    if (p & permission_read) result += HAPHelper::wrap("pr")+",";
    if (p & permission_write) result += HAPHelper::wrap("pw")+",";
    if (p & permission_notify) result += HAPHelper::wrap("ev")+",";
    if (p & permission_hidden) result += HAPHelper::wrap("hd")+",";
    result = result.substring(0, result.length()-1);
    result += "]";
    result += ",";

    result += HAPHelper::wrap("type") + ":" + HAPHelper::wrap(typeString);
    result += ",";
    
    snprintf(tempStr, 4, "%hd", acclaim);
    result += HAPHelper::wrap("iid")+":"+tempStr;
    result += ",";
    
    if (len > 0) {
        snprintf(tempStr, 4, "%hd", len);
        result += HAPHelper::wrap("maxLen")+":"+tempStr;
        result += ",";
    }
    
    if (desc != "") {
        result += "\"description\":" + HAPHelper::wrap(desc.c_str());
        result += ",";
    }

    if (isData) {
        result += "\"format\":\"data\"";
    } else {
        result += "\"format\":\"string\"";
    }
    
    
    return "{"+result+"}";
}


// 
// uint8_t char describe
// 
inline String attribute(unsigned short type, unsigned short acclaim, int p, uint8_t value, uint8_t minVal, uint8_t maxVal, uint8_t step, unit valueUnit, String desc, uint8_t validValuesSize, uint8_t validValues[]) {
    String result;
    char tempStr[16];
    
    snprintf(tempStr, 16, "%d", value);
    
    if (p & permission_read) {
        result += HAPHelper::wrap("value")+":"+tempStr;
        result += ",";
    }
    
    snprintf(tempStr, 16, "%d", minVal);    
    result += HAPHelper::wrap("minValue")+":"+tempStr+",";
    
    snprintf(tempStr, 16, "%d", maxVal);    
    result += HAPHelper::wrap("maxValue")+":"+tempStr+",";
    
    snprintf(tempStr, 16, "%d", step);
    if (step > 0)
        result += HAPHelper::wrap("minStep")+":"+tempStr+",";
    
    result += HAPHelper::wrap("perms")+":";
    result += "[";
    if (p & permission_read) result += HAPHelper::wrap("pr")+",";
    if (p & permission_write) result += HAPHelper::wrap("pw")+",";
    if (p & permission_notify) result += HAPHelper::wrap("ev")+",";
    if (p & permission_hidden) result += HAPHelper::wrap("hd")+",";
    result = result.substring(0, result.length()-1);
    result += "]";
    result += ",";

    char uuidStr[8];
    snprintf(uuidStr, 8, "%X", type);    
    result += HAPHelper::wrap("type") + ":" + HAPHelper::wrap(uuidStr);
    result += ",";
    
    snprintf(tempStr, 4, "%hd", acclaim);
    result += HAPHelper::wrap("iid")+":"+tempStr;
    result += ",";
    
    result += HAPHelper::wrap("unit")+":"+HAPHelper::wrap(characteristics::unitJson(valueUnit))+",";
    
    if (desc != "") {
        result += "\"description\":" + HAPHelper::wrap(desc.c_str());
        result += ",";        
    }

    if (validValuesSize > 0) {
        result += "\"valid-values\": [";
        for (uint8_t i = 0; i < validValuesSize; i++){
            result += String(validValues[i]);

            if ( i < validValuesSize - 1) {
                result += ",";
            }
        }
        result += "],";  
    }

    result += "\"format\":\"uint8\"";
    
    return "{"+result+"}";
}


// 
// uint8_t char describe
// 
inline String attribute(String typeString, unsigned short acclaim, int p, uint8_t value, uint8_t minVal, uint8_t maxVal, uint8_t step, unit valueUnit, String desc, uint8_t validValuesSize, uint8_t validValues[]) {
    String result;
    char tempStr[16];
    
    snprintf(tempStr, 16, "%d", value);
    
    if (p & permission_read) {
        result += HAPHelper::wrap("value")+":"+tempStr;
        result += ",";
    }
    
    snprintf(tempStr, 16, "%d", minVal);    
    result += HAPHelper::wrap("minValue")+":"+tempStr+",";
    
    snprintf(tempStr, 16, "%d", maxVal);    
    result += HAPHelper::wrap("maxValue")+":"+tempStr+",";
    
    snprintf(tempStr, 16, "%d", step);
    if (step > 0)
        result += HAPHelper::wrap("minStep")+":"+tempStr+",";
    
    result += HAPHelper::wrap("perms")+":";
    result += "[";
    if (p & permission_read) result += HAPHelper::wrap("pr")+",";
    if (p & permission_write) result += HAPHelper::wrap("pw")+",";
    if (p & permission_notify) result += HAPHelper::wrap("ev")+",";
    if (p & permission_hidden) result += HAPHelper::wrap("hd")+",";
    result = result.substring(0, result.length()-1);
    result += "]";
    result += ",";


    result += HAPHelper::wrap("type") + ":" + HAPHelper::wrap(typeString);
    result += ",";
    
    snprintf(tempStr, 4, "%hd", acclaim);
    result += HAPHelper::wrap("iid")+":"+tempStr;
    result += ",";
    
    result += HAPHelper::wrap("unit")+":"+HAPHelper::wrap(characteristics::unitJson(valueUnit))+",";
    
    if (desc != "") {
        result += "\"description\":" + HAPHelper::wrap(desc.c_str());
        result += ",";        
    }

    if (validValuesSize > 0) {
        result += "\"valid-values\": [";
        for (uint8_t i = 0; i < validValuesSize; i++){
            result += String(validValues[i]);
            
            if ( i < validValuesSize - 1) {
                result += ",";
            }
        }

        result += "],";  
    }

    result += "\"format\":\"uint8\"";
    
    return "{"+result+"}";
}


String boolCharacteristics::describe() {

    if (type == CHAR_TYPE_NULL) {        
        return attribute(typeString, iid, permission, _value, desc);
    } else {
        return attribute(type, iid, permission, _value, desc);
    }    
}

String floatCharacteristics::describe() {
    if (type == CHAR_TYPE_NULL) {        
        return attribute(typeString, iid, permission, _value, _minVal, _maxVal, _step, _unit, desc);        
    } else {
        return attribute(type, iid, permission, _value, _minVal, _maxVal, _step, _unit, desc);
    }
}

String intCharacteristics::describe() {
    if (type == CHAR_TYPE_NULL) {  
        return attribute(typeString, iid, permission, _value, _minVal, _maxVal, _step, _unit, desc);
    } else {
        return attribute(type, iid, permission, _value, _minVal, _maxVal, _step, _unit, desc);
        
    }
}

String stringCharacteristics::describe() {
    if (type == CHAR_TYPE_NULL) {  
        return attribute(typeString, iid, permission, _value, maxLen, desc, false);        
    } else {        
        return attribute(type, iid, permission, _value, maxLen, desc, false);
    }
}

String dataCharacteristics::describe() {
    if (type == CHAR_TYPE_NULL) {  
        return attribute(typeString, iid, permission, _value, maxLen, desc, true);
    } else {        
        return attribute(type, iid, permission, _value, maxLen, desc, true);
    }
}

String uint16Characteristics::describe() {
    if (type == CHAR_TYPE_NULL) {  
        return attribute(typeString, iid, permission, _value, _minVal, _maxVal, _step, _unit, desc);
    } else {
        return attribute(type, iid, permission, _value, _minVal, _maxVal, _step, _unit, desc);        
    }
}

String uint8Characteristics::describe() {
    if (type == CHAR_TYPE_NULL) {  
        return attribute(typeString, iid, permission, _value, _minVal, _maxVal, _step, _unit, desc, _validValuesSize, _validValues);
    } else {
        return attribute(type, iid, permission, _value, _minVal, _maxVal, _step, _unit, desc, _validValuesSize, _validValues);        
    }
}

