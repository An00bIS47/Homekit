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
#include <HTTPClient.h>

#include "HAPVersion.hpp"
#include "HAPGlobals.hpp"
#include "HAPConfig.hpp"


struct HAPUpdateVersionInfo {
	HAPVersion version;
	String md5;
	uint32_t size;
	uint32_t featureRev;
};


class HAPUpdate {
public:
	HAPUpdate();
	~HAPUpdate();

	void begin(HAPConfig* config);
	void handle();

	void setHostAndPort(const char* url, int port, uint32_t interval = HAP_UPDATE_WEB_INTERVAL) {
		_port = port;
		_host = url;
		_interval = interval;
	}

	void setInterval(uint16_t interval){
		_interval = interval;
	}
	
	bool updateAvailable();

#if HAP_UPDATE_ENABLE_FROM_WEB
	bool checkUpdateAvailable();
	void execWebupdate();
#endif

	String onlineVersion() {
		return _remoteInfo.version.toString();
	}

private:
	bool 			_isOTADone;
	
	String 			_host;
	uint16_t 		_port;
	uint32_t 		_interval;
	unsigned long 	_previousMillis;	
	bool 			_available;

 	HAPUpdateVersionInfo _remoteInfo;
	
	HTTPClient* _http;
	// String getHeaderValue(String header, String headerName);
};

#endif /* HAPUPDATE_HPP_ */
