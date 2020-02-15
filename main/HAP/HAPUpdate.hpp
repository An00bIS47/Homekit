//
// HAPUpdate.hpp
// Homekit
//
//  Created on: 20.08.2017
//      Author: michael
//

#ifndef HAPUPDATE_HPP_
#define HAPUPDATE_HPP_

#include <Arduino.h>
#include "HAPVersion.hpp"

#if HAP_UPDATE_SSL_ENABLED
#include <WiFiClientSecure.h>
#else
#include <WiFiClient.h>
#endif

#include "HAPGlobals.hpp"



class HAPUpdate {
public:
	HAPUpdate();
	~HAPUpdate();

	void begin(const char* local_hostname);
	void handle();

	void setHostAndPort(const char* url, int port, uint32_t interval = 1000 * 60 * 120) {
		_port = port;
		_host = url;
		_interval = interval;
	}

	void setInterval(uint16_t interval){
		_interval = interval;
	}
	
	bool updateAvailable();
	bool checkUpdateAvailable(HAPVersion* localVersion);
	void execWebupdate();

private:
	bool 			_isOTADone;
	uint16_t 		_contentLength;
	bool 			_isValidContentType;
	String 			_host;
	uint16_t 		_port;
	uint32_t 		_interval;
	unsigned long 	_previousMillis;
	HAPVersion* 	_localVersion;
	bool 			_available;

	char 			_expectedMd5[32];

#if HAP_UPDATE_ENABLE_SSL
	WiFiClientSecure _wifiClient;
#else
	WiFiClient _wifiClient;
#endif
	HAPVersion _remoteVersion;

	String getHeaderValue(String header, String headerName);
};

#endif /* HAPUPDATE_HPP_ */
