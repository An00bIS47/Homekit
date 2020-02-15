//
// HAPService.cpp
// Homekit
//
//  Created on: 22.04.2018
//      Author: michael
//

#include "HAPService.hpp"
#include "HAPHelper.hpp"

HAPService::HAPService(int _uuid)
: uuid(_uuid), uuidString("") {

}

HAPService::HAPService(String _uuid)
: uuid(CHAR_TYPE_NULL), uuidString(_uuid) {

}


String HAPService::describe() {
	
    String keys[3] = {"iid", "type", "characteristics"};
    String values[3];
    {
        char temp[8];
        snprintf(temp, 8, "%d", serviceID);
        values[0] = temp;
    }
    {

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


    {
        int no = numberOfCharacteristics();
        String *chars = new String[no];
        for (int i = 0; i < no; i++) {
            chars[i] = _characteristics[i]->describe();
        }
        values[2] = HAPHelper::arrayWrap(chars, no);
        delete [] chars;
    }
    return HAPHelper::dictionaryWrap(keys, values, 3);
    
}