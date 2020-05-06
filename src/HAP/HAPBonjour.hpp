//
// HAPBonjour.hpp
// Homekit
//
//  Created on: 24.06.2017
//      Author: michael
//

#ifndef HAPBONJOUR_HPP_
#define HAPBONJOUR_HPP_

#include "Arduino.h"
#include <ESPmDNS.h>
#include "mdns.h"

class MDNSResponderExt : public MDNSResponder {
public:

    // Added for Homekit
    bool removeServiceTxt(char *name, const char * proto);
    bool addServiceTxtSet(char *name, const char * proto, uint8_t num_items, mdns_txt_item_t *txt);
    
private:

};

extern MDNSResponderExt mDNSExt;

#endif /* HAPBONJOUR_HPP_ */
