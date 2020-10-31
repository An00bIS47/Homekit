//
// HAPWiFiHelper.hpp
// Homekit
//
//  Created on: 08.08.2017
//      Author: michael
//

#ifndef HAPWIFIHELPER_HPP_
#define HAPWIFIHELPER_HPP_

#include <esp_wps.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <DNSServer.h>
#include <functional>
#include <vector>

#include "HTTPServer.hpp"
#include "HAPGlobals.hpp"
#include "HAPConfig.hpp"

#if HAP_PIXEL_INDICATOR_ENABLED
#include "HAPIndicatorPixelColors.hpp"
#endif

using namespace httpsserver;

enum HAP_WIFI_MODE {
	HAP_WIFI_MODE_AP 			= 0x00,		// 0
	HAP_WIFI_MODE_MULTI			= 0x01,		// 1
	HAP_WIFI_MODE_WPS			= 0x02,		// 2
	HAP_WIFI_MODE_SMARTCONFIG	= 0x03,		// 3

	HAP_WIFI_MODE_BLE_PROV		= 0x04,		// 4
	HAP_WIFI_MODE_AP_PROV		= 0x05,		// 5
};

class HAPWiFiHelper {
public:
	
	HAPWiFiHelper();
	~HAPWiFiHelper();

	static void begin(HAPConfig* config, std::function<bool(bool)> callbackBegin, const char* hostname);	
	static void connect(enum HAP_WIFI_MODE mode);	

	static bool captiveInitialized();
	static void handle();

	inline static bool isProvisioned(){
		return _isProvisioned;
	}

#if HAP_PIXEL_INDICATOR_ENABLED
	// static uint32_t getColorForMode(enum HAP_WIFI_MODE mode);
	// static RgbColor getColorForMode(enum HAP_WIFI_MODE mode);
	static CRGB getColorForMode(enum HAP_WIFI_MODE mode);
#endif

	static HAP_WIFI_MODE getNextMode();
	static HAP_WIFI_MODE getNextMode(enum HAP_WIFI_MODE mode);
	static HAP_WIFI_MODE getCurrentMode();

private:
	
#if HAP_PROVISIONING_ENABLE_BLE == 0	
	static void eventHandler(WiFiEvent_t event);
#else
	static void eventHandlerBLEProv(system_event_t *sys_event, wifi_prov_event_t *prov_event);
#endif

	static void startWPS();
	static void startCaptivePortal();
	static void stopCaptivePortal();
	static void connectMulti();

	static void handleRootGet(HTTPRequest * req, HTTPResponse * res);
	static void handleRootPost(HTTPRequest * req, HTTPResponse * res);
	static void rootKeyProcessor(const String& key, HTTPResponse * res);

	static esp_wps_config_t _wpsConfig;

	static WiFiMulti _wifiMulti;
	static enum HAP_WIFI_MODE _mode;
	static HAPConfig* _config;

	static uint8_t _errorCount;
	static DNSServer* _dnsServer;
	static HTTPServer* _webserver;

	static bool _captiveInitialized;
	static bool _isProvisioned;

	static std::function<bool(bool)> _callbackBegin;	
};

#endif /* HAPWIFIHELPER_HPP_ */
