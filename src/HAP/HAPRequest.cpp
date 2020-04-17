//
// HAPRequest.cpp
// Homekit
//
//  Created on: 12.08.2017
//      Author: michael
//

#include "HAPRequest.hpp"
#include <WString.h>

HAPRequest::HAPRequest() {
	path = "";
	contentType = "";
	contentLength = 0;
	method = METHOD_UNKNOWN;
}

HAPRequest::~HAPRequest() {
	clear();
}

String HAPRequest::toString() const {
	String result = F("method: ");
	result += method;

	result += F("\npath: ");
	result += path;

	result += F("\ncontentType: ");
	result += contentType;

	result += F("\ncontentLength: ");
	result += contentLength;

	return result;
}

void HAPRequest::clear(){
	path = "";
	contentType = "";
	contentLength = 0;
	method = METHOD_UNKNOWN;

	tlv.clear();
	params.clear();
}
