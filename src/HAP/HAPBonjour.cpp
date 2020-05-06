//
// HAPBonjour.cpp
// Homekit
//
//  Created on: 24.06.2017
//      Author: michael
//

#include "HAPBonjour.hpp"


bool MDNSResponderExt::addServiceTxtSet(char *name, const char * proto, uint8_t num_items, mdns_txt_item_t *txt){
    
    if(mdns_service_txt_set(name, proto, txt, num_items)) {
        log_e("Failed setting service TXT set");
        return false;
    }
    return true;
}

bool MDNSResponderExt::removeServiceTxt(char *name, const char * proto){
    if(mdns_service_remove(name, proto)) {
        log_w("Failed removing TXT service");
        return false;
    }
    return true;
}



MDNSResponderExt mDNSExt;
