//
// HAPRequest.hpp
// Homekit
//
//  Created on: 12.08.2017
//      Author: michael
//

#ifndef HAPREQUEST_HPP_
#define HAPREQUEST_HPP_

#include <Arduino.h>
#include <map>

#include "HAPTLV8.hpp"

enum HAPServerMethod {
	METHOD_UNKNOWN = 0,
	METHOD_POST,
	METHOD_GET,
	METHOD_PUT,
	METHOD_DELETE,
};

class HAPRequest {
public:
	HAPRequest();
	~HAPRequest();


	HAPServerMethod 			method;
	String 						path;
	String 						contentType;
	uint16_t 					contentLength;
	TLV8						tlv;
	std::map<String, String> 	params;

	void clear();

	String toString() const;

private:

};

#endif /* HAPREQUEST_HPP_ */
