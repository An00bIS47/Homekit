//
// HAPWebServerBodyParserMultipart.hpp
// Homekit
//
//  Created on: 14.12.2018
//      Author: michael
//

#ifndef HAPWEBSERVERBODYPARSERMULTIPART_HPP_
#define HAPWEBSERVERBODYPARSERMULTIPART_HPP_

#include <Arduino.h>
#include <HTTPSServer.hpp>


// Easier access to the classes of the server
using namespace httpsserver;

class HAPWebServerBodyParserMultipart {

public:    
    static void processAndParse(HTTPRequest* req, const std::function<void(const std::string&, const std::string&)> fieldValueCallback);
private:

};

#endif /* HAPWEBSERVERBODYPARSERMULTIPART_HPP_ */