//
// HAPWiFiHelper.cpp
// Homekit
//
//  Created on: 08.08.2017
//      Author: michael
//

#include "HAPWiFiHelper.hpp"

#include <WString.h>
#include <esp_wifi.h>
#include <ArduinoJson.h>

#include "HAPWebServer.hpp"

#if HAP_WEBSERVER_USE_SPIFFS    
#include <SPIFFS.h>
#else
#include "HAPWebServerFiles.hpp"
#endif

#include "HAPDeviceID.hpp"
#include "HAPLogger.hpp"
#include "HAPWebServerTemplateProcessor.hpp"
#include "HAPWebServerBodyParserURLEncoded.hpp"

enum HAP_WIFI_MODE HAPWiFiHelper::_mode;
WiFiMulti HAPWiFiHelper::_wifiMulti;
HAPConfig* HAPWiFiHelper::_config;
esp_wps_config_t HAPWiFiHelper::_wpsConfig;
uint8_t HAPWiFiHelper::_errorCount;
DNSServer* HAPWiFiHelper::_dnsServer;
HTTPServer* HAPWiFiHelper::_webserver;
std::function<bool(bool)> HAPWiFiHelper::_callbackBegin;

bool HAPWiFiHelper::_captiveInitialized;
bool HAPWiFiHelper::_isProvisioned;

#define ESP_WPS_MODE WPS_TYPE_PBC
//#define ESP_WPS_MODE WPS_TYPE_PIN

HAPWiFiHelper::HAPWiFiHelper(){
	_errorCount = 0;		
	_captiveInitialized = false;
	_isProvisioned = false;
	
	_webserver = nullptr;
	_dnsServer = nullptr;
}


HAPWiFiHelper::~HAPWiFiHelper() {
	// TODO Auto-generated destructor stub
}


void HAPWiFiHelper::begin(HAPConfig* config, std::function<bool(bool)> callbackBegin, const char* hostname){
	_config = config;
	_callbackBegin = callbackBegin;

	WiFi.persistent(false);
	WiFi.setHostname(hostname);
	
	enum HAP_WIFI_MODE selectedMode = (enum HAP_WIFI_MODE)_config->config()["wifi"]["mode"].as<uint8_t>();

#if HAP_PROVISIONING_ENABLE_BLE == 0
	WiFi.onEvent(eventHandler);
#endif

	if (selectedMode == HAP_WIFI_MODE_WPS) {
		//_wpsConfig.crypto_funcs = &g_wifi_default_wps_crypto_funcs;
		_wpsConfig.wps_type = WPS_TYPE_PBC;
		strcpy(_wpsConfig.factory_info.manufacturer, HAP_MANUFACTURER);
		// strcpy(_wpsConfig.factory_info.model_number, "1");
		strcpy(_wpsConfig.factory_info.model_name,   HAP_MODELL_NAME);
		strcpy(_wpsConfig.factory_info.device_name,  hostname);
	} 

#if HAP_PROVISIONING_ENABLE_BLE

	else if (selectedMode == HAP_WIFI_MODE_BLE_PROV) {
		//Sample uuid that user can pass during provisioning using BLE
		/* uint8_t uuid[16] = {0xb4, 0xdf, 0x5a, 0x1c, 0x3f, 0x6b, 0xf4, 0xbf,
					0xea, 0x4a, 0x82, 0x03, 0x04, 0x90, 0x1a, 0x02 };*/
		

		WiFi.onEvent(eventHandlerBLEProv);
		WiFi.beginProvision(provSchemeBLE, WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM, WIFI_PROV_SECURITY_1, HAP_PROVISIONING_POP, HAPDeviceID::provisioningID(HAP_PROVISIONING_PREFIX).c_str(), NULL, NULL);		
	}

	
#endif /* HAP_PROVISIONING_ENABLE_BLE==0 */

	connect(selectedMode);
}


void HAPWiFiHelper::startWPS(){

	WiFi.mode(WIFI_MODE_STA);
	
	// ESP_ERROR_CHECK(esp_wifi_wps_enable(WPS_TYPE_PBC));
	ESP_ERROR_CHECK(esp_wifi_wps_enable(&_wpsConfig));
	ESP_ERROR_CHECK(esp_wifi_wps_start(0));

	LogI("Connecting via WPS ..", false);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		LogI(".", false);
	}

	_isProvisioned = true;
	LogI( F("OK"), true);
}

void HAPWiFiHelper::startCaptivePortal(){

	LogI("Starting captive portal ...", false);
	
	//WiFi.disconnect();   //added to start with the wifi off, avoid crashing
	WiFi.mode(WIFI_OFF); //added to start with the wifi off, avoid crashing
	WiFi.mode(WIFI_AP);

	IPAddress apIP;
	apIP.fromString(HAP_CAPTIVE_AP_IP);

	WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
	WiFi.softAP(HAP_CAPTIVE_AP_SSID);

	_dnsServer = new DNSServer();
	_dnsServer->start(HAP_CAPTIVE_DNSSERVER_PORT , "*", apIP);
	
	// SPIFFS.begin();

	_webserver = new HTTPServer();

	ResourceNode* nodeRootGet = new ResourceNode("/", "GET", &handleRootGet);
	ResourceNode* nodeRootPost = new ResourceNode("/", "POST", &handleRootPost);
	// Add the root node to the servers. We can use the same ResourceNode on multiple
	// servers (you could also run multiple HTTPS servers)
	_webserver->registerNode(nodeRootGet);
	_webserver->registerNode(nodeRootPost);
  

	// We do the same for the default Node
	_webserver->setDefaultNode(nodeRootGet);

	LogD("Starting HTTP server ...", true);
	_webserver->start();
	if (_webserver->isRunning()) {
		LogD("OK", true);
	}
	
	_captiveInitialized = true;
	LogI(" OK", true);
}


void HAPWiFiHelper::handleRootGet(HTTPRequest *req, HTTPResponse *res)
{
	req->discardRequestBody();
    // template processing	
#if HAP_WEBSERVER_USE_SPIFFS        	
	HAPWebServerTemplateProcessor::processAndSend(res, "/index.html", &rootKeyProcessor);
#else		
	HAPWebServerTemplateProcessor::processAndSendEmbedded(res, html_template_index_start, html_template_index_end, &rootKeyProcessor);
#endif
}

void HAPWiFiHelper::handleRootPost(HTTPRequest *req, HTTPResponse *res)
{

	HAPWebServerBodyParserURLEncoded urlParser;
	std::vector<std::pair<std::string, std::string>> parameters = urlParser.processAndParse(req);

	String ssid = "";
	String password = "";

	
	for (auto param : parameters) {
		
		if (param.first == "ssid") {
			ssid = param.second.c_str();
		} else // if (param.first == "password") 
		{
			password = param.second.c_str();
		}
	}
	
	if (ssid != "") {
		_config->addNetwork(ssid, password);
		_config->config()["wifi"]["mode"] = (uint8_t)HAP_WIFI_MODE_MULTI;
		_config->save();

		res->setStatusCode(200);
		res->setStatusText("OK");

		delay(1000);
		stopCaptivePortal();
		
		connect(HAP_WIFI_MODE_MULTI);
	} else {
		// template processing	
#if HAP_WEBSERVER_USE_SPIFFS        	
    	HAPWebServerTemplateProcessor::processAndSend(res, "/index.html", &rootKeyProcessor);
#else		
		HAPWebServerTemplateProcessor::processAndSendEmbedded(res, html_template_index_start, html_template_index_end, &rootKeyProcessor);
#endif		
	}	
}

void HAPWiFiHelper::rootKeyProcessor(const String& key, HTTPResponse * res){
#if HAP_WEBSERVER_TEMPLATE_PROCESSING_CHUNKED							        
    if (key == "VAR_TITLE") {
						        
		HAPWebServerTemplateProcessor::sendChunk(res, HAP_CAPTIVE_TITLE);
    } else if (key == "VAR_NAVIGATION") {

      	HAPWebServerTemplateProcessor::sendChunk(res, HAPWebServer::buildNavigation(false));  
    } else if (key == "VAR_CONTENT") {

		int n = WiFi.scanNetworks();

		if (n == 0) {
			

			HAPWebServerTemplateProcessor::sendChunk(res, "<p>No networks found</p>");
		} else {


			String result = "";
			result += "<div class=\"pure-u-1 pure-u-md-1\"><p>Found Networks:</p>";
			result += "<table class=\"pure-table pure-table-horizontal\" id=\"tblNetworks\">";
			result += "<thead> <tr> <th>SSID</th> <th>RSSI</th> <th>Encrypted</th> </tr> </thead>";
			result += "<tbody>";
			for (int i = 0; i < n; ++i) {
				// res->printf("<tr><td>%s</td><td>%d</td><td>%s</td></tr>\n", WiFi.SSID(i).c_str(), WiFi.RSSI(i), (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
				result += "<tr>";				
				result += "<td>" + WiFi.SSID(i) + "</td>";
				result += "<td>" + String(WiFi.RSSI(i)) + "</td>";
				result += "<td>" + (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? String(" ") : String("*") + "</td>";								
				result += "</tr>";
			}
			
			result += "</tbody>";
			result += "</table>";
			result += "</div>";

			result += "<br>";

			result += "<form action='/' method='post' enctype='application/x-www-form-urlencoded'>";
			result += "<input type='text' name='ssid'></input>";
			result += "<input type='password' name='password'></input>";
			result += "<input type='submit' value='Save'>";
			result += "</form>";

			HAPWebServerTemplateProcessor::sendChunk(res, result);
		}
    } else {
		res->print("");
	}
#else
   if (key == "VAR_TITLE") {
		res->print(HAP_CAPTIVE_TITLE);
    } else if (key == "VAR_NAVIGATION") {
		res->print(HAPWebServer::buildNavigation(false));	
    } else if (key == "VAR_CONTENT") {

		int n = WiFi.scanNetworks();

		if (n == 0) {
			res->print("<p>No networks found</p>");
		} else {
			res->print("<div class=\"pure-u-1 pure-u-md-1\"><p>Found Networks:</p>");
			res->print("<table class=\"pure-table pure-table-horizontal\" id=\"tblNetworks\">");
			res->print("<thead> <tr> <th>SSID</th> <th>RSSI</th> <th>Encrypted</th> </tr> </thead>");
			res->print("<tbody>");
			for (int i = 0; i < n; ++i) {
				res->printf("<tr><td>%s</td><td>%d</td><td>%s</td></tr>", WiFi.SSID(i).c_str(), WiFi.RSSI(i), (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
			}
			res->print("</tbody>");
			res->print("</table>");
			res->print("</div>");

			res->print("<br>");

			res->print("<form action='/' method='post' enctype='application/x-www-form-urlencoded'>");
			res->print("<input type='text' name='ssid'></input>");
			res->print("<input type='password' name='password'></input>");
			res->print("<input type='submit' value='Save'>");
			res->print("</form>");

			
		}
    } else {
		res->print("");
	}
#endif
}


void HAPWiFiHelper::stopCaptivePortal() {
	LogI("Stopping captive portal ...", false);
	// Stop DNSServer
	_dnsServer->stop();
	delete _dnsServer;

	_webserver->stop();
	while( _webserver->isRunning()) {
		LogI(".", false);
		delay(100);
	}

	delete _webserver;
	
	// SPIFFS.end();
	LogI(" OK", true);

	_captiveInitialized = false;
	_callbackBegin(true);
}

void HAPWiFiHelper::connectMulti(){
	LogI("Connecting WiFi...", false);
	if(_wifiMulti.run() == WL_CONNECTED) {
		LogI(" OK", true);
		_isProvisioned = true;
	} else {
		_errorCount = _errorCount + 1;
		if (_errorCount > HAP_WIFI_CONNECTION_MAX_RETRIES) {
			LogW("WiFi connection failed! Setting WiFi mode back to default mode", true);
			_config->config()["wifi"]["mode"] = (uint8_t)HAP_WIFI_MODE_DEFAULT;
			_config->save();
			_errorCount = 0;
			connect((enum HAP_WIFI_MODE)HAP_WIFI_MODE_DEFAULT);
		} else {
			LogW("WiFi connection failed! Retrying", true);
			delay(HAP_WIFI_CONNECTION_RETRY_DELAY);
			connectMulti();
		}
					
	}
}

void HAPWiFiHelper::connect(enum HAP_WIFI_MODE mode){
	
	switch (mode){

		case HAP_WIFI_MODE_AP:
			startCaptivePortal();
			break;

		case HAP_WIFI_MODE_MULTI:	
			
			// Add networks
			for( const auto& value : _config->config()["wifi"]["networks"].as<JsonArray>() ) { 					
				_wifiMulti.addAP(value["ssid"].as<const char*>(), value["password"].as<const char*>());
			}	
			connectMulti();

			break;
		case HAP_WIFI_MODE_WPS:	
			startWPS();
			break;

		case HAP_WIFI_MODE_SMARTCONFIG:
			//Init WiFi as Station, start SmartConfig
			WiFi.mode(WIFI_AP_STA);			
			WiFi.beginSmartConfig();

			//Wait for SmartConfig packet from mobile
			LogI("Waiting for SmartConfig..", false);
			while (!WiFi.smartConfigDone()) {
				delay(500);
				LogV( F("."), false);
			}

			LogI(" OK", true);
			LogI("SmartConfig received.", true);

			//Wait for WiFi to connect to AP
			LogI("Connecting to WiFi..", false);
			while (WiFi.status() != WL_CONNECTED) {
				delay(500);
				LogI(".", false);
			}
			LogI(" OK", true);
			_isProvisioned = true;
			break;

		default:
			break;
	}

	if (mode != HAP_WIFI_MODE_AP) {
		LogI("WiFi connected to ", false);
		LogI(WiFi.SSID(), true);
	}
}

bool HAPWiFiHelper::captiveInitialized(){
	return _captiveInitialized;
}

void HAPWiFiHelper::handle(){

	_dnsServer->processNextRequest();
	
	_webserver->loop();
	// _config->config()["wifi"]["mode"] = (uint8_t)HAPWiFiModeMulti;
	// stopCaptivePortal();

	delay(1);
}


#if HAP_PROVISIONING_ENABLE_BLE == 0
void HAPWiFiHelper::eventHandler(WiFiEvent_t event) {
	switch(event) {
		case SYSTEM_EVENT_STA_START:

			break;

		case SYSTEM_EVENT_STA_DISCONNECTED:
			LogW("WiFi lost connection.  Attempting to reconnect...", true);
			WiFi.reconnect();
			break;

		case SYSTEM_EVENT_STA_CONNECTED:
			//enable sta ipv6 here
            WiFi.enableIpV6();
			_isOnline = true;
			// LogV( F("OK"), true);
			break;

		case SYSTEM_EVENT_STA_GOT_IP:
			LogD( F("Got IP address "), false);
			LogD(WiFi.localIP().toString().c_str(), true);
			break;

		case SYSTEM_EVENT_AP_STA_GOT_IP6:
			// LogD( F("\nGot IPv6 address "), false);
			// LogD(WiFi.localIPv6().toString().c_str(), true);			
			break;	
			
		case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
			/*point: the function esp_wifi_wps_start() only get ssid & password
			 * so call the function esp_wifi_connect() here
			 * */
			LogI( F("WPS succeeded! Stopping WPS and connecting to "), false);
			LogI(WiFi.SSID(), true);
			LogD(" with password ", false);
			LogD(WiFi.psk().c_str(), false);
			LogI("", true);

			_config->addNetwork(WiFi.SSID(), WiFi.psk());
			_config->config()["wifi"]["mode"] = (uint8_t)HAP_WIFI_MODE_MULTI;			
			_config->save();
			
			ESP_ERROR_CHECK(esp_wifi_wps_disable());
			delay(500);			
			WiFi.begin();

			break;

		case SYSTEM_EVENT_STA_WPS_ER_FAILED:
			LogE( F("WPS failed! - Retrying"), true);			
            ESP_ERROR_CHECK(esp_wifi_wps_disable());
			ESP_ERROR_CHECK(esp_wifi_wps_enable(&_wpsConfig));
			ESP_ERROR_CHECK(esp_wifi_wps_start(0));  
			break;

		case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
			LogE( F("WPS timedout! - Please enable WPS on your router! Retrying"), true);			
            ESP_ERROR_CHECK(esp_wifi_wps_disable());
			ESP_ERROR_CHECK(esp_wifi_wps_enable(&_wpsConfig));
			ESP_ERROR_CHECK(esp_wifi_wps_start(0));            
			break;
//		case SYSTEM_EVENT_STA_WPS_ER_PIN:
//			Log(COLOR_GREEN, "WPS PIN Code", true);
////			ESP_LOGI(TAG, "SYSTEM_EVENT_STA_WPS_ER_PIN");
//			/*show the PIN code here*/
////			ESP_LOGI(TAG, "WPS_PIN = "PINSTR, PIN2STR(event->event_info.sta_er_pin.pin_code));
//			Log(COLOR_GREEN, "WPS_PIN", false);
//			char str[12];
//			sprintf(str, PINSTR, PIN2STR(event.event_info.sta_er_pin.pin_code));
//			Log(COLOR_GREEN, str, true);
//			break;
		default:
			break;
	}
}

#else 

void HAPWiFiHelper::eventHandlerBLEProv(system_event_t *sys_event, wifi_prov_event_t *prov_event)
{
    if (sys_event) {
		switch (sys_event->event_id) {
			case SYSTEM_EVENT_STA_GOT_IP:
				LogD( F("Got IP address "), false);
				LogD(ip4addr_ntoa(&sys_event->event_info.got_ip.ip_info.ip), true);		
				break;

			case SYSTEM_EVENT_STA_DISCONNECTED:
				LogW("WiFi lost connection.  Attempting to reconnect...", true);
				WiFi.reconnect();			
				break;
			case SYSTEM_EVENT_STA_CONNECTED:
				//enable sta ipv6 here
    	        WiFi.enableIpV6();			
				// LogV( F("OK"), true);
				break;

			case SYSTEM_EVENT_AP_STA_GOT_IP6:
				LogD( F("\nGot IPv6 address "), false);
				LogD(WiFi.localIPv6().toString().c_str(), true);			
				break;	
			
			case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
				/*point: the function esp_wifi_wps_start() only get ssid & password
				* so call the function esp_wifi_connect() here
				* */
				LogI( F("WPS succeeded! Stopping WPS and connecting to "), false);
				LogI(WiFi.SSID(), true);
				LogD(" with password ", false);
				LogD(WiFi.psk().c_str(), false);
				LogI("", true);

				_config->addNetwork(WiFi.SSID(), WiFi.psk());
				_config->config()["wifi"]["mode"] = (uint8_t)HAP_WIFI_MODE_MULTI;			
				_config->save();
				
				ESP_ERROR_CHECK(esp_wifi_wps_disable());
				delay(500);			
				WiFi.begin();

				break;

			case SYSTEM_EVENT_STA_WPS_ER_FAILED:
				LogE( F("WPS failed! - Retrying"), true);			
				ESP_ERROR_CHECK(esp_wifi_wps_disable());
				ESP_ERROR_CHECK(esp_wifi_wps_enable(&_wpsConfig));
				ESP_ERROR_CHECK(esp_wifi_wps_start(0));  
				break;

			case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
				LogE( F("WPS timedout! - Please enable WPS on your router! Retrying"), true);			
				ESP_ERROR_CHECK(esp_wifi_wps_disable());
				ESP_ERROR_CHECK(esp_wifi_wps_enable(&_wpsConfig));
				ESP_ERROR_CHECK(esp_wifi_wps_start(0));            
				break;
	// 		case SYSTEM_EVENT_STA_WPS_ER_PIN:
	// 			Log(COLOR_GREEN, "WPS PIN Code", true);
	// //			ESP_LOGI(TAG, "SYSTEM_EVENT_STA_WPS_ER_PIN");
	// 			/*show the PIN code here*/
	// //			ESP_LOGI(TAG, "WPS_PIN = "PINSTR, PIN2STR(event->event_info.sta_er_pin.pin_code));
	// 			Log(COLOR_GREEN, "WPS_PIN", false);
	// 			char str[12];
	// 			sprintf(str, PINSTR, PIN2STR(event.event_info.sta_er_pin.pin_code));
	// 			Log(COLOR_GREEN, str, true);
	// 			break;

			default:
				break;
		}      
    } 

	if (prov_event) {
        switch (prov_event->event) {
			case WIFI_PROV_START:				
				LogI("Provisioning started! Please enter the credentials of your access point using the iPhone app", true);
				break;

			case WIFI_PROV_CRED_RECV: 
				{
					LogD("Received Wi-Fi credentials: ", true);
					wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)prov_event->event_data;
					LogD("   - SSID : ", false);
					LogD((const char *) wifi_sta_cfg->ssid, true);
					LogD("   - Password : ", false);
					LogD((const char *) wifi_sta_cfg->password, true);

					_config->addNetwork((const char *) wifi_sta_cfg->ssid, (const char *)wifi_sta_cfg->password);
					_config->config()["wifi"]["mode"] = (uint8_t)HAP_WIFI_MODE_MULTI;			
					_config->save();

					break;
				}

			
			case WIFI_PROV_CRED_FAIL: 
				{
					wifi_prov_sta_fail_reason_t *reason = (wifi_prov_sta_fail_reason_t *)prov_event->event_data;
					LogE("\nProvisioning failed!\nPlease reset to factory and retry provisioning\n", true);
					if(*reason == WIFI_PROV_STA_AUTH_ERROR) 
						LogE("\nWi-Fi AP password incorrect", true);
					else
						LogE("\nWi-Fi AP not found....Add API \" nvs_flash_erase() \" before beginProvision()", true);        
					break;
				}

			
			case WIFI_PROV_CRED_SUCCESS:
				LogI("Provisioning was successful!", true);
				break;

			case WIFI_PROV_END:
				LogI("Provisioning ends!", true);
				_isProvisioned = true;
				break;

			default:
				break;
		}      
    }
}

#endif

#if HAP_PIXEL_INDICATOR_ENABLED
// uint32_t HAPWiFiHelper::getColorForMode(const HAP_WIFI_MODE mode){
// 	switch (mode) {
// 		case HAP_WIFI_MODE_AP:
// 			// return CRGB::Cyan;
// 			return HAPColorCyan;
// 		case HAP_WIFI_MODE_WPS:
// 			// return CRGB::Yellow;
// 			return HAPColorYellow;
// 		case HAP_WIFI_MODE_MULTI:
// 			// return CRGB::Magenta;
// 			return HAPColorMagenta;
// 		case HAP_WIFI_MODE_SMARTCONFIG:
// 			// return CRGB::Orange;
// 			return HAPColorOrange;
// 		case HAP_WIFI_MODE_BLE_PROV:
// 			// return CRGB::Blue;		
// 			return HAPColorBlue;
// 		case HAP_WIFI_MODE_AP_PROV:
// 			// return CRGB::Violet;
// 			return HAPColorPurple;

// 		default:
// 			// return CRGB::Black;
// 			return HAPColorBlack;
// 	}
// }

// RgbColor HAPWiFiHelper::getColorForMode(const HAP_WIFI_MODE mode){
// 	switch (mode) {
// 		case HAP_WIFI_MODE_AP:
// 			// return CRGB::Cyan;
// 			return HAPColorCyan;
// 		case HAP_WIFI_MODE_WPS:
// 			// return CRGB::Yellow;
// 			return HAPColorYellow;
// 		case HAP_WIFI_MODE_MULTI:
// 			// return CRGB::Magenta;
// 			return HAPColorMagenta;
// 		case HAP_WIFI_MODE_SMARTCONFIG:
// 			// return CRGB::Orange;
// 			return HAPColorOrange;
// 		case HAP_WIFI_MODE_BLE_PROV:
// 			// return CRGB::Blue;		
// 			return HAPColorBlue;
// 		case HAP_WIFI_MODE_AP_PROV:
// 			// return CRGB::Violet;
// 			return HAPColorPurple;

// 		default:
// 			// return CRGB::Black;
// 			return HAPColorBlack;
// 	}
// }

CRGB HAPWiFiHelper::getColorForMode(const HAP_WIFI_MODE mode){
	switch (mode) {
		case HAP_WIFI_MODE_AP:
			// return CRGB::Cyan;
			return HAPColorCyan;
		case HAP_WIFI_MODE_WPS:
			// return CRGB::Yellow;
			return HAPColorYellow;
		case HAP_WIFI_MODE_MULTI:
			// return CRGB::Magenta;
			return HAPColorMagenta;
		case HAP_WIFI_MODE_SMARTCONFIG:
			// return CRGB::Orange;
			return HAPColorOrange;
		case HAP_WIFI_MODE_BLE_PROV:
			// return CRGB::Blue;		
			return HAPColorBlue;
		case HAP_WIFI_MODE_AP_PROV:
			// return CRGB::Violet;
			return HAPColorPurple;

		default:
			// return CRGB::Black;
			return HAPColorBlack;
	}
}
#endif

HAP_WIFI_MODE HAPWiFiHelper::getCurrentMode(){
	return (enum HAP_WIFI_MODE)_config->config()["wifi"]["mode"].as<uint8_t>();
}

HAP_WIFI_MODE HAPWiFiHelper::getNextMode(){
	uint8_t curMode = _config->config()["wifi"]["mode"].as<uint8_t>();
	uint8_t nextMode = (curMode + 1) % 6;

	return (enum HAP_WIFI_MODE)nextMode;
}

HAP_WIFI_MODE HAPWiFiHelper::getNextMode(enum HAP_WIFI_MODE mode){
	uint8_t nextMode = ((uint8_t)mode + 1) % 6;
	return (enum HAP_WIFI_MODE)nextMode;
}