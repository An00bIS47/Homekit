//
// HAPWebServerBodyParserURLEncoded.cpp
// Homekit
//
//  Created on: 14.12.2018
//      Author: michael
//

#include "HAPWebServerBodyParserURLEncoded.hpp"

/** HTTP Query parameters, as key-value pairs */
  

std::vector<std::pair<std::string, std::string>> HAPWebServerBodyParserURLEncoded::processAndParse(HTTPRequest* req){
	uint8_t buffer[256];
	std::vector<std::pair<std::string, std::string>> keyValues;
	
	std::string key = "";
	std::string value = "";
	bool keyFound = false;

	do {
		size_t readBytes = req->readBytes(buffer, 256);				
		for (int i = 0; i < readBytes; i++){
			if ( buffer[i] == '=' ) {
				keyFound = true;
			} else if ( buffer[i] == '&') {

				// Serial.print(key.c_str());
				// Serial.print(" = ");
				// Serial.println(value.c_str());
				
				std::pair<std::string, std::string> param;
				param.first = urlDecode(key);
				if (value.length() > 0) {
					param.second = urlDecode(value);
				} else {
					param.second = "";
				}
					
				keyValues.push_back(param);				

				key = "";
				value = "";
				keyFound = false;
			} else {
				if (keyFound == false) {
					key += buffer[i];
				} else {
					value +=buffer[i];
				}
			}
		}


    } while(!(req->requestComplete()));

	if (keyFound) {
		std::pair<std::string, std::string> param;
		param.first = urlDecode(key);
		if (value.length() > 0) {
			param.second = urlDecode(value);
		} else {
			param.second = "";
		}

		keyValues.push_back(param);	

		// Serial.print(key.c_str());
		// Serial.print(" = ");
		// Serial.println(value.c_str());
	}

	return keyValues;
}