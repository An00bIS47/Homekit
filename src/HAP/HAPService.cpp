//
// HAPService.cpp
// Homekit
//
//  Created on: 22.04.2018
//      Author: michael
//

#include "HAPService.hpp"
#include "HAPHelper.hpp"

HAPService::HAPService(uint8_t _uuid)
: uuid(_uuid), uuidString("") {
    hidden = false;
    primary = false;
}

HAPService::HAPService(String _uuid)
: uuid(CHAR_TYPE_NULL), uuidString(_uuid) {
    hidden = false;
    primary = false;
}


String HAPService::describe() {
	
    // String keys[6] = {"iid", "type", "characteristics", "hidden", "primary", "linked"};
    uint8_t keySize = 5;
    
    if (_linkedServiceIds.size() > 0){        
        keySize = 6;
    }
    
    String keys[keySize];
    String values[keySize];
    
    // key: iid
    {
        keys[0] = "iid";
        char temp[8];
        snprintf(temp, 8, "%d", serviceID);
        values[0] = temp;
    }

    // key: type
    {
        keys[1] = "type";
        if (uuid == 0x00) {            
            String uuidStr = uuidString;
            values[1] = HAPHelper::wrap(uuidStr);
        } else {
#if HAP_LONG_UUID    
            char uuidStr[HAP_UUID_LENGTH];
            snprintf(uuidStr, HAP_UUID_LENGTH, HAP_UUID, uuid);        
#else    
            char uuidStr[8];
            snprintf(uuidStr, 8, "%X", uuid);    
#endif  
            values[1] = HAPHelper::wrap(uuidStr);
        }                              
    }

    // key: characteristics
    {
        keys[2] = "characteristics";

        int no = numberOfCharacteristics();
        String *chars = new String[no];
        for (int i = 0; i < no; i++) {
            chars[i] = _characteristics[i]->describe();
        }
        values[2] = HAPHelper::arrayWrap(chars, no);
        delete [] chars;
    }

    // key: hidden
    {      
        keys[3] = "hidden";  
        values[3] = hidden ? HAPHelper::wrap("true") : HAPHelper::wrap("false");
    }
    // key: primary
    {        
        keys[4] = "primary";  
        values[4] = primary ? HAPHelper::wrap("true") : HAPHelper::wrap("false");
    }

    // key: linked serviceIds
    if (_linkedServiceIds.size() > 0){

        keys[5] = "linked";  

        int no = _linkedServiceIds.size();
        String *chars = new String[no];
        for (int i = 0; i < no; i++) {
            chars[i] = String(_linkedServiceIds[i]);
        }
        values[5] = HAPHelper::arrayWrap(chars, no);
        delete [] chars;

        // String result = "[";
        // for (uint8_t i = 0; i < _linkedServiceIds.size(); i++){
        //     result += String(_linkedServiceIds[i]);

        //     if (i < (_linkedServiceIds.size() - 1)){
        //         result += ",";
        //     }
        // }
        // String result += "]";

        // char tmp[result.length()];
        // result.toCharArry(tmp);
        // values[5] = tmp;
    }

    return HAPHelper::dictionaryWrap(keys, values, keySize);
    
}