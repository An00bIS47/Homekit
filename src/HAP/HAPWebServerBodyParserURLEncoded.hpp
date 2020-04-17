//
// HAPWebServerBodyParserURLEncoded.hpp
// Homekit
//
//  Created on: 14.12.2018
//      Author: michael
//

#ifndef HAPWEBSERVERBODYPARSERURLENCODED_HPP_
#define HAPWEBSERVERBODYPARSERURLENCODED_HPP_

#include <Arduino.h>
#include <HTTPSServer.hpp>
#include <vector>
#include <algorithm>

// Easier access to the classes of the server
using namespace httpsserver;

class HAPWebServerBodyParserURLEncoded {

public:    
    static std::vector<std::pair<std::string, std::string>> processAndParse(HTTPRequest* req);
private:

};

#endif /* HAPWEBSERVERBODYPARSERURLENCODED_HPP_ */