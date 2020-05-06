//
// HAPServer.cpp
// Homekit
//
//  Created on: 08.08.2017
//      Author: michael
//

#include <WiFi.h>
#include <WString.h>
#include <algorithm>
#include "HAPServer.hpp"
#include "HAPLogger.hpp"
#include "HAPHelper.hpp"
#include "HAPBonjour.hpp"
#include "HAPDeviceID.hpp"

#include "HAPEncryption.hpp"

#include "../WiFiCredentials.hpp"

#include "concat.h"

#if HAP_DEBUG
#include "esp_system.h"
#endif

#if HAP_USE_MBEDTLS_HKDF
#include "m_hkdf.h"
#else
#include "hkdf.h"
#endif

#include <mbedtls/version.h>
#include <sodium.h>

#if HAP_USE_MBEDTLS_CURVE25519
#include "m_curve25519.h"
#endif

#if HAP_USE_LIBSODIUM
#else
#include "curve25519.h"
#include "ed25519.h"
#endif

#if HAP_USE_MBEDTLS_POLY
#include "m_chacha20_poly1305.h"
#else
#include "chacha20_poly1305.h"
#endif

#define IS_BIG_ENDIAN (*(uint16_t *)"\0\xff" < 0x100)

#if HAP_PRINT_QRCODE
#include "qrcode.h"
#include "HAPSVG.hpp"
#endif


#if HAP_DEBUG_HEAP
#include "esp_heap_trace.h"
#define HAP_DEBUG_HEAP_NUM_RECORDS 100
static heap_trace_record_t trace_record[HAP_DEBUG_HEAP_NUM_RECORDS]; // This buffer must be in internal RAM
#endif

//
// init static variables
//
struct tm HAPServer::_timeinfo;
EventManager HAPServer::_eventManager;

HAPServer::HAPServer(uint16_t port, uint8_t maxClients)
:  _server(port)
, __HOMEKIT_SIGNATURE("\x25\x48\x4f\x4d\x45\x4b\x49\x54\x5f\x45\x53\x50\x33\x32\x5f\x46\x57\x25")
{
	_port = port;

	_previousMillis = 0;

	

#if HAP_DEBUG
	_previousMillisHeap = 0;	
#endif

	_minimalPluginInterval = HAP_MINIMAL_PLUGIN_INTERVAL;
	_accessorySet = new HAPAccessorySet();

#if HAP_ENABLE_WEBSERVER
	_webserver = new HAPWebServer();
	_webserver->setAccessorySet(_accessorySet);
#endif

	_stopEvents = false;
	_stopPlugins = false;

#if HAP_USE_MBEDTLS_SRP
	_srp = (Srp*) malloc( sizeof(Srp) );
	_srpInitialized = false;
#endif

	_isInPairingMode = false;
	
	_homekitFailedLoginAttempts = 0;

}

HAPServer::~HAPServer() {
	// TODO Auto-generated destructor stub
	delete _accessorySet;
}

bool HAPServer::begin(bool resume) {

#if HAP_DEBUG_HEAP
	ESP_ERROR_CHECK( heap_trace_init_standalone(trace_record, HAP_DEBUG_HEAP_NUM_RECORDS) );
#endif


	int error_code = 0;
	
	// Generate DeviceID	
	uint8_t *baseMac = HAPDeviceID::generateID();	

	if (resume == false){		

		// Logging
		HAPLogger::setPrinter(&Serial);
		HAPLogger::setLogLevel(HAP_LOGLEVEL);

		/*
		 * Configuration
		 */
		LogV("Loading configuration ...", false);	
		auto callback = std::bind(&HAPServer::updateConfig, this);
		_config.registerCallback(callback);
		_config.begin();

		bool res = _config.load();
		if (res == false) {
			LogE("ERROR: Could not load configuration!", true);			
		} else {
			LogV("OK", true);
		}
		HAPLogger::setLogLevel(_config.config()["homekit"]["loglevel"].as<uint8_t>());

#if HAP_DEBUG

		HAPLogger::printInfo();

		LogD( "\nDevice Information", true);
		LogD("===================================================", true);	
		LogD("Device ID:    " + HAPDeviceID::deviceID(), true);	
		LogD("Chip ID:      " + HAPDeviceID::chipID(), true);
		LogD("MAC address:  " + WiFi.macAddress(), true);


		esp_chip_info_t chip_info;
		esp_chip_info(&chip_info);

		LogD("", true);	
		LogD("ESP32:", true);
		LogD("   features:  " + String(chip_info.features), true); 
		LogD("   cores:     " + String(chip_info.cores), true); 
		LogD("   revision:  " + String(chip_info.revision), true); 

		LogD("", true);	
		LogD("Flash chip:", true);
		LogD("   size:      " + String(ESP.getFlashChipSize()), true);
		LogD("   speed:     " + String(ESP.getFlashChipSpeed()), true);
		LogD("   mode:      " + String(ESP.getFlashChipMode()), true);
		
		LogD("", true);	
		LogD("Endian:       ", false);
		LogD(IS_BIG_ENDIAN ? "BIG" : "little", true);

		LogD("Main stack:   ", false);
		LogD(String(CONFIG_MAIN_TASK_STACK_SIZE), true);


		LogD("", true);	
		LogD("Fakegato:", true);
		LogD("   interval:  ", false);
		LogD(String(HAP_FAKEGATO_INTERVAL), true);
		LogD("   buffer:    ", false);
		LogD(String(HAP_FAKEGATO_BUFFER_SIZE), true);		

		char mbedtlsVersion[32];
		mbedtls_version_get_string_full(mbedtlsVersion);

		LogD("", true);	
		LogD("Versions:", true);    
		LogD("   SDK:       " + String(ESP.getSdkVersion()), true);
		LogD("   mbedtls:   " + String(mbedtlsVersion), true);
		LogD("   libsodium: " + String(sodium_version_string()), true);	
		LogD("===================================================", false);
		LogD("", true);
		LogD("Now starting ...", true);


		HAPHelper::getPartionTableInfo();
#endif	

#if HAP_RESET_EEPROM
		LogW("Reset EEPROM - Delete pairings ...", false);
		_accessorySet->getPairings()->resetEEPROM();		
		LogW("OK", true);
#endif

#if HAP_WEBSERVER_USE_SPIFFS        
		SPIFFS.begin();
#if HAP_DEBUG
    	LogD("Listing files of SPIFFS:", true);
    	HAPServer::listDir(SPIFFS, "/", 0);
#endif
#endif

	}

	// Hostname
	char* hostname = (char*) malloc(sizeof(char) * strlen(HAP_HOSTNAME_PREFIX) + 8);
	sprintf(hostname, "%s-%02X%02X%02X", HAP_HOSTNAME_PREFIX, baseMac[3], baseMac[4], baseMac[5]);

	// WiFi
	_wifi.begin(&_config, std::bind(&HAPServer::begin, this, std::placeholders::_1), hostname);	
	// if captive portal, return here	
	if (_config.config()["wifi"]["mode"].as<uint8_t>() == (uint8_t)HAPWiFiModeAccessPoint){
		free(hostname);
		return true;
	}	

	
#if HAP_NTP_ENABLED
	LogV( F("Starting NTP client ..."), false);
	configTzTime(HAP_NTP_TZ_INFO, HAP_NTP_SERVER_URL);

	if (getLocalTime(&_timeinfo, 10000)) {  // wait up to 10sec to sync
		//Serial.println(&_timeinfo, "Time set: %B %d %Y %H:%M:%S (%A)");		
		LogV(F("OK"), true);
		LogV("Set time to: " + timeString(), true);

		_config.setRefTime(timestamp());
	} else {
		LogV(F("[ERROR] Time not set!"), true);	
  	}
#endif


	// Todo: Move pairings to accessorySet
	LogV("Loading pairings ...", false);	
	if (_accessorySet->getPairings()->begin()) {

		_accessorySet->getPairings()->load();		
	} 
	LogV("OK", true);
	LogV("Loaded " + String(_accessorySet->getPairings()->size()) + " pairings from EEPROM", true);

#if HAP_DEBUG
	_accessorySet->getPairings()->print();
#endif


	if ( isPaired() ){
		LogV("Loading long term keys ...", false);	
		_longTermContext = (struct HAPLongTermContext*) calloc(1, sizeof(struct HAPLongTermContext));
		if (_longTermContext == NULL) {
			LogE( F("[ERROR] Initializing struct _longTermContext failed!"), true);
			return false;
		}
	
		_longTermContext->publicKey = (uint8_t*) malloc(sizeof(uint8_t) * ED25519_PUBLIC_KEY_LENGTH);
		_longTermContext->publicKeyLength = ED25519_PUBLIC_KEY_LENGTH;
		_longTermContext->privateKey = (uint8_t*) malloc(sizeof(uint8_t) * ED25519_PRIVATE_KEY_LENGTH);
		_longTermContext->privateKeyLength = ED25519_PRIVATE_KEY_LENGTH;
		
	 	_accessorySet->getPairings()->loadKeys(_longTermContext->publicKey, _longTermContext->privateKey);


// #if HAP_DEBUG
//  		Serial.println("_longTermContext->publicKey: ");
//  		HAPHelper::arrayPrint(_longTermContext->publicKey, ED25519_PUBLIC_KEY_LENGTH);

//  		Serial.println("_longTermContext->privateKey: ");
//  		HAPHelper::arrayPrint(_longTermContext->privateKey, ED25519_PRIVATE_KEY_LENGTH);
// #endif 		


		LogV("OK", true);
	}

	LogV("Starting encrpytion engine ...", false);
	error_code = HAPEncryption::begin();
	if (error_code != 0){
		LogE(F("[ERROR] Failed to initialize libsodium!"), true);		
	} else {
		LogV("OK", true);
	}


	LogV( F("Setup accessory ..."), false);
	_accessorySet->setModelName(hostname);	
	_accessorySet->setAccessoryType(HAP_ACCESSORY_TYPE_BRIDGE);
	_accessorySet->setPinCode(HAP_PIN_CODE);
	_accessorySet->begin();
	LogV("OK", true);

	
	
	// 
	// Event Manager
	//
	LogV( F("\nAdding listener to event manager ..."), false);
	// Incoming
  	listenerNotificaton.mObj = this;
  	listenerNotificaton.mf = &HAPServer::handleEvents;
	_eventManager.addListener( EventManager::kEventNotifyController, &listenerNotificaton );
	

	// UpdateConfigNumber
	listenerUpdateConfigNumber.mObj = this;
  	listenerUpdateConfigNumber.mf = &HAPServer::handleEventUpdateConfigNumber;
	_eventManager.addListener( EventManager::kEventIncrementConfigNumber, &listenerUpdateConfigNumber );  	  	
	
	// kEventUpdatedConfig
	listenerConfigUpdated.mObj = this;
  	listenerConfigUpdated.mf = &HAPServer::handleEventUpdatedConfig;
	_eventManager.addListener( EventManager::kEventUpdatedConfig, &listenerConfigUpdated );  	  	

	// kEventUpdatedConfig
	listenerRebootNow.mObj = this;
  	listenerRebootNow.mf = &HAPServer::handleEventRebootNow;
	_eventManager.addListener( EventManager::kEventRebootNow, &listenerRebootNow );  	  	

	LogV(F("OK"), true);







	// 
	// QR Code generation
	// 
#if HAP_GENERATE_XHM	

	/*
	 * Generate setupID and xmi uri
	 */
	_accessorySet->generateSetupID();
	
	//LogD("Homekit setupID: ", false);
	//LogD(_accessorySet->et.setupID(), true);


	LogV("Homekit X-HM URI: " + String(_accessorySet->xhm()), true);
	LogV("Homekit setup hash:  " + String(_accessorySet->setupHash()), true);



#if HAP_PRINT_QRCODE || HAP_DEBUG_QRCODE_SVG

	/*
	 * Generate QR Code
	 */
	QRCode qrCode;
	uint8_t qrcodeData[qrcode_getBufferSize(3)];
	qrcode_initText(&qrCode, qrcodeData, 3, ECC_HIGH, _accessorySet->xhm());

#if HAP_PRINT_QRCODE
	for (uint8_t y = 0; y < qrCode.size; y++) {
		// Left quiet zone
		Serial.print("        ");
		// Each horizontal module
		for (uint8_t x = 0; x < qrCode.size; x++) {
            // Print each module (UTF-8 \u2588 is a solid block)
			Serial.print(qrcode_getModule(&qrCode, x, y) ? "\u2588\u2588": "  ");
		}
		Serial.println("");
	}
#endif	

#if HAP_DEBUG_QRCODE_SVG
	Serial.println("SVG:");
	HAPSVG::drawQRCode(&Serial, &qrCode);
	Serial.println("");
#endif	


#endif

#endif


	//
	// Starting Webserver
	// 
#if HAP_ENABLE_WEBSERVER

	if (_config.config()["webserver"]["enabled"]){
		LogV("Starting webserver ...", false);

		// #if HAP_API_ADMIN_MODE	
		// 	// Get hap accessories
		// 	_webserver.setCallbackApiAccessories(std::bind(&HAPServer::callbackGetAccessories, this));
		// #endif

		// #if HAP_DEBUG
		// 	_webserver.setCallbackApiDebugHapClients(std::bind(&HAPServer::callbackApiHapClients, this));
		// #endif	
		_webserver->setAccessorySet(_accessorySet);
		_webserver->setConfig(&_config);
		_webserver->setEventManager(&_eventManager);
		_webserver->begin();
		LogV("OK", true);
	}
#endif


  	// 
  	// Loading fakegato factory
  	// 

	// Setting Reference Time to FakeGato
	_fakeGatoFactory.setRefTime(_config.config()["fakegato"]["reftime"].as<uint32_t>());


  	// 
  	// Loading plugins
  	// 
  	LogI( F("Loading plugins ..."), true);

	auto &factory = HAPPluginFactory::Instance();        
    std::vector<String> names = factory.names();    

    for(std::vector<String>::iterator it = names.begin(); it != names.end(); ++it) {
    	//Serial.println(*it);
    	auto plugin = factory.getPlugin(*it);
		
		plugin->setAccessorySet(_accessorySet);
		plugin->setConfig(_config.config()["plugins"][plugin->name()]);
		// plugin->setWebServer(_webserver);
	
		if ( plugin->isEnabled()) {    		

			plugin->setFakeGatoFactory(&_fakeGatoFactory);
			plugin->addEventListener(&_eventManager);
						
			if (plugin->begin()) {

				LogI("   - ENABLED  " + plugin->name(), false);
				LogD(" (v" + String(plugin->version()) + ")", false);	
				LogD(" of type: " + String(plugin->type()), false);
				LogI("", true);

				HAPAccessory *accessory = plugin->initAccessory();

				if (accessory != nullptr){
					_accessorySet->addAccessory(accessory); 					 					
				}	    		


				if (_config.config()["webserver"]["enabled"]){
					std::vector<HAPWebServerPluginNode*> vector = plugin->getResourceNodes();
					if (!vector.empty()) {
						for (auto node : vector) {
							_webserver->registerPluginNode(node);
						}
					}
				}

				// initial handle for HAP values
				plugin->handle(true);  

				if (_minimalPluginInterval == HAP_MINIMAL_PLUGIN_INTERVAL && _minimalPluginInterval < plugin->interval() ) {
					_minimalPluginInterval = plugin->interval();	
				} else if (_minimalPluginInterval > plugin->interval()) {
					_minimalPluginInterval = plugin->interval();
				}
				
				_plugins.push_back(std::move(plugin));

			} else {
				LogW("   - DISABLED " + plugin->name(), false);
				LogD(" (v" + String(plugin->version()) + ")", false);	
				LogD(" of type: " + String(plugin->type()), false);
				LogW("", true);
			}
			
    	} else {
    		LogW("   - DISABLED " + plugin->name(), false);
    		LogD(" (v" + String(plugin->version()) + ")", false);	
    		LogD(" of type: " + String(plugin->type()), false);
    		LogW("", true);
    	}
	}

	//
	// Starting HAP server
	// 
	LogV( F("Starting homekit server ..."), false);
	_server.begin();
	_server.setNoDelay(true);
	LogV(F("OK"), true);
  	
	// 
	// Bonjour
	// 
	// Set up mDNS responder:
	// - first argument is the domain name, in this example
	//   the fully-qualified domain name is "esp8266.local"
	// - second argument is the IP address to advertise
	//   we send our IP address on the WiFi network
	LogD( "Advertising bonjour service ...", false);
	if (!mDNSExt.begin(_accessorySet->modelName())) {
		LogE( "ERROR; Starting mDNS responder failed!", true);
		return false;
	}

	// Add service to MDNS-SD
	mDNSExt.addService("_hap", "_tcp", _port);

#if HAP_ENABLE_WEBSERVER	
	if (_config.config()["webserver"]["enabled"]){	
		mDNSExt.addService("http", "_tcp", 443);
	}
#endif
	LogD( " OK", true);


#if HAP_UPDATE_ENABLE_FROM_WEB || HAP_UPDATE_ENABLE_OTA
	if (_config.config()["update"]["ota"]["enabled"]){
		//
		// Starting Arduino OTA
		//
		LogD( "Starting Arduino OTA ...", false);
		_updater.begin(&_config);
		LogD( " OK", true);
	}

#if HAP_UPDATE_ENABLE_FROM_WEB
	if (_config.config()["update"]["web"]["enabled"]){
#if HAP_UPDATE_ENABLE_SSL	
		// ToDo: Implement proper update routine with root cert container download etc
		_updater.setHostAndPort(HAP_UPDATE_SERVER_HOST, HAP_UPDATE_SERVER_PORT);
#else
		_updater.setHostAndPort(HAP_UPDATE_SERVER_HOST, HAP_UPDATE_SERVER_PORT);
#endif
	}

#endif

#endif

	if ( !updateServiceTxt() ){
		LogE( "ERROR: Advertising HAP service failed!", true);
		return false;
	}




#if HAP_DEBUG_HOMEKIT
	LogD(_accessorySet->describe(), true);    
#endif

	
	stopEvents(false);

	// queue event
	_eventManager.queueEvent(EventManager::kEventHomekitStarted, HAPEvent());
	
	//
	// Show event listerners
	//
  	LogV( "Number of event listeners:  ", false );
  	LogV( String(_eventManager.numListeners()), true );



	LogD(_accessorySet->describe(), true);

	// 
	// Startup completed
	// 	
	LogI("Homekit has started successfully!", true);	
	if (!_accessorySet->isPaired()) {
		LogI("Homekit pin code: ", false);
		LogI(_accessorySet->pinCode(), true);
	}

#if HAP_ENABLE_WEBSERVER
	if (_config.config()["webserver"]["enabled"]){	
		LogI("Webinterface available at: ", false);
		if (HAP_WEBSERVER_USE_SSL) {
			LogI("https://", false);
		} else {
			LogI("http://", false);
		}
		LogI(String(hostname), true);
	}
#endif

// #if HAP_UPDATE_ENABLE_FROM_WEB
// 	if (_config.config()["update"]["web"]["enabled"]){		
// 		if ( _updater.checkUpdateAvailable() ) {
// 			LogI("Online pdate available: " + _updater.onlineVersion() , true);
// 		} else {
// 			LogV( F("No update available"), true);	
// 		}
// 	}
// #endif

	free(hostname);

	// Handle any events that are in the queue
	_eventManager.processEvent();	

	return true;
}


bool HAPServer::updateServiceTxt() {
	/*
	mdns_txt_item_t hapTxtData[8] = {
        {(char*)"c#"    ,(char*)"2"},                   // c# - Configuration number
        {(char*)"ff"    ,(char*)"0"},                   // ff - feature flags
        {(char*)"id"    ,(char*)"11:22:33:44:55:66"},   				// id - unique identifier
        {(char*)"md"	,(char*)"Huzzah32"},            // md - model name
        {(char*)"pv"    ,(char*)"1.0"},                 // pv - protocol version
        {(char*)"s#"    ,(char*)"1"},                   // s# - state number
        {(char*)"sf"    ,(char*)"1"},                   // sf - Status flag
        {(char*)"ci"    ,(char*)"2"}                    // ci - Accessory category indicator
    }; 

    return mDNSExt.addServiceTxtSet((char*)"_hap", "_tcp", 8, hapTxtData);


    */
#if HAP_GENERATE_XHM
	mdns_txt_item_t hapTxtData[9];
#else
	mdns_txt_item_t hapTxtData[8];
#endif
	uint8_t *baseMac = HAPDeviceID::generateID();

	// c# - Configuration number
	// Current configuration number. Required.
	// Must update when an accessory, service, or characteristic is added or removed 
	// 
	// Accessories must increment the config number after a firmware update. 
	// This must have a range of 1-4294967295 and wrap to 1 when it overflows. 
	// This value must persist across reboots, power cycles, etc.
	hapTxtData[0].key 		= (char*) "c#";
	hapTxtData[0].value 	= (char*) malloc(sizeof(char) * HAPHelper::numDigits(_accessorySet->configurationNumber) );
	sprintf(hapTxtData[0].value, "%lu", (unsigned long)_accessorySet->configurationNumber );		

	// id - unique identifier
	hapTxtData[1].key 		= (char*) "id";
	hapTxtData[1].value 	= (char*) malloc(sizeof(char) * 18);
	sprintf(hapTxtData[1].value, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
	
	// ff - feature flags
	// Supports HAP Pairing. This flag is required for all HomeKit accessories.
	hapTxtData[2].key 		= (char*) "ff";
	hapTxtData[2].value 	= (char*) malloc(sizeof(char));
	sprintf(hapTxtData[2].value, "%d", _accessorySet->isPaired() );
	
	// md - model name	
	hapTxtData[3].key 		= (char*) "md";
	hapTxtData[3].value 	= (char*) malloc(sizeof(char) * strlen(_accessorySet->modelName()));
	sprintf(hapTxtData[3].value, "%s", _accessorySet->modelName());

	// pv - protocol version
	hapTxtData[4].key 		= (char*) "pv";
	hapTxtData[4].value 	= (char*) HOMEKIT_PROTOKOL_VERSION;
	
	// s# - state number
	// must have a value of "1"
	hapTxtData[5].key 		= (char*) "s#";
	hapTxtData[5].value 	= (char*) "1";

	// sf - feature flags
	// Status flags (e.g. "0x04" for bit 3). Value should be an unsigned integer. 
	// Required if non-zero.
	// 1 if not paired
	// 0 if paired ??
	hapTxtData[6].key 		= (char*) "sf";
	hapTxtData[6].value 	= (char*) malloc(sizeof(char));
	sprintf(hapTxtData[6].value, "%d", !_accessorySet->isPaired() );

	 // ci - Accessory category indicator
	hapTxtData[7].key 		= (char*) "ci";
	hapTxtData[7].value 	= (char*) malloc(sizeof(char) * HAPHelper::numDigits(_accessorySet->accessoryType()) );
	sprintf(hapTxtData[7].value, "%d", _accessorySet->accessoryType() );

#if HAP_GENERATE_XHM
	// sh - Required for QR Code 
	hapTxtData[8].key 		= (char*) "sh";
	hapTxtData[8].value 	= (char*) malloc(sizeof(char) * strlen(_accessorySet->setupHash()) );


	sprintf(hapTxtData[8].value, "%s", _accessorySet->setupHash() );

    return mDNSExt.addServiceTxtSet((char*)"_hap", "_tcp", 9, hapTxtData);
#else
    return mDNSExt.addServiceTxtSet((char*)"_hap", "_tcp", 8, hapTxtData);
#endif  
 
}



void HAPServer::handle() {

#if HAP_DEBUG
	// Free Heap every interval ms
	if ( millis() - _previousMillisHeap >= 1000) {
	    // save the last time you blinked the LED
	    _previousMillisHeap = millis();
	    Heap(_clients.size(), _eventManager.getNumEventsInQueue());
	}
#endif

	if ( (_wifi.captiveInitialized()) && (_config.config()["wifi"]["mode"].as<uint8_t>() == (uint8_t)HAPWiFiModeAccessPoint) ){
		_wifi.handle();
		return;
	}


	// Handle new clients
	WiFiClient client = _server.available();
	if (client) {

#if HAP_DEBUG_HEAP        
    	LogE("+++++++++++++++++++ " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " start", true);
    	HAPLogger::logFreeHeap(0,0, COLOR_MAGENTA);
		// ESP_ERROR_CHECK( heap_trace_init_standalone(trace_record, HAP_DEBUG_HEAP_NUM_RECORDS) );
		ESP_ERROR_CHECK( heap_trace_start(HEAP_TRACE_LEAKS) );
#endif

		HAPClient hapClient;

		// New client connected
		hapClient.client = client;
		hapClient.state = HAP_CLIENT_STATE_CONNECTED;

		handleClientState(&hapClient);
	}



	for (auto& hapClient : _clients) {

		// Connected
		if (hapClient.client.connected()) {

			// Available
			unsigned long timeout = 100;
			unsigned long previousMillis = millis();
			while ( millis() - previousMillis < timeout) {

				delay(1);

				if (hapClient.client.available()) {
					hapClient.state = HAP_CLIENT_STATE_AVAILABLE;
					handleClientState(&hapClient);
					break;
				}

				// Idle
				hapClient.state = HAP_CLIENT_STATE_IDLE;
			}


		} else {
			// Disconnected
			hapClient.state = HAP_CLIENT_STATE_DISCONNECTED;
			handleClientState(&hapClient);
		}

		// LogV( "HAPClient state " + hapClient.getClientState(), true );
		// delay(1);

	}

	// Handle ntp client
#if HAP_NTP_ENABLED
	getLocalTime(&_timeinfo);
    //Serial.println(&_timeinfo, HAP_NTP_TIME_FORMAT);  	
#endif

	// Handle Webserver
#if HAP_ENABLE_WEBSERVER
	if (_config.config()["webserver"]["enabled"]){	
		_webserver->handle();
	}
#endif

	// Handle Arduino OTA
#if HAP_UPDATE_ENABLE_FROM_WEB || HAP_UPDATE_ENABLE_OTA
	if (_config.config()["update"]["ota"]["enabled"] || _config.config()["update"]["web"]["enabled"]){
		_updater.handle();
	}
#endif	

	// Handle plugins
	if (!_stopPlugins){			
		for (auto & plugin : _plugins) {			
			if (plugin->isEnabled()) {
				plugin->handle();					
			}			
		}
	}

	// 
	// Handle fakeGatos
	_fakeGatoFactory.handle();

	// Handle any events that are in the queue
	_eventManager.processEvent();		
}

#if HAP_NTP_ENABLED


/**
 * @brief Return current time as string
 * 		  Format: YYYY-MM-DD HH:MM:SS.sss
 * 		  example 2019-09-22 21:30:57.239
 * 
 * @return String current time
 */
String HAPServer::timeString(){
	timeval curTime;
	gettimeofday(&curTime, NULL);

	char buffer [30];
	if (String(HAP_NTP_TIME_FORMAT).endsWith(".%f")){
		const char* timeformat = String(HAP_NTP_TIME_FORMAT).substring(0, String(HAP_NTP_TIME_FORMAT).length() - 3).c_str();
		strftime(buffer, 30, timeformat, localtime(&curTime.tv_sec));

		int milli = curTime.tv_usec / 1000;
		char currentTime[35] = "";
		sprintf(currentTime, "%s.%03d", buffer, milli);
		
		return String(currentTime);
	} else {
		strftime(buffer, 30, HAP_NTP_TIME_FORMAT, localtime(&curTime.tv_sec));
	}
	
	return String(buffer);
}


/**
 * @brief Returns current timestamp in seconds
 * 
 * @return uint32_t timestamp (in seconds)
 */
uint32_t HAPServer::timestamp(){
	timeval now;
	gettimeofday(&now, NULL);	
	return now.tv_sec;
}

#endif


void HAPServer::handleClientDisconnect(HAPClient hapClient) {
	std::vector<HAPClient>::iterator position = std::find(_clients.begin(), _clients.end(), hapClient);
	if (position != _clients.end()) { // == myVector.end() means the element was not found

		if (position->client.connected() ) {

			LogW("Client disconnecting", true);
			position->client.stop();
		}
		
		position->request.clear();
		position->clear();		

		_clients.erase(position);

#if HAP_DEBUG_HEAP            
		ESP_ERROR_CHECK( heap_trace_stop() );
		Serial.println("HEAP_DUMP:");
		heap_trace_dump();

		LogE("=================== " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " end", true);
		HAPLogger::logFreeHeap(0,0, COLOR_MAGENTA);

#endif

		return;
	}
	//LogE( F( "FAILED"), true );
}

void HAPServer::handleClientState(HAPClient* hapClient) {
	switch(hapClient->state) {
		case HAP_CLIENT_STATE_DISCONNECTED:
			LogD( ">>> client [" + hapClient->client.remoteIP().toString() + "] disconnected", true );
			handleClientDisconnect( *hapClient );

			break;
		case HAP_CLIENT_STATE_CONNECTED:		
			LogD( "<<< client [" + hapClient->client.remoteIP().toString() + "] connected", true );		
			_clients.push_back(*hapClient);

			break;
		case HAP_CLIENT_STATE_AVAILABLE:
			LogD( "<<< client [" + hapClient->client.remoteIP().toString() + "] available: ", false );
			LogD( String(hapClient->client.available()), true );

			handleClientAvailable(hapClient);

			break;
		case HAP_CLIENT_STATE_SENT:
			LogD( F("<<< client sent"), true );
			break;
		case HAP_CLIENT_STATE_RECEIVED:
			LogD( F("<<< client received"), true );
			break;
		case HAP_CLIENT_STATE_IDLE:
			LogD( "<<< client [" + hapClient->client.remoteIP().toString() + "] idle", true );
			break;
		case HAP_CLIENT_STATE_ALL_PAIRINGS_REMOVED:
			LogD( "<<< client [" + hapClient->client.remoteIP().toString() + "] admin removed", true );
			handleAllPairingsRemoved();
			break;
	}
}

void HAPServer::handleAllPairingsRemoved(){
	LogV( F("<<< Handle admin removed ..."), false);
	for (int i=0; i < _clients.size(); i++){
		LogD("\nClosing connection to client [" + _clients[i].client.remoteIP().toString() + "]", true);
		_clients[i].client.stop();
		_clients[i].state = HAP_CLIENT_STATE_DISCONNECTED;
	}
	LogV("OK", true);
}

void HAPServer::handleClientAvailable(HAPClient* hapClient) {
	_curLine = "";
	
	//LogD( F("<<< Handle client available"), true);
	
	while ( hapClient->client.available() ) {

		if (hapClient->isEncrypted()) {
			processIncomingEncryptedRequest( hapClient );	
		} else {
			processIncomingRequest( hapClient );	
		}
		
		delay(1);
	}

#if HAP_DEBUG
	LogD(_curLine, true);
#endif

	if ( !hapClient->client.connected() ) {		
		hapClient->state = HAP_CLIENT_STATE_DISCONNECTED;
	}

	// Update client state *print*
	handleClientState( hapClient );

	// clear request
	hapClient->request.clear();
	hapClient->clear();

}


void HAPServer::processIncomingEncryptedRequest(HAPClient* hapClient){

#if HAP_DEBUG_HEAP        
    LogE("+++++++++++++++++++ " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " start", true);
    HAPLogger::logFreeHeap(0,0, COLOR_MAGENTA);
#endif


	LogV( F("<<< Handle encrypted request ..."), false);

	//
    // Each HTTP message is split into frames no larger than 1024 bytes
    // 
    // Each frame has the following format:
    // 
    // < 2:    AAD for little endian length of encrypted data (n) in bytes>
    // < n:    encrypted data according to AEAD algorithm, up to 1024 bytes>
    // <16:    authTag according to AEAD algorithm>
    // 

	
	//String bodyData = "";

	while ( hapClient->client.available() )	{


		//
	    // AAD
	    //
	    // < 2:    AAD for little endian length of encrypted data (n) in bytes>
	    // uint8_t aad[HAP_AAD_LENGTH];
	    // aad[0] = (uint8_t) (length >> 8);   // Get upper byte of 16-bit var;
	    // aad[1] = (uint8_t) length;          // Get lower byte of 16-bit var;
		uint8_t AAD[HAP_ENCRYPTION_AAD_SIZE];		
		hapClient->client.readBytes(AAD, HAP_ENCRYPTION_AAD_SIZE);
		
		uint16_t trueLength = ((uint16_t)AAD[1] << 8) | AAD[0];
		int availableSize = hapClient->client.available() - HAP_ENCRYPTION_HMAC_SIZE;	// 16 is the size of the HMAC

		// Serial.printf("AAD: %02X%02X - %d\n", AAD[0], AAD[1], trueLength);				
		// Serial.printf("availableSize: %d\n", availableSize);

		// LogD("\nNeed " + String(trueLength) + " bytes and have " + String(availableSize) + " bytes", true);
		while (trueLength > availableSize) {
			// The packet is bigger than the available data; wait till more comes in
			delay(1);
		}

	    // 
	    // nonce
	    //
	    // The 32-bit fixed-common part of the 96-bit ( 12? ) nonce 
	    // is all zeros: 00 00 00 00.
	    //
	    // length 8
	    //
	    // < n:    encrypted data according to AEAD algorithm, up to 1024 bytes>
	    //
	    // Needs to be incremented each time it is called after the 1st 4 bytes
		int nonce = hapClient->encryptionContext.decryptCount;

		//
		// cipherText
		uint8_t cipherText[trueLength];		
		hapClient->client.readBytes(cipherText, trueLength);
		
		// Serial.println("cipherText:");
		// HAPHelper::arrayPrint(cipherText, trueLength);


		// 
		// hmac
		uint8_t hmac[HAP_ENCRYPTION_HMAC_SIZE];	// 16 is the size of the HMAC
		availableSize = hapClient->client.available();
		//LogD("Need " + String(HAP_ENCRYPTION_HMAC_SIZE) + " bytes and have " + String(availableSize) + " bytes", true);
		while ( HAP_ENCRYPTION_HMAC_SIZE > availableSize ) {
			// The packet is bigger than the available data; wait till more comes in
			delay(1);
		}

		hapClient->client.readBytes(hmac, HAP_ENCRYPTION_HMAC_SIZE);

		uint8_t decrypted[trueLength];
		HAPEncryption::verifyAndDecrypt(decrypted, cipherText, trueLength, hmac, AAD, nonce, hapClient->encryptionContext.decryptKey);		

		// increment decrypt counter
		hapClient->encryptionContext.decryptCount++;

		
		int bodyDataLen = 0;
		uint8_t *bodyData;
		parseRequest(hapClient, (char*)decrypted, trueLength, &bodyData, &bodyDataLen);
		
#if HAP_DEBUG
		HAPHelper::array_print("bodyData", bodyData, bodyDataLen);
#endif		

		handlePath(hapClient, bodyData, bodyDataLen);
		
		if (bodyDataLen > 0){
			free(bodyData);
		}
		
	}

#if HAP_DEBUG_HEAP            
	LogE("=================== " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " end", true);
    HAPLogger::logFreeHeap(0,0, COLOR_MAGENTA);
#endif		
}

bool HAPServer::stopEvents(){
	return _stopEvents;
}

void HAPServer::stopEvents(bool value) {
	
	if (value) {
		LogD("<<< Stopping Events", true);
	} else {
		LogD(">>> Starting Events", true);
	}
	_stopEvents = value;
}

bool HAPServer::handlePath(HAPClient* hapClient, uint8_t* bodyData, size_t bodyDataLen){
	
	bool validPath = false;
	// /accessories
	if ( (hapClient->request.path == "/accessories") && (hapClient->request.method == METHOD_GET) ) {
		validPath = true;
		handleAccessories( hapClient );
	} 

	// /characteristics
	else if ( hapClient->request.path == "/characteristics" ) {

		// GET
		if ( hapClient->request.method == METHOD_GET ) {
			validPath = true;
			handleCharacteristicsGet( hapClient );
		} 
		// PUT
		else if ( (hapClient->request.method == METHOD_PUT) && (hapClient->request.contentType == "application/hap+json") ) {
			validPath = true;
			// char bodyDataStr[bodyDataLen + 1];
			// strncpy(bodyDataStr, (char*)bodyData, bodyDataLen);
			
			handleCharacteristicsPut( hapClient, String((char*)bodyData) );
		}	

	} else if ( hapClient->request.path == "/pairings" ) {
		// POST
		if ( ( hapClient->request.method == METHOD_POST ) && (hapClient->request.contentType == "application/pairing+tlv8") ) {
			validPath = true;
			handlePairingsPost( hapClient, bodyData,  bodyDataLen);
		} 	
	} 
	
	
	if (validPath == false) {
		LogE("Not yet implemented! >>> client [" + hapClient->client.remoteIP().toString() + "] ", false);
		LogE(" - method: " + String(hapClient->request.method), true);
		LogE(" - path: " + String(hapClient->request.path), true);		
		LogE(" - Content-Type: " + String(hapClient->request.contentType), true);		

		hapClient->request.clear();
		hapClient->client.stop();
		return false;
	}

	// Moved to handle functions
	//hapClient->state = CLIENT_STATE_IDLE;
	return true;
}


void HAPServer::parseRequest(HAPClient* hapClient, const char* msg, size_t msg_len, uint8_t** out, int* outLen){	

	

	int curPos = 0;
	for (int i = 0; i < msg_len; i++) {
    		//Serial.print(decrypted[i]);
		if ( msg[i] == '\r' && msg[i + 1] == '\n' ) {
			
			processIncomingLine(hapClient, String(msg).substring(curPos, i));
			i++;

			if (i - curPos == 2) {
				curPos = i + 1;
				break;
			}
			curPos = i + 1;
		}

	}	

	
	//Serial.printf(">>> curPos: %d\n", curPos);

	if (curPos + hapClient->request.contentLength == msg_len) {
		//bodyData += String(msg).substring(curPos);
		int siz = msg_len - curPos;		

		if (siz > 0) {
			*out = (uint8_t*)malloc(sizeof(uint8_t) * siz);			
			memcpy(*out, &msg[curPos], siz);		
		} 

		*outLen = siz;
	} else {
		LogW("Size mismatch", true);		
	}

	//return bodyData;
}



// // ToDo: Move to hapClient ?
// /**
//  * @brief 
//  * 
//  * @param message 
//  * @param length 
//  * @param encrypted_len 
//  * @param key 
//  * @param encryptCount 
//  * @return char* 
//  */
// char* HAPServer::encrypt(uint8_t *message, size_t length, int* encrypted_len, uint8_t* key, uint16_t encryptCount) {

	
// 	char* encrypted = (char*) calloc(1, length + (length / HAP_ENCRYPTION_BUFFER_SIZE + 1) * (HAP_ENCRYPTION_AAD_SIZE + CHACHA20_POLY1305_AUTH_TAG_LENGTH) + 1);

// 	uint8_t nonce[12] = {0,};
// 	uint8_t* decrypted_ptr = (uint8_t*)message;
// 	uint8_t* encrypted_ptr = (uint8_t*)encrypted;

// 	int err_code = 0;

// 	while (length > 0) {
// 		int chunk_len = (length < HAP_ENCRYPTION_BUFFER_SIZE) ? length : HAP_ENCRYPTION_BUFFER_SIZE;
// 		length -= chunk_len;

// 		uint8_t aad[HAP_ENCRYPTION_AAD_SIZE];
// 		aad[0] = chunk_len % 256;
// 		aad[1] = chunk_len / 256;

// 		memcpy(encrypted_ptr, aad, HAP_ENCRYPTION_AAD_SIZE);
// 		encrypted_ptr += HAP_ENCRYPTION_AAD_SIZE;
// 		*encrypted_len += HAP_ENCRYPTION_AAD_SIZE;

// 		nonce[4] = encryptCount % 256;
// 		nonce[5] = encryptCount++ / 256;

// 		err_code = chacha20_poly1305_encrypt_with_nonce(nonce, key, aad, HAP_ENCRYPTION_AAD_SIZE, decrypted_ptr, chunk_len, encrypted_ptr);	

// 		if (err_code != 0 ) {
// 			LogE("[ERROR] Encrypting failed!", true);
// 		}

// 		decrypted_ptr += chunk_len;
// 		encrypted_ptr += chunk_len + CHACHA20_POLY1305_AUTH_TAG_LENGTH;
// 		*encrypted_len += (chunk_len + CHACHA20_POLY1305_AUTH_TAG_LENGTH);
// 	}


// 	//_pairSetup->encryptCount = 0;
// 	return encrypted;
// }

void HAPServer::sendErrorTLV(HAPClient* hapClient, uint8_t state, uint8_t error){
	TLV8 response;
	response.encode(HAP_TLV_STATE, 1, state);
	response.encode(HAP_TLV_ERROR, 1, error);
	
	sendResponse(hapClient, &response); 
	response.clear();
	
	hapClient->request.clear();	
	hapClient->clear();			
	hapClient->client.stop();							

	_homekitFailedLoginAttempts++;
	_isInPairingMode = false;

	hapClient->state = HAP_CLIENT_STATE_DISCONNECTED;

	_eventManager.queueEvent(EventManager::kEventErrorOccurred, HAPEvent());

	stopEvents(false);
}


void HAPServer::processIncomingRequest(HAPClient* hapClient){



	const byte b = hapClient->client.read();

	if ( (char) b == '\n' ) {
		// if the current line is blank, you got two newline characters in a row.
		// that's the end of the client HTTP request, so send a response:
		if (_curLine.length() == 0) {


#if HAP_DEBUG_HOMEKIT
			// Handle data
			LogD( F("request: "), false);
			LogD(hapClient->request.toString(), true);
#endif

			// /identify
			if ( (hapClient->request.path == "/identify") && (hapClient->request.method == METHOD_POST) ) {
				handleIdentify(hapClient);
				hapClient->state = HAP_CLIENT_STATE_IDLE;
			}

			// has content
			else if ( hapClient->request.contentLength > 0) {
				// encode tlv8
				if ( hapClient->request.contentType == "application/pairing+tlv8" )  {


					if ( !encode(hapClient) ) {
						LogE( "ERROR: Decoding pairing request failed!", true);

						sendErrorTLV(hapClient, HAP_PAIR_STATE_M2, HAP_ERROR_UNKNOWN);
						return;
					}

					
					// pair-setup M1
					if ( (hapClient->request.path == "/pair-setup" ) && (hapClient->pairState == HAP_PAIR_STATE_M1) ) {
						
						if (_accessorySet->isPaired() == true) {
							// accessory is already paired
							LogE( "ERROR: Accessory is already paired!", true);						
							sendErrorTLV(hapClient, HAP_PAIR_STATE_M2, HAP_ERROR_AUTHENTICATON);
							return;
						} else if (_isInPairingMode == true) {
							// accessory is in pairing mode
							sendErrorTLV(hapClient, HAP_PAIR_STATE_M2, HAP_ERROR_BUSY);
							return;								
						} else if (_homekitFailedLoginAttempts >= 100) {
							// accessory has more than 100 failed attempts
							sendErrorTLV(hapClient, HAP_PAIR_STATE_M2, HAP_ERROR_MAX_TRIES);
							return;								
						} else {
							if (!handlePairSetupM1( hapClient ) ) {
								LogE( "ERROR: Pair-setup failed at M1!", true);
								
								hapClient->clear();
								hapClient->client.stop();
								stopEvents(false);

								hapClient->state = HAP_CLIENT_STATE_DISCONNECTED;
							}
						}
					}

					// pair-setup M3
					else if ( (hapClient->request.path == "/pair-setup" ) && (hapClient->pairState == HAP_PAIR_STATE_M3) ) {
						if (!handlePairSetupM3( hapClient ) ) {
							LogE( "ERROR: Pair-setup failed at M3!", true);
							hapClient->state = HAP_CLIENT_STATE_DISCONNECTED;
							hapClient->client.stop();

							stopEvents(false);
						}
					}

					// pair-setup M5
					else if ( (hapClient->request.path == "/pair-setup" ) && (hapClient->pairState == HAP_PAIR_STATE_M5) ) {
						if ( !handlePairSetupM5( hapClient ) ) {
							LogE( "ERROR: Pair-setup failed at M5!", true);
							hapClient->state = HAP_CLIENT_STATE_DISCONNECTED;
							hapClient->client.stop();
							stopEvents(false);
						}
					}
					



					// pair-verify M1
					if ( (hapClient->request.path == "/pair-verify" ) && (hapClient->verifyState == HAP_VERIFY_STATE_M1) ) {
						if ( !handlePairVerifyM1( hapClient ) ) {
							LogE( "ERROR: Pair-verify failed at M1!", true);
							hapClient->state = HAP_CLIENT_STATE_DISCONNECTED;
							hapClient->client.stop();
							stopEvents(false);
						}
					}

					// pair-verify M3
					else if ( (hapClient->request.path == "/pair-verify" ) && (hapClient->verifyState == HAP_VERIFY_STATE_M3) ) {
						if ( !handlePairVerifyM3( hapClient ) ) {
							LogE( "ERROR: Pair-verify failed at M3!", true);
							hapClient->state = HAP_CLIENT_STATE_DISCONNECTED;
							hapClient->client.stop();
							stopEvents(false);
						}
					}
				}
			}

			_curLine = "";
			return;
		} else {    					// if you got a newline, then clear currentLine:
			// Handle lines
			processIncomingLine(hapClient, _curLine);
			_curLine = "";
		}
	} else if ( (char) b != '\r') {  	// if you got anything else but a carriage return character,		
		_curLine += (char) b;      		// add it to the end of the currentLine
	}
	
	hapClient->state = HAP_CLIENT_STATE_IDLE;



}


void HAPServer::processPathParameters(HAPClient* hapClient, String line, int curPos){
	
	int index = line.indexOf("?", curPos);

	if ( index == -1) {
		// no ? in request
		hapClient->request.path = line.substring(curPos, line.indexOf(" ", curPos));
		hapClient->request.params = std::map<String, String>();
	} else {
		hapClient->request.path = line.substring(curPos, index);
		
		//Serial.print("path: ");
		//Serial.println(hapClient->request.path);

		curPos = index + 1;
		String paramStr = line.substring(curPos, line.indexOf(" ", curPos));


		//Serial.println("paramStr:");
		//Serial.println(paramStr);
		

		do {
			curPos = 0;
			int endIndex = paramStr.indexOf("&");		
			if (endIndex == -1){
				endIndex = paramStr.length();		
			}

			String keyPair = paramStr.substring(curPos, endIndex); 
			//Serial.printf("tmp: %s\n", keyPair.c_str());

			int equalIndex = keyPair.indexOf("=");

			/*
			Serial.print("key: "); 
			Serial.print(keyPair.substring(0, equalIndex));
			Serial.print(" - value: "); 
			Serial.println(keyPair.substring(equalIndex + 1));
			*/

			hapClient->request.params[keyPair.substring(0, equalIndex)] = keyPair.substring(equalIndex + 1); 

			paramStr = paramStr.substring(endIndex + 1); 
		} while ( paramStr.length() > 0 );		
	}
}


void HAPServer::processIncomingLine(HAPClient* hapClient, String line){

	// Print Line
#if HAP_DEBUG_HOMEKIT_REQUEST
	LogD( line, true );
#endif

	int curPos = 0;

	// Method
	if ( line.startsWith("POST ") ) {
		hapClient->request.method = METHOD_POST;
		curPos = 5;
		// Path
		processPathParameters( hapClient, line, curPos);
		//hapClient->request.path = line.substring(curPos, line.indexOf(" ", curPos));
	} else if ( line.startsWith("GET ") ) {
		hapClient->request.method = METHOD_GET;
		curPos = 4;
		// Path
		processPathParameters( hapClient, line, curPos);
		//hapClient->request.path = line.substring(curPos, line.indexOf(" ", curPos));
	} else if ( line.startsWith("PUT ") ) {
		hapClient->request.method = METHOD_PUT;
		curPos = 4;
		// Path
		processPathParameters( hapClient, line, curPos);
		//hapClient->request.path = line.substring(curPos, line.indexOf(" ", curPos));
	} else if ( line.startsWith("DELETE ") ) {
		hapClient->request.method = METHOD_DELETE;
		curPos = 7;
		// Path
		processPathParameters( hapClient, line, curPos);
		//hapClient->request.path = line.substring(curPos, line.indexOf(" ", curPos));
	}

	if (line.length() == 0) {
		//Serial.println("END OF HEADERS!!!");



	} else {

		String orgLine = line;
		line.toLowerCase();

		// Content Type
		if ( line.startsWith("content-type:") ) {
			curPos = 13;
			String strValue = orgLine.substring(curPos);
			strValue.trim();
			hapClient->request.contentType = strValue;
		}

		// Content Length
		else if ( line.startsWith("content-length:") ) {
			curPos = 15;
			String strValue = orgLine.substring(curPos);
			strValue.trim();
			hapClient->request.contentLength = strValue.toInt();
		}


	}
}


/**
 * @brief 
 * 
 * 
 * @param hapClient 
 * @return true 
 * @return false 
 */
bool HAPServer::encode(HAPClient* hapClient){

	uint16_t written = 0;
	bool success = false;

	// Method not supported :(
	if ( hapClient->client.peek() == 0x00) {
		hapClient->client.read();
//		Serial.println(c, HEX);
		hapClient->client.read();
//		Serial.println(c, HEX);
		hapClient->client.read();
//		Serial.println(c, HEX);
		hapClient->request.contentLength = hapClient->request.contentLength - 3;
	}

	// Reset pairing state
	hapClient->pairState = HAP_PAIR_STATE_RESERVED;

	while (hapClient->client.available()) {            	// loop while the client's connected

		if ( TLV8::isValidTLVType( hapClient->client.peek()) ) {

			byte type = hapClient->client.read();
			byte length = hapClient->client.read();

			uint8_t data[length];
			hapClient->client.readBytes(data, length);

#if HAP_DEBUG_TLV8
			LogD( F("------------------------------------------"), true );
			LogD("type:    " + String(type, HEX), true);
			LogD("length:  " + String(length), true);
			HAPHelper::array_print("value", data, length);
#endif


			if (type == HAP_TLV_STATE) {

				if (hapClient->request.path == F("/pair-verify")) {
					hapClient->verifyState = static_cast<HAP_VERIFY_STATE>(data[0]);
				} else 
					hapClient->pairState = static_cast<HAP_PAIR_STATE>(data[0]);
			}


			written += length + 2;


			if (!hapClient->request.tlv.encode(type, length, data)) {
				LogE( F("ERROR: Encoding TLV data failed!"), true );
				success = false;
				break;
			}

			if ( written == hapClient->request.contentLength ) {

#if HAP_DEBUG_TLV8
				hapClient->request.tlv.print();
#endif
				success = true;
				
				break;
			}

		} else {
			hapClient->client.read();
		}

	}

	hapClient->state = HAP_CLIENT_STATE_IDLE;
	return success;
}


void HAPServer::handleIdentify(HAPClient* hapClient){
	LogI( F("<<< Handle /identify: "), true );

	characteristics* c = _accessorySet->getCharacteristicsOfType(_accessorySet->aid(), HAP_CHARACTERISTIC_IDENTIFY);

	if ( !isPaired() ) {
		// Send 204
		hapClient->client.write( HTTP_204 );
		hapClient->client.write( HTTP_CRLF );

		if (c != NULL){
			c->setValue(String(true));
		}
		
	} else {
		// Send 400
		hapClient->client.write( HTTP_400 );

		hapClient->client.write( HTTP_CONTENT_TYPE_HAPJSON );

		hapClient->client.print( F("Content-Length: 21") );
		hapClient->client.write( HTTP_CRLF );
		hapClient->client.write( HTTP_CRLF );

		hapClient->client.print( F("{ \"status\" : -70401 }") );
		hapClient->client.write( HTTP_CRLF );
	}

	hapClient->client.write( HTTP_CRLF );

	hapClient->request.clear();
	hapClient->clear();
	
	if (c != NULL){
		c->setValue(String(false));
	}
}

/*
bool HAPServer::beginSRP(){	
	return true;
}
*/


bool HAPServer::sendEncrypt(HAPClient* hapClient, String httpStatus, const uint8_t* bytes, size_t length, bool chunked, const char* ContentType){
	bool result = true;

	LogD("\nEncrpyting response ...", false);

	String response;

	uint8_t* encrypted = nullptr;
	int encryptedLen = 0;


	response = httpStatus;
	//response += String( HTTP_CRLF );

	if (httpStatus == HTTP_204) {		
		response += String(HTTP_CRLF);

		encrypted = HAPEncryption::encrypt((uint8_t*)response.c_str(), response.length(), &encryptedLen, hapClient->encryptionContext.encryptKey, hapClient->encryptionContext.encryptCount++);

	} else {
		response += String( "Content-Type: " ); 
		response += String(ContentType);
		response += String(HTTP_CRLF);


		if ( httpStatus != EVENT_200 ) {
			response += String( HTTP_KEEP_ALIVE );	
			response += "Host: " + String(_accessorySet->modelName()) + ".local\r\n";	
		}
		

		if (chunked) {
			response += String( HTTP_TRANSFER_ENCODING );
			response += String( HTTP_CRLF );

			char chunkSize[10];
			sprintf(chunkSize, "%x%s", length, HTTP_CRLF);
			response += String(chunkSize);

		} else {
			response += String("Content-Length: " + String(length) + "\r\n" );			
			response += String( HTTP_CRLF );
		}	
		
		int buffersize = response.length() + length + 10;
		uint8_t* buffer;

		buffer = (uint8_t*) malloc(sizeof(uint8_t) * buffersize);
		buffersize = 0;

		memcpy(buffer, response.c_str(), response.length());
		buffersize += response.length();

		memcpy(buffer + response.length(), bytes, length);
		buffersize += length;

		// CRLF after payload
		{
			const char* chunkSize = "\r\n";
			memcpy(buffer + buffersize, chunkSize, strlen(chunkSize));
			buffersize	+= strlen(chunkSize);
		}


		if (chunked) {
			char chunkSize[8];
			sprintf(chunkSize, "%x\r\n", 0);
			memcpy(buffer + buffersize, (uint8_t*)chunkSize, strlen(chunkSize));
			
			buffersize += strlen(chunkSize);
		}

		// CRLF for ending
		{
			const char* chunkSize = "\r\n";
			memcpy(buffer + buffersize, chunkSize, strlen(chunkSize));
			buffersize	+= strlen(chunkSize);
		}
		
		
#if HAP_DEBUG_RESPONSES		
		HAPHelper::array_print("Response Buffer:", buffer, buffersize);
#endif

		encrypted = HAPEncryption::encrypt(buffer, buffersize, &encryptedLen, hapClient->encryptionContext.encryptKey, hapClient->encryptionContext.encryptCount++);

		free(buffer);
	}

    if (encryptedLen == 0) {
    	LogE("ERROR: Encrpyting response failed!", true);
    	hapClient->request.clear();
		hapClient->clear();
    	return false;
    } else {
		LogD(" OK", true);
    }
    

	LogD("\n>>> Sending " + String(encryptedLen) + " bytes encrypted response to client [" + hapClient->client.remoteIP().toString() + "] ...", false);

// #if HAP_DEBUG
// 	LogD(response, true);	
// 	HAPHelper::array_print("encrypted response", (uint8_t*)encrypted, encryptedLen);
// #endif


	int bytesSent = hapClient->client.write(encrypted, encryptedLen);
	// hapClient->client.flush();

	free(encrypted);

	if (bytesSent < encryptedLen) {
		LogE( "ERROR: Could not send all bytes", true );
		result = false;
	} else {
		LogD( " OK", true);
	}

	hapClient->request.clear();
	hapClient->clear();

	

#if HAP_DEBUG_HEAP        
    LogE("+++++++++++++++++++ " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " start", true);
    HAPLogger::logFreeHeap(0,0, COLOR_MAGENTA);
#endif	

	return result;
}

bool HAPServer::sendEncrypt(HAPClient* hapClient, String httpStatus, String plainText, bool chunked){		
	return sendEncrypt(hapClient, httpStatus, (const uint8_t*)plainText.c_str(), plainText.length(), chunked, "application/hap+json");
}



	
/**
 * @brief Sends a TLV8 formatted response to the hapClient
 * 		  Used for pairing and verify
 * 
 * // ToDo: Check if buffer is required!
 * // ToDo: Move to hapClient
 * 
 * @param hapClient Pointer to hapClient
 * @param response  The tlv8 response to send
 * @param chunked 	Seend in chunked mode
 * @return true     Return true on success
 * @return false 	Return false on error
 */
bool HAPServer::sendResponse(HAPClient* hapClient, TLV8* response, bool chunked, bool closeConnection){

#if HAP_DEBUG_HEAP        
    LogE("+++++++++++++++++++ " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " start", true);
    HAPLogger::logFreeHeap(0,0, COLOR_MAGENTA);
#endif

	bool result = true;

	hapClient->setChunkedMode(chunked);

	hapClient->setHeader("Content-Type", "application/pairing+tlv8");
	hapClient->setHeader("Host", _accessorySet->modelName());

	if (closeConnection) {
		hapClient->setHeader("Connection", "close");
	} else {
		hapClient->setHeader("Connection", "keep-alive");
	}

#if HAP_DEBUG_ENCRYPTION


	response->print();	
#endif

	int bytesSent = response->decode(*hapClient);

	// uint8_t outResponse[response->size()];
	// size_t written = 0;

	// response->decode(outResponse, &written);

	// if (written == 0) {
	// 	LogE("[ERROR] Failed to decode tlv8!", true);
	// 	result = false;		
	// }

	// size_t bytesSent = hapClient->write(outResponse, written);

	LogD("\nSent " + String(bytesSent) + " bytes", true);

	response->clear();
	hapClient->request.clear();
	hapClient->clear();

#if HAP_DEBUG_HEAP            
	LogE("=================== " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " end", true);
    HAPLogger::logFreeHeap(0,0, COLOR_MAGENTA);
#endif

#if 0

#if HAP_BUFFERED_SEND

	uint8_t buffer[HAP_BUFFER_SEND_SIZE];
	int offset = 0;

	// HTTP 200
	memcpy(buffer, String(HTTP_200).c_str(), String(HTTP_200).length());
	offset = String(HTTP_200).length();	


	// HTTP_CONTENT_TYPE_TLV8
	memcpy(buffer + offset, String(HTTP_CONTENT_TYPE_TLV8).c_str(), String(HTTP_CONTENT_TYPE_TLV8).length());
	offset += String(HTTP_CONTENT_TYPE_TLV8).length();


	// HTTP_KEEP_ALIVE
	memcpy(buffer + offset, String(HTTP_KEEP_ALIVE).c_str(), String(HTTP_KEEP_ALIVE).length());
	offset += String(HTTP_KEEP_ALIVE).length();


	// HTTP_HOST
	String hostString = "Host: " + String(_accessorySet->modelName()) + ".local\r\n";
	memcpy(buffer + offset, hostString.c_str(), hostString.length());
	offset += hostString.length();

	if (chunked) {
		// HTTP_TRANSFER_ENCODING  
		memcpy(buffer + offset, String(HTTP_TRANSFER_ENCODING).c_str(), String(HTTP_TRANSFER_ENCODING).length());
		offset += String(HTTP_TRANSFER_ENCODING).length();

		// CR_LF
		memcpy(buffer + offset, String(HTTP_CRLF).c_str(), String(HTTP_CRLF).length());
		offset += String(HTTP_CRLF).length();

		char chunkSize[10];
		sprintf(chunkSize, "%x%s", (int)response->size(), HTTP_CRLF);

		// Chunk size
		memcpy(buffer + offset, chunkSize, String(chunkSize).length());
		offset += String(chunkSize).length();

	} else {
		memcpy(buffer + offset, "Content-Length: ", String("Content-Length: ").length());
		offset += String("Content-Length: ").length();

		memcpy(buffer + offset, String(response->size()).c_str(), sizeof(response->size()) );
		offset += String(response->size()).length();


		// CR_LF
		memcpy(buffer + offset, String(HTTP_CRLF).c_str(), String(HTTP_CRLF).length());
		offset += String(HTTP_CRLF).length();
	}



	uint8_t outResponse[response->size()];
	size_t written = 0;

	response->decode(outResponse, &written);
	if (written == 0) {
		LogE("[ERROR] Failed to decode tlv8!", true);
	}

	memcpy(buffer + offset, outResponse, written);
	offset += written;


	// CR_LF
	memcpy(buffer + offset, String(HTTP_CRLF).c_str(), String(HTTP_CRLF).length());
	offset += String(HTTP_CRLF).length();

	if (chunked) {

		char chunkSize[10];
		sprintf(chunkSize, "%x%s", 0, HTTP_CRLF);

		// Chunk size
		memcpy(buffer + offset, chunkSize, String(chunkSize).length());
		offset += String(chunkSize).length();

		memcpy(buffer + offset, String(HTTP_CRLF).c_str(), String(HTTP_CRLF).length());
		offset += String(HTTP_CRLF).length();
	}

	int bytesSent = hapClient->client.write(buffer, offset);
	// hapClient->client.flush();

	LogD("\nSent " + String(bytesSent) + "/" + String(offset) + " bytes", true);


	if (bytesSent < offset) {
		LogE( F("[ERROR] Sent bytes did not match the response length"), true );
		result = false;
	}

#else
		// Send 200
	hapClient->client.print( HTTP_200 );
	hapClient->client.print( HTTP_CONTENT_TYPE_TLV8 );
	hapClient->client.print( HTTP_KEEP_ALIVE );

	// HTTP_HOST
	String hostString = "Host: " + String(_accessorySet->modelName()) + ".local\r\n";
	hapClient->client.print(hostString);

#if HAP_DEBUG
	LogD(">>> Sending " + String(response->size()) + " bytes response to client [" + hapClient->client.remoteIP().toString() + "]", true);
	response->print();
#endif

	if (chunked) {
		hapClient->client.print( HTTP_TRANSFER_ENCODING );
		hapClient->client.write( HTTP_CRLF );

		char chunkSize[10];
		sprintf(chunkSize, "%x%s", (int)response->size(), HTTP_CRLF);
		hapClient->client.write(chunkSize);

		hapClient->client.write( HTTP_CRLF );

	} else {
		hapClient->client.println("Content-Length: " + String(response->size()));
		hapClient->client.write( HTTP_CRLF );
	}

	uint8_t outResponse[response->size()];
	size_t written = 0;

	response->decode(outResponse, &written);
	int bytesSent = hapClient->client.write(outResponse, response->size());
	hapClient->client.write( HTTP_CRLF );
	
	// hapClient->client.flush();
	
	free(outResponse);

	if (bytesSent < response->size()) {
		LogE( F("[ERROR] Sending failed. Sent bytes did not match the expected length"), true );
		result = false;
	}

	if (chunked) {
		hapClient->client.write((uint8_t) 0x30);		// 0x30 is needed
		hapClient->client.write( HTTP_CRLF );		
	}
	hapClient->client.write( HTTP_CRLF );
	
#endif

	response->clear();
	hapClient->request.clear();
	hapClient->clear();
	
#endif



	return result;
}



bool HAPServer::handlePairSetupM1(HAPClient* hapClient){

#if HAP_DEBUG_HEAP        
    LogE("+++++++++++++++++++ " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " start", true);
    HAPLogger::logFreeHeap(0,0, COLOR_MAGENTA);
#endif	

	LogI( F("Homekit PIN: "), false);
	LogI( String(_accessorySet->pinCode()), true);

	LogV( "<<< Handle client [" + hapClient->client.remoteIP().toString() + "] -> /pair-setup Step 1/4 ...", false);	

	_eventManager.queueEvent(EventManager::kEventPairingStep1, HAPEvent());
	
	_isInPairingMode = true;
	TLV8 response;

	// generate keys if not stored	
	if (!isPaired()) {
		_longTermContext = (struct HAPLongTermContext*) calloc(1, sizeof(struct HAPLongTermContext));
		if (_longTermContext == NULL) {
			LogE( F("[ERROR] Initializing struct _longTermContext failed!"), true);
			return false;
		}
	
		LogD("\nGenerating key pairs ...", false);
		_longTermContext->publicKey = (uint8_t*) malloc(sizeof(uint8_t) * ED25519_PUBLIC_KEY_LENGTH);
		_longTermContext->publicKeyLength = ED25519_PUBLIC_KEY_LENGTH;
		_longTermContext->privateKey = (uint8_t*) malloc(sizeof(uint8_t) * ED25519_PRIVATE_KEY_LENGTH);
		_longTermContext->privateKeyLength = ED25519_PRIVATE_KEY_LENGTH;

#if HAP_USE_LIBSODIUM
		crypto_sign_ed25519_keypair(_longTermContext->publicKey, _longTermContext->privateKey);
#else		
		ed25519_key_generate(_longTermContext->publicKey, _longTermContext->privateKey);
#endif
		LogD("OK", true);
	}

#if HAP_DEBUG_HOMEKIT
	HAPHelper::array_print("LTPK", _longTermContext->publicKey, ED25519_PUBLIC_KEY_LENGTH);	
	HAPHelper::array_print("LTSK", _longTermContext->privateKey, ED25519_PRIVATE_KEY_LENGTH);
#endif

 	// _pairings.saveLTPK(_longTermContext->publicKey);
 	// _pairings.saveLTSK(_longTermContext->privateKey);
 	_accessorySet->getPairings()->saveKeys(_longTermContext->publicKey, _longTermContext->privateKey);


 	LogD("Initializing srp ...", false);
	
#if HAP_USE_MBEDTLS_SRP
	memset(_srp, 0, sizeof(Srp));
	_srp->ses = srp_session_new(SRP_SHA512,SRP_NG_3072, NULL,NULL);

#if SRP_TEST
	HAPHelper::mpi_print("N",_srp->ses->ng->N);
	HAPHelper::mpi_print("g",_srp->ses->ng->g);
#endif

#else	

	if (_srp) {
		//LogW("Free SRP!", false);
		srp_cleanup(_srp);
		//LogW("OK", true);
	}

	_srp = srp_init(_accessorySet->pinCode());
	uint8_t host_public_key[SRP_PUBLIC_KEY_LENGTH] = {0,};
	
#endif	
	LogD("OK", true);




#if HAP_USE_MBEDTLS_SRP

	const char* username = "Pair-Setup";
	int unlen = strlen(username) + 1;

	_srp->username = (char*) malloc(sizeof(char) * unlen);
	if (_srp->username == NULL) {
		LogE("Failed to store SRP username", true);
		sendErrorTLV(hapClient, HAP_PAIR_STATE_M2, HAP_ERROR_UNKNOWN);
		return false;
	}
	memcpy(_srp->username, username, unlen);
	

	_srp->len_s = SRP_SALT_LENGTH;
	srp_create_salted_verification_key1(
		_srp->ses,
		username,(const uint8_t*)_accessorySet->pinCode(),strlen(_accessorySet->pinCode()),
		&_srp->bytes_s, _srp->len_s,
		&_srp->bytes_v, &_srp->len_v
	);

	if (_srp->bytes_s==NULL || _srp->bytes_v==NULL) {
		 LogE("Failed to create SRP verifier ", true);
		 sendErrorTLV(hapClient, HAP_PAIR_STATE_M2, HAP_ERROR_UNKNOWN);
		 return false;
	}


	if (_srp->keys == NULL) {
		LogD("Calculating srp public key", true);
		_srp->keys = srp_keypair_new(_srp->ses, _srp->bytes_v, _srp->len_v, &_srp->bytes_B, &_srp->len_B);
		if (_srp->keys == NULL) {
			LogE("Failed to calculate srp public key ", true);
			sendErrorTLV(hapClient, HAP_PAIR_STATE_M2, HAP_ERROR_UNKNOWN);
		
			return false;
		}
	}

	_srpInitialized = true;

	// if (*buffer_size < _srp->len_B) {
    //     *buffer_size = _srp->len_B;
    //     return -2;
    // }
	// if (buffer == NULL) {
	// 	return -4;
	// }
	
    // memcpy(buffer, _srp->bytes_B, _srp->len_B);
    // *buffer_size = _srp->len_B;

	LogD("Sending response ...", false);
	response.encode(HAP_TLV_STATE, 1, HAP_PAIR_STATE_M2);
	response.encode(HAP_TLV_SALT, SRP_SALT_LENGTH, _srp->bytes_s);
	response.encode(HAP_TLV_PUBLIC_KEY, SRP_PUBLIC_KEY_LENGTH, _srp->bytes_B);

#else
		LogD("Generating srp key ...", false);
	if (srp_host_key_get(_srp, host_public_key) < 0) {
		LogE( F("[ERROR] srp_host_key_get failed"), true);
		
		sendErrorTLV(hapClient, HAP_PAIR_STATE_M2, HAP_ERROR_UNKNOWN);
		//return HomekitHelper::pairError(HAP_ERROR_UNKNOWN, acc_msg, acc_msg_length);

		if (_srp) {
			srp_cleanup(_srp);
		}	
		return false;
	}
	LogD("OK", true);

	LogD("Generating salt ...", false);
	uint8_t salt[SRP_SALT_LENGTH] = {0,};
	if (srp_salt(_srp, salt) < 0) {
		LogE( F("[ERROR] srp_salt failed"), true);
		sendErrorTLV(hapClient, HAP_PAIR_STATE_M2, HAP_ERROR_UNKNOWN);

		if (_srp) {
			srp_cleanup(_srp);
		}	
		return false;
	}
	LogD("OK", true);

	LogD("Sending response ...", false);
	response.encode(HAP_TLV_STATE, 1, PAIR_STATE_M2);
	response.encode(HAP_TLV_SALT, SRP_SALT_LENGTH, salt);
	response.encode(HAP_TLV_PUBLIC_KEY, SRP_PUBLIC_KEY_LENGTH, host_public_key);


	
#endif
	
	sendResponse(hapClient, &response);

	LogD("OK", true);


	hapClient->request.clear();
	response.clear();	
	hapClient->clear();	

#if HAP_DEBUG_HEAP            
	LogE("=================== " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " end", true);
    HAPLogger::logFreeHeap(0,0, COLOR_MAGENTA);
#endif	

	LogV("OK", true);
	return true;
}

bool HAPServer::handlePairSetupM3(HAPClient* hapClient) {

#if HAP_DEBUG_HEAP        
    LogE("+++++++++++++++++++ " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " start", true);
    HAPLogger::logFreeHeap(0,0, COLOR_MAGENTA);
#endif	



	LogD( "<<< Handle client [" + hapClient->client.remoteIP().toString() + "] -> /pair-setup Step 2/4 ...", false);

	_eventManager.queueEvent(EventManager::kEventPairingStep3, HAPEvent());

#if HAP_USE_MBEDTLS_SRP
#else
	int err_code = 0;
#endif

	LogV( "\nDecoding TLV ...", false);

	// uint8_t *device_public_key = hapClient->request.tlv.decode(HAP_TLV_PUBLIC_KEY);

	size_t decodedLen = 0;	
	uint8_t device_public_key[hapClient->request.tlv.size(HAP_TLV_PUBLIC_KEY)];

	hapClient->request.tlv.decode(HAP_TLV_PUBLIC_KEY, device_public_key, &decodedLen);

	if (decodedLen == 0) {
		LogE( "ERROR: Invalid payload: no client public key", true);		

		sendErrorTLV(hapClient, HAP_PAIR_STATE_M4, HAP_ERROR_AUTHENTICATON);	
		return false;
	}
	LogV(" OK", true);
	

	LogV( "Generating proof ...", false);
#if HAP_USE_MBEDTLS_SRP

	_srp->ver = srp_verifier_new1(		
		_srp->ses, _srp->username, 0, _srp->bytes_s, _srp->len_s, _srp->bytes_v, _srp->len_v,
		device_public_key, decodedLen,
		NULL, NULL, _srp->keys
	);

	if (_srp->ver == NULL) {
		LogE( "ERROR: srp_verifier_new1 failed", true);		
		sendErrorTLV(hapClient, HAP_PAIR_STATE_M4, HAP_ERROR_AUTHENTICATON);		
        return false;
    }
#else	
	err_code = srp_client_key_set(_srp, device_public_key);
	if (err_code < 0) {
		LogE( F("[ERROR] srp_client_key_set failed"), true);		
		sendErrorTLV(hapClient, HAP_PAIR_STATE_M4, HAP_ERROR_AUTHENTICATON);	
        
        if (_srp) {
			srp_cleanup(_srp);
		}	
        return false;
    }
#endif



    // uint8_t *proof = hapClient->request.tlv.decode(HAP_TLV_PROOF);
    uint8_t proof[hapClient->request.tlv.size(HAP_TLV_PROOF)];
	hapClient->request.tlv.decode(HAP_TLV_PROOF, proof, &decodedLen);

    if (decodedLen == 0) {
    	LogE( "ERROR: Invalid payload: no device proof", true);		    	
		sendErrorTLV(hapClient, HAP_PAIR_STATE_M4, HAP_ERROR_AUTHENTICATON);		
    	return false;
    }
	LogV(" OK", true);


	LogV( "Verifying device proof ...", false);

#if HAP_USE_MBEDTLS_SRP
	if (decodedLen != srp_hash_length(_srp->ses)){
        
		LogE("ERROR: Client SRP proof does not match session hash length", true);
		sendErrorTLV(hapClient, HAP_PAIR_STATE_M4, HAP_ERROR_AUTHENTICATON);			
		return false;
	}


	if (srp_verifier_verify_session(_srp->ver, proof, NULL) == 0) {
        LogE("ERROR: Failed to verify client SRP proof", true);
		
		// Server keys
		// HAPHelper::mpi_print("_srp->keys->B", _srp->keys->B);
		// HAPHelper::mpi_print("_srp->keys->b", _srp->keys->b);

		// HAPHelper::array_print("_srp->bytes_v", _srp->bytes_v, sizeof(_srp->bytes_v));

		// // salt		
		// HAPHelper::array_print("_srp->bytes_s = SALT", (uint8_t*)_srp->bytes_s, SRP_SALT_LENGTH);		
		// HAPHelper::array_print("_srp_bytes_b", (uint8_t*)_srp->bytes_B, SRP_PUBLIC_KEY_LENGTH);		
		
		// proof
		HAPHelper::array_print("_srp->ver->M", _srp->ver->M, SHA512_DIGEST_LENGTH);
		HAPHelper::array_print("proof", proof, SHA512_DIGEST_LENGTH);

		
		// HAPHelper::array_print("username", (uint8_t*)_srp->username, strlen(_srp->username)); 
		// HAPHelper::array_print("password", (uint8_t*)_accessorySet->pinCode(), strlen(_accessorySet->pinCode())); 

		sendErrorTLV(hapClient, HAP_PAIR_STATE_M4, HAP_ERROR_AUTHENTICATON);	
        return false;
	}
#else
    
    err_code = srp_client_proof_verify(_srp, proof);
    if (err_code < 0) {        
        LogE( F("[ERROR] srp_client_proof_verify failed"), true);		    	
        response.encode(HAP_TLV_STATE, 1, PAIR_STATE_M4);
		response.encode(HAP_TLV_ERROR, 1, HAP_ERROR_AUTHENTICATON);
        

		if (_srp) {
			srp_cleanup(_srp);
		}

        sendResponse(hapClient, &response);
		response.clear();
		hapClient->request.clear();	
		hapClient->clear();		
        return false; 
    }
#endif	
    LogV(" OK", true);


    LogV( "Generating accessory proof ...", false);
	

#if HAP_USE_MBEDTLS_SRP
	//int hlen = srp_hash_length(_srp->ses);
	
	// if (*proof_size < hlen){
    //     *proof_size = hlen;
        
	// 	LogE("Failed to verify client SRP proof", true);
	// 	response.encode(HAP_TLV_STATE, 1, PAIR_STATE_M4);
	// 	response.encode(HAP_TLV_ERROR, 1, HAP_ERROR_AUTHENTICATON);
        

	// 	// if (_srp) {
	// 	// 	srp_cleanup(_srp);
	// 	// }

    //     sendResponse(hapClient, &response);
    //     return false;
	// }

	const uint8_t *acc_srp_proof = srp_verifier_get_HAMK(_srp->ver);
	if (acc_srp_proof == NULL) {
		LogE( "ERROR: srp_verifier_get_HAMK failed", true);		    	
		sendErrorTLV(hapClient, HAP_PAIR_STATE_M4, HAP_ERROR_AUTHENTICATON);		
        return false; 
	} 
	
#else
    uint8_t acc_srp_proof[SRP_PROOF_LENGTH] = {0,};
    err_code = srp_host_proof_get(_srp, acc_srp_proof);
    if (err_code < 0) {
        LogE("ERROR: srp_host_proof_get failed", true);		    	
		sendErrorTLV(hapClient, HAP_PAIR_STATE_M4, HAP_ERROR_AUTHENTICATON);	
		if (_srp) {
			srp_cleanup(_srp);
		}

        return false; 
    }
#endif	   
	LogV(" OK", true);

    LogV("Sending response ...", false);
	TLV8 response;
    response.encode(HAP_TLV_STATE, 1, HAP_PAIR_STATE_M4);
    response.encode(HAP_TLV_PROOF, SRP_PROOF_LENGTH, acc_srp_proof);

#if HAP_DEBUG_TLV8
	response.print();
#endif    
	
	sendResponse(hapClient, &response);
	LogV(" OK", true);
	
	hapClient->request.clear();
	response.clear();

	hapClient->clear();		

#if HAP_DEBUG_HEAP            
	LogE("=================== " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " end", true);
    HAPLogger::logFreeHeap(0,0, COLOR_MAGENTA);
#endif


	LogD(" OK", true);	
    return true;
}

bool HAPServer::handlePairSetupM5(HAPClient* hapClient) {

#if HAP_DEBUG_HEAP        
    LogE("+++++++++++++++++++ " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " start", true);
    HAPLogger::logFreeHeap(0,0, COLOR_MAGENTA);
#endif

	LogV( "<<< Handle client [" + hapClient->client.remoteIP().toString() + "] -> /pair-setup Step 3/4 ...", false);

	_eventManager.queueEvent(EventManager::kEventPairingStep3, HAPEvent());


	int err_code = 0;
	TLV8 response;

    uint8_t srp_key[SRP_SESSION_KEY_LENGTH] = {0,};	

#if HAP_USE_MBEDTLS_SRP
	int srp_key_length = 0;
	memcpy(srp_key, srp_verifier_get_session_key(_srp->ver, &srp_key_length), SRP_SESSION_KEY_LENGTH);

	if (srp_key_length == 0) {
		LogE("ERROR: Failed to verify session key!", true);
		
		sendErrorTLV(hapClient, HAP_PAIR_STATE_M6, HAP_ERROR_AUTHENTICATON);		
		response.clear();
		return false;
	}
#else
    srp_host_session_key(_srp, srp_key);
	if (_srp) {
		srp_cleanup(_srp);
	}
#endif   

    LogV( "\nDecoding TLV values ...", false);
	// uint8_t *encrypted_tlv = hapClient->request.tlv.decode(HAP_TLV_ENCRYPTED_DATA);

	size_t decodedLen = 0;
	size_t encryptedTLVLen = hapClient->request.tlv.size(HAP_TLV_ENCRYPTED_DATA);

	uint8_t encryptedTLV[encryptedTLVLen];
	hapClient->request.tlv.decode(HAP_TLV_ENCRYPTED_DATA, encryptedTLV, &decodedLen);
	

	if (decodedLen == 0) {
        LogE( "ERROR: Decrypting TLV failed", true);		    	
		sendErrorTLV(hapClient, HAP_PAIR_STATE_M6, HAP_ERROR_AUTHENTICATON);			     
		response.clear();
    	return false;
    }
    LogV( "OK", true);

    LogV("Decrypting chacha20_poly1305 ...", false);		
	uint8_t subtlv_key[HKDF_KEY_LEN] = {0,};
	hkdf_key_get(HKDF_KEY_TYPE_PAIR_SETUP_ENCRYPT, srp_key, SRP_SESSION_KEY_LENGTH, subtlv_key);

	if (err_code < 0) {
        LogE("ERROR: Failed to get HKDF key", true);		    	        
		sendErrorTLV(hapClient, HAP_PAIR_STATE_M6, HAP_ERROR_AUTHENTICATON);				
		response.clear();
        return false;
    }
    LogV( "OK", true);

    LogV( "Decrypting chacha20_poly1305 ...", false);
    // uint8_t *subtlv = (uint8_t*) malloc(sizeof(uint8_t) * encrypted_tlv_len);
    uint8_t subtlv[encryptedTLVLen];


	err_code = chacha20_poly1305_decrypt(CHACHA20_POLY1305_TYPE_PS05, subtlv_key, NULL, 0, encryptedTLV, encryptedTLVLen, subtlv);

    if (err_code < 0) {
        LogE("ERROR: Decrypting CHACHA20_POLY1305_TYPE_PS05 failed", true);		    	
		sendErrorTLV(hapClient, HAP_PAIR_STATE_M6, HAP_ERROR_AUTHENTICATON);
		response.clear();
        return false;
    }
    LogD( F("OK"), true);

    
	TLV8 encTLV; 
	encTLV.encode(subtlv, strlen((char*)subtlv));
	
#if HAP_DEBUG	
	encTLV.print();
#endif
	
    uint8_t ios_devicex[HKDF_KEY_LEN];
    hkdf_key_get(HKDF_KEY_TYPE_PAIR_SETUP_CONTROLLER, srp_key, SRP_SESSION_KEY_LENGTH, ios_devicex);


	
    uint8_t ios_device_pairing_id_len 	= encTLV.size(HAP_TLV_IDENTIFIER);
	uint8_t ios_device_pairing_id[ios_device_pairing_id_len];
	encTLV.decode(HAP_TLV_IDENTIFIER, ios_device_pairing_id, &decodedLen);	

	if (decodedLen == 0) {
		LogE( "ERROR: TLV decoding identifier failed", true);		    	
		sendErrorTLV(hapClient, HAP_PAIR_STATE_M6, HAP_ERROR_AUTHENTICATON);
		encTLV.clear();
		response.clear();		
		return false;
	}


	uint8_t  ios_device_ltpk_len 		= encTLV.size(HAP_TLV_PUBLIC_KEY);
    // uint8_t* ios_device_ltpk 			= encTLV.decode(HAP_TLV_PUBLIC_KEY);
	
	uint8_t ios_device_ltpk[ios_device_ltpk_len];
	
	encTLV.decode(HAP_TLV_PUBLIC_KEY, ios_device_ltpk, &decodedLen);	

	if (decodedLen == 0) {
		LogE( "ERROR: TLV decoding public key failed", true);		    	
		sendErrorTLV(hapClient, HAP_PAIR_STATE_M6, HAP_ERROR_AUTHENTICATON);
		encTLV.clear();
		response.clear();		
		return false;
	}
    

    uint8_t  ios_device_signature_len 	= encTLV.size(HAP_TLV_SIGNATURE);
    // uint8_t* ios_device_signature 		= encTLV.decode(HAP_TLV_SIGNATURE);
	uint8_t ios_device_signature[ios_device_signature_len];
	encTLV.decode(HAP_TLV_SIGNATURE, ios_device_signature, &decodedLen);	

	if (decodedLen == 0) {
		LogE( "ERROR: TLV decoding signature failed", true);		    	
		sendErrorTLV(hapClient, HAP_PAIR_STATE_M6, HAP_ERROR_AUTHENTICATON);
		encTLV.clear();
		response.clear();		
		return false;
	}


    int ios_device_info_len = 0;
    uint8_t* ios_device_info = concat3(ios_devicex, sizeof(ios_devicex), 
            ios_device_pairing_id, ios_device_pairing_id_len, 
            ios_device_ltpk, ios_device_ltpk_len,
            &ios_device_info_len);
    

    LogV( "Verifying ED25519 ...", false);
#if HAP_USE_LIBSODIUM	
	int verified = crypto_sign_verify_detached(ios_device_signature,
                                ios_device_info,
                                ios_device_info_len,
                                ios_device_ltpk);
#else
	int verified = ed25519_verify(ios_device_ltpk, ios_device_ltpk_len,
            ios_device_signature, ios_device_signature_len,
            ios_device_info, ios_device_info_len);
#endif		

	
    concat_free(ios_device_info);

	if (verified < 0) {
        LogE( F("[ERROR] Verification failed"), true);		    	
		sendErrorTLV(hapClient, HAP_PAIR_STATE_M6, HAP_ERROR_AUTHENTICATON);
		encTLV.clear();
		response.clear();		
        return false;
	}
	LogV( " OK", true);


	// ToDo: When is admin set?
	// Save to Pairings as admin
	LogD( F("Saving pairing ..."), false);
	_accessorySet->getPairings()->add(ios_device_pairing_id, ios_device_ltpk, true);
	_accessorySet->getPairings()->save();		
	LogV( " OK", true);
	
	encTLV.clear();
	// delete enc_tlv;


	LogV( "<<< Handle [" + hapClient->client.remoteIP().toString() + "] -> /pair-setup Step 4/4 ...", true);
	_eventManager.queueEvent(EventManager::kEventPairingStep4, HAPEvent());


	//  _acc_m6_subtlv(srp_key, ps->acc_id, ps->keys.public, ps->keys.private, &acc_subtlv, &acc_subtlv_length);
	uint8_t accessoryx[HKDF_KEY_LEN] = {0,};
	hkdf_key_get(HKDF_KEY_TYPE_PAIR_SETUP_ACCESSORY, srp_key, SRP_SESSION_KEY_LENGTH, 
            accessoryx);

	
    int acc_info_len = 0;
    uint8_t* acc_info = concat3(accessoryx, sizeof(accessoryx), 
            (uint8_t*)HAPDeviceID::deviceID().c_str(), 17, 
            _longTermContext->publicKey, ED25519_PUBLIC_KEY_LENGTH, &acc_info_len);

    LogV( F("\nVerifying signature ..."), false);	
    uint8_t acc_signature[ED25519_SIGN_LENGTH] = {0,};

#if HAP_USE_LIBSODIUM

	long long unsigned int acc_signature_length = ED25519_SIGN_LENGTH;
	if (crypto_sign_detached(acc_signature, &acc_signature_length, acc_info, acc_info_len, _longTermContext->privateKey) != 0) {
		LogE( "ERROR: Signing failed", true);
		sendErrorTLV(hapClient, HAP_PAIR_STATE_M6, HAP_ERROR_AUTHENTICATON);
		response.clear();		
		return false;
	}

#else
	int acc_signature_length = ED25519_SIGN_LENGTH;
	
	err_code = ed25519_sign(_longTermContext->publicKey, _longTermContext->privateKey, acc_info, acc_info_len,
            acc_signature, &acc_signature_length);

	if (err_code < 0){
		LogE( "ERROR: Signing failed", true);
		sendErrorTLV(hapClient, HAP_PAIR_STATE_M6, HAP_ERROR_AUTHENTICATON);
		response.clear();

		concat_free(acc_info);
		

		return false;
	}
#endif	
    


    concat_free(acc_info);

    if (err_code != 0) {
        LogE("ERROR: Verify signature failed", true);		    	
		sendErrorTLV(hapClient, HAP_PAIR_STATE_M6, HAP_ERROR_AUTHENTICATON);
		response.clear();
        return false;
	}
	LogV( " OK", true);

	// Encrypt data
	TLV8 subTLV;
	subTLV.encode(HAP_TLV_IDENTIFIER, 17, (uint8_t*)HAPDeviceID::deviceID().c_str()  );
	subTLV.encode(HAP_TLV_PUBLIC_KEY, ED25519_PUBLIC_KEY_LENGTH, _longTermContext->publicKey);
	subTLV.encode(HAP_TLV_SIGNATURE, ED25519_SIGN_LENGTH, acc_signature);


	size_t tlv8Len = subTLV.size();
	uint8_t tlv8Data[tlv8Len];
	size_t written = 0;

	subTLV.decode(tlv8Data, &written);
	if (written == 0) {
		LogE("ERROR: Failed to decode subtlv8!", true);
		sendErrorTLV(hapClient, HAP_PAIR_STATE_M6, HAP_ERROR_AUTHENTICATON);
		response.clear();
		subTLV.clear();
		return false;
	}
	
	

#if HAP_DEBUG_TLV8
	subTLV.print();
	//HAPHelper::arrayPrint(tlv8Data, tlv8Len);
#endif
		

	uint8_t encryptedData[tlv8Len + CHACHA20_POLY1305_AUTH_TAG_LENGTH];

	LogV( F("Getting session key ..."), false);
	err_code = hkdf_key_get(HKDF_KEY_TYPE_PAIR_SETUP_ENCRYPT, srp_key, SRP_SESSION_KEY_LENGTH, subtlv_key);
	if (err_code != 0) {
        LogE( "ERROR: Getting session key failed", true);		    	
		sendErrorTLV(hapClient, HAP_PAIR_STATE_M6, HAP_ERROR_AUTHENTICATON);
		response.clear();
		subTLV.clear();		
        return false;
	}
	LogV( " OK", true);


	LogV( "Encrypting Data ...", false);	
	err_code = chacha20_poly1305_encrypt(CHACHA20_POLY1305_TYPE_PS06, subtlv_key, NULL, 0, tlv8Data, tlv8Len, encryptedData);	

	if (err_code != 0) {
        LogE( F("[ERROR] Verify signature failed"), true);		    	
		sendErrorTLV(hapClient, HAP_PAIR_STATE_M6, HAP_ERROR_AUTHENTICATON);
		response.clear();
		subTLV.clear();
		
        return false;
	}
	LogV( " OK", true);


	LogV( "Sending response ...", false);
	response.encode(HAP_TLV_STATE, 1, HAP_PAIR_STATE_M6);
	response.encode(HAP_TLV_ENCRYPTED_DATA, tlv8Len + CHACHA20_POLY1305_AUTH_TAG_LENGTH, encryptedData);

#if HAP_DEBUG_TLV8
	response.print();
#endif

	sendResponse(hapClient, &response);	
	LogV( " OK", true);

	response.clear();
	subTLV.clear();
	// delete subTLV;


	LogV( "Updating mDNS ...", false);
	updateServiceTxt();
	LogV( " OK", true);
	
	LogI(">>> Pairing with client [" + hapClient->client.remoteIP().toString() + "] complete!", true);	
	
	// ToDo: set timeout for resetting to false automatically?
	_isInPairingMode = false;

	_eventManager.queueEvent(EventManager::kEventPairingComplete, HAPEvent());


	hapClient->request.clear();
	hapClient->clear();		

#if HAP_DEBUG_HEAP            
	LogE("=================== " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " end", true);
    HAPLogger::logFreeHeap(0,0, COLOR_MAGENTA);
#endif

	//stopEvents(false);
    return true;
}

/**
 * handlePairVerifyM1
 * checked with heap trace: 
 * 6711 bytes 'leaked' in trace (24 allocations)
 *	total allocations 137 total frees 130
 * 
 */
bool HAPServer::handlePairVerifyM1(HAPClient* hapClient){


	LogV( "<<< Handle client [" + hapClient->client.remoteIP().toString() + "] -> /pair-verify Step 1/2 ...", false);

	_eventManager.queueEvent(EventManager::kEventVerifyStep1, HAPEvent());
	
	int err_code = 0;
	


	if ( !isPaired() ) {
		LogW( F("\n[WARNING] Attempt to verify unpaired accessory!"), true);        
		sendErrorTLV(hapClient, HAP_VERIFY_STATE_M2, HAP_ERROR_UNKNOWN);
		
		return false;
	}

	hapClient->encryptionContext.decryptCount = 0;
	hapClient->encryptionContext.encryptCount = 0;

	LogD("\nGenerating accessory curve25519 keys ...", false);

#if HAP_USE_MBEDTLS_CURVE25519

	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_ecdh_context ctx_srv;

	uint8_t acc_curve_public_key[CURVE25519_KEY_LENGTH]= {0,};
	uint8_t acc_curve_private_key[CURVE25519_KEY_LENGTH]= {0,};

	if (m_curve25519_key_generate(ctx_srv, ctr_drbg, acc_curve_public_key, acc_curve_private_key) != 0) {
		LogE( F("[ERROR] crypto_sign_keypair failed"), true);
		response.encode(HAP_TLV_STATE, 1, VERIFY_STATE_M2);
		response.encode(HAP_TLV_ERROR, 1, HAP_ERROR_UNKNOWN);
        
        sendResponse(hapClient, &response);
		response.clear();
		hapClient->request.clear();	
		hapClient->clear();
		return false;
	}
#elif HAP_USE_LIBSODIUM

	uint8_t acc_curve_public_key[CURVE25519_KEY_LENGTH]= {0,};
	uint8_t acc_curve_private_key[CURVE25519_KEY_LENGTH]= {0,};

	// if (crypto_box_keypair(acc_curve_public_key, acc_curve_private_key) != 0){
	// 	LogE( F("[ERROR] crypto_sign_keypair failed"), true);
	// 	response.encode(HAP_TLV_STATE, 1, VERIFY_STATE_M2);
	// 	response.encode(HAP_TLV_ERROR, 1, HAP_ERROR_UNKNOWN);
        
    //     sendResponse(hapClient, &response);
	// 	return false;
	// }

	/* Create server's secret and public keys */
	randombytes_buf(acc_curve_private_key, crypto_scalarmult_SCALARBYTES);
	if (crypto_scalarmult_base(acc_curve_public_key, acc_curve_private_key) != 0) {
		LogE( F("[ERROR] crypto_sign_keypair failed"), true);
		sendErrorTLV(hapClient, HAP_VERIFY_STATE_M2, HAP_ERROR_UNKNOWN);
		return false;
	}

#else
	uint8_t acc_curve_public_key[CURVE25519_KEY_LENGTH] = {0,};		// my_key_public
	uint8_t acc_curve_private_key[CURVE25519_KEY_LENGTH] = {0,};	// my_key	

	if (curve25519_key_generate(acc_curve_public_key, acc_curve_private_key) < 0) {
		LogE( F("[ERROR] curve25519_key_generate failed"), true);
		sendErrorTLV(hapClient, HAP_VERIFY_STATE_M2, HAP_ERROR_UNKNOWN);
		return false;
	}
#endif	
	LogD( F("OK"), true);

	
	uint8_t ios_device_curve_key_len = hapClient->request.tlv.size(HAP_TLV_PUBLIC_KEY);
	// uint8_t *ios_device_curve_key = hapClient->request.tlv.decode(HAP_TLV_PUBLIC_KEY); 	// device_key

	size_t decodedLen = 0;

	uint8_t ios_device_curve_key[ios_device_curve_key_len];
	hapClient->request.tlv.decode(HAP_TLV_PUBLIC_KEY, ios_device_curve_key, &decodedLen);	


	
	if (decodedLen == 0) {
		LogE( F("[PAIR-VERIFY M1] [ERROR] HAP_TLV_ENCRYPTED_DATA failed "), true);		    	
		sendErrorTLV(hapClient, HAP_VERIFY_STATE_M2, HAP_ERROR_AUTHENTICATON);
		return false;
	}
	
#if HAP_DEBUG_HOMEKIT
	
	HAPHelper::array_print("acc_curve_public_key", acc_curve_public_key, CURVE25519_KEY_LENGTH);
	HAPHelper::array_print("acc_curve_private_key", acc_curve_private_key, HAP_USE_MBEDTLS_CURVE25519);
	HAPHelper::array_print("ios_device_curve_key", ios_device_curve_key, ios_device_curve_key_len);

#endif

	// shared_secret
	LogD("Generating Curve25519 shared secret ...", false);
	uint8_t sharedSecret[CURVE25519_SECRET_LENGTH] = {0,};	
	

#if HAP_USE_MBEDTLS_CURVE25519

	if (m_curve25519_shared_secret(ctx_srv, ctr_drbg, ios_device_curve_key, sharedSecret, &sharedSecretLength) < 0) {
		LogE( F("[ERROR] curve25519_shared_secret failed"), true);

		sendErrorTLV(hapClient, HAP_VERIFY_STATE_M2, HAP_ERROR_AUTHENTICATON);
		return false;
	}

#elif HAP_USE_LIBSODIUM
	/* The server derives a shared key from its secret key and the client's public key */
	/* shared key = h(q ‖ client_publickey ‖ server_publickey) */
	if (crypto_scalarmult(sharedSecret, acc_curve_private_key, ios_device_curve_key) != 0) {
    	LogE( F("[ERROR] crypto_scalarmult failed"), true);
		sendErrorTLV(hapClient, HAP_VERIFY_STATE_M2, HAP_ERROR_AUTHENTICATON);
		return false;
	}

#else
	int sharedSecretLength = CURVE25519_SECRET_LENGTH;	
	
	if (curve25519_shared_secret(ios_device_curve_key, acc_curve_private_key, sharedSecret, &sharedSecretLength) < 0) {
		LogE( F("[ERROR] curve25519_shared_secret failed"), true);

		sendErrorTLV(hapClient, HAP_VERIFY_STATE_M2, HAP_ERROR_AUTHENTICATON);
		return false;
	}	

#endif


	LogD( F("OK"), true);

	LogD("Generating signature ...", false);
	int acc_info_len;
	uint8_t* acc_info = concat3(acc_curve_public_key, CURVE25519_KEY_LENGTH,
		(uint8_t*)HAPDeviceID::deviceID().c_str(), 17,
		ios_device_curve_key, ios_device_curve_key_len,
		&acc_info_len);


	uint8_t acc_signature[ED25519_SIGN_LENGTH] = {0,};

#if HAP_USE_LIBSODIUM

	long long unsigned int acc_signature_length = ED25519_SIGN_LENGTH;
	if (crypto_sign_detached(acc_signature, &acc_signature_length, acc_info, acc_info_len, _longTermContext->privateKey) != 0) {
		LogE( F("[ERROR] crypto_sign_detached failed"), true);
		sendErrorTLV(hapClient, HAP_VERIFY_STATE_M2, HAP_ERROR_AUTHENTICATON);
		concat_free(acc_info);
		return false;
	}

#else
	int acc_signature_length = ED25519_SIGN_LENGTH;
	
	err_code = ed25519_sign(_longTermContext->publicKey, _longTermContext->privateKey, 
		acc_info, acc_info_len,
		acc_signature, &acc_signature_length);

	if (err_code < 0){
		LogE( F("[ERROR] ed25519_sign failed"), true);
		sendErrorTLV(hapClient, HAP_VERIFY_STATE_M2, HAP_ERROR_AUTHENTICATON);
		return false;
	}
#endif

	concat_free(acc_info);
	LogD( F("OK"), true);

	
	// Encrypt data

	LogD( F("Encoding into TLV ..."), false);
	TLV8 *subTLV = new TLV8();
	subTLV->encode(HAP_TLV_IDENTIFIER, 17, (uint8_t*)HAPDeviceID::deviceID().c_str()  );
	subTLV->encode(HAP_TLV_SIGNATURE, ED25519_SIGN_LENGTH, acc_signature);

	size_t tlv8Len = subTLV->size();
	uint8_t tlv8Data[tlv8Len];
	size_t written = 0;

	subTLV->decode(tlv8Data, &written);
	if (written == 0) {
		LogE("[ERROR] Failed to decode subtlv8!", true);
		sendErrorTLV(hapClient, HAP_VERIFY_STATE_M2, HAP_ERROR_AUTHENTICATON);
		
		subTLV->clear();
		delete subTLV;

		return false;
	}

#if HAP_DEBUG_TLV8
	subTLV->print();
	//HAPHelper::arrayPrint(tlv8Data, tlv8Len);
#endif
	LogD( F("OK"), true);


	LogD("Generating proof ...", false);	
    uint8_t sessionKey[HKDF_KEY_LEN] = {0,};   		// session_key 
    err_code = hkdf_key_get(HKDF_KEY_TYPE_PAIR_VERIFY_ENCRYPT, sharedSecret, CURVE25519_SECRET_LENGTH, sessionKey);
	if (err_code != 0) {
        LogE( F("[ERROR] Verify signature failed"), true);		    	
        sendErrorTLV(hapClient, HAP_VERIFY_STATE_M2, HAP_ERROR_AUTHENTICATON);
		
		subTLV->clear();
		delete subTLV;
	
        return false;
	}
	LogD( F("OK"), true);



	LogD( F("Encrypting data ..."), false);
	// uint8_t* encryptedData;
	// encryptedData = (uint8_t*)malloc(sizeof(uint8_t) * (tlv8Len + CHACHA20_POLY1305_AUTH_TAG_LENGTH));
	uint8_t encryptedData[tlv8Len + CHACHA20_POLY1305_AUTH_TAG_LENGTH];

	// if (!encryptedData){
	// 	LogE( F("[ERROR] Malloc of encryptedData failed"), true);		    	
	// 	response.encode(HAP_TLV_STATE, 1, VERIFY_STATE_M2);
	// 	response.encode(HAP_TLV_ERROR, 1, HAP_ERROR_UNKNOWN);
        
 //        sendResponse(hapClient, &response);
 //        return false;
	// }


	err_code = chacha20_poly1305_encrypt(CHACHA20_POLY1305_TYPE_PV02, sessionKey, NULL, 0, tlv8Data, tlv8Len, encryptedData);


	if (err_code != 0) {
        LogE( F("[ERROR] Verify signature failed"), true);		    	
        sendErrorTLV(hapClient, HAP_VERIFY_STATE_M2, HAP_ERROR_AUTHENTICATON);


		subTLV->clear();
		delete subTLV;

        return false;
	}
	LogD( F("OK"), true);

	LogD("Saving context ...", false);

	//hapClient->verifyContext = new struct HAPVerifyContext;	
	memcpy(hapClient->verifyContext.secret, sharedSecret, HKDF_KEY_LEN);
	memcpy(hapClient->verifyContext.sessionKey, sharedSecret, CURVE25519_SECRET_LENGTH);
	memcpy(hapClient->verifyContext.accessoryLTPK, acc_curve_public_key, ED25519_PUBLIC_KEY_LENGTH);
	memcpy(hapClient->verifyContext.deviceLTPK, ios_device_curve_key, ED25519_PUBLIC_KEY_LENGTH);
	LogD( F("OK"), true);
    
	LogD( F("Sending response ..."), false);
	TLV8 response;
	response.encode(HAP_TLV_STATE, 1, HAP_VERIFY_STATE_M2);
	response.encode(HAP_TLV_PUBLIC_KEY, CURVE25519_KEY_LENGTH, acc_curve_public_key);
	response.encode(HAP_TLV_ENCRYPTED_DATA, tlv8Len + CHACHA20_POLY1305_AUTH_TAG_LENGTH, encryptedData);

	//memcpy(_pairSetup->sessionKey, sharedSecret, CURVE25519_SECRET_LENGTH);

#if HAP_DEBUG_TLV8
	response.print();
	//HAPHelper::arrayPrint(encryptedData, tlv8Len + CHACHA20_POLY1305_AUTH_TAG_LENGTH);
#endif

	sendResponse(hapClient, &response);	
	LogD( F("OK"), true);
	response.clear();
	
	subTLV->clear();
	delete subTLV;
	
	hapClient->request.clear();	
	hapClient->clear();	

	//stopEvents(false);

	LogI("OK", true);

	return true;
}


bool HAPServer::handlePairVerifyM3(HAPClient* hapClient){

#if HAP_DEBUG_HEAP        
    LogE("+++++++++++++++++++ " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " start", true);
    HAPLogger::logFreeHeap(0,0, COLOR_MAGENTA);
#endif

	LogV( "<<< Handle client [" + hapClient->client.remoteIP().toString() + "] -> /pair-verify Step 2/2 ...", false);
	_eventManager.queueEvent(EventManager::kEventVerifyStep2, HAPEvent());

	int err_code = 0;
	


	
	int encryptedDataLen = hapClient->request.tlv.size(HAP_TLV_ENCRYPTED_DATA);
	size_t decodedLen = 0;

	uint8_t encryptedData[encryptedDataLen];
	hapClient->request.tlv.decode(HAP_TLV_ENCRYPTED_DATA, encryptedData, &decodedLen);
	

	if (decodedLen == 0) {
		LogE( F("[PAIR-VERIFY M3] [ERROR] HAP_TLV_ENCRYPTED_DATA failed "), true);		    	

		sendErrorTLV(hapClient, HAP_VERIFY_STATE_M4, HAP_ERROR_AUTHENTICATON);
		return false;
	}



	LogD("\nGenerating decrpytion key ...", true);
	uint8_t subtlv_key[HKDF_KEY_LEN] = {0,};
	err_code = hkdf_key_get(HKDF_KEY_TYPE_PAIR_VERIFY_ENCRYPT, hapClient->verifyContext.secret, CURVE25519_SECRET_LENGTH, subtlv_key);
	if (err_code != 0) {
		LogE( F("[PAIR-VERIFY M3] [ERROR] hkdf_key_get failed"), true);		    	
		sendErrorTLV(hapClient, HAP_VERIFY_STATE_M4, HAP_ERROR_AUTHENTICATON);
		return false;
	}


	LogD("Decrypting data ...", true);
	
	uint8_t subtlvData[encryptedDataLen];

	err_code = chacha20_poly1305_decrypt(CHACHA20_POLY1305_TYPE_PV03, subtlv_key, NULL, 0, encryptedData, encryptedDataLen, subtlvData);


	if (err_code != 0) {
		LogE( F("[ERROR] Decrypting failed"), true);		    	
		sendErrorTLV(hapClient, HAP_VERIFY_STATE_M4, HAP_ERROR_AUTHENTICATON);
		return false;
	}	

	TLV8 subTlv;
	subTlv.encode(subtlvData, encryptedDataLen);

// #if HAP_DEBUG
// 	LogD("subTLV: ", true);
// 	subTlv.print();
// #endif


	uint8_t ios_device_pairing_id_len 	= subTlv.size(HAP_TLV_IDENTIFIER);
	// uint8_t *ios_device_pairing_id 		= subTlv.decode(HAP_TLV_IDENTIFIER);
	uint8_t ios_device_pairing_id[ios_device_pairing_id_len];
	subTlv.decode(HAP_TLV_IDENTIFIER, ios_device_pairing_id, &decodedLen);	

	if (decodedLen == 0) {

		LogE( "ERROR: HAP_TLV_IDENTIFIER failed ", true);		    	
		sendErrorTLV(hapClient, HAP_VERIFY_STATE_M4, HAP_ERROR_AUTHENTICATON);
		subTlv.clear();

		return false;
	}



	// LogD("iOS device_pairing_id: ", true);
	// HAPHelper::arrayPrint(ios_device_pairing_id, ios_device_pairing_id_len);	


#if HAP_DEBUG_HOMEKIT
	LogD("Looking up iOS device LTPK for client: ", true);
	HAPHelper::arrayPrint(ios_device_pairing_id, ios_device_pairing_id_len);	
#endif

	uint8_t ios_device_ltpk[ED25519_PUBLIC_KEY_LENGTH];
	err_code = _accessorySet->getPairings()->getKey(ios_device_pairing_id, ios_device_ltpk);
	
	if (err_code == -1) {
		LogE( F("[ERROR] No iOS Device LTPK found!"), true);		    	
		sendErrorTLV(hapClient, HAP_VERIFY_STATE_M4, HAP_ERROR_AUTHENTICATON);
		subTlv.clear();
		return false;
	}
		
#if HAP_DEBUG_HOMEKIT
	LogD("Found LTPK: ", true);	
	HAPHelper::arrayPrint(ios_device_ltpk, ED25519_PUBLIC_KEY_LENGTH);
#endif

	
	uint8_t ios_device_signature_len = subTlv.size(HAP_TLV_SIGNATURE);
	

	
	uint8_t ios_device_signature[ios_device_signature_len];
	subTlv.decode(HAP_TLV_SIGNATURE, ios_device_signature, &decodedLen);	

	if (decodedLen == 0) {
		LogE( F("[PAIR-VERIFY M3] [ERROR] HAP_TLV_ENCRYPTED_DATA failed "), true);		    	
		sendErrorTLV(hapClient, HAP_VERIFY_STATE_M4, HAP_ERROR_AUTHENTICATON);
		subTlv.clear();
		return false;
	}

#if HAP_DEBUG_HOMEKIT
	LogD("Found Signature: ", true);
	HAPHelper::arrayPrint(ios_device_signature, ios_device_signature_len);
#endif

	int ios_device_info_len = 0;
    uint8_t* ios_device_info = concat3(hapClient->verifyContext.deviceLTPK, HKDF_KEY_LEN, 
            ios_device_pairing_id, ios_device_pairing_id_len, 
            hapClient->verifyContext.accessoryLTPK, ED25519_PUBLIC_KEY_LENGTH, &ios_device_info_len);



    LogD( F("Verifying Signature ..."), false);

#if HAP_USE_LIBSODIUM	
	int verified = crypto_sign_verify_detached(ios_device_signature,
                                ios_device_info,
                                ios_device_info_len,
                                ios_device_ltpk);
#else
	int verified = ed25519_verify(ios_device_ltpk, ED25519_PUBLIC_KEY_LENGTH,
            ios_device_signature, ios_device_signature_len,
            ios_device_info, ios_device_info_len);
#endif	
    concat_free(ios_device_info);

	if (verified < 0) {
        LogE( F("[ERROR] Signature verification failed"), true);		    	
        sendErrorTLV(hapClient, HAP_VERIFY_STATE_M4, HAP_ERROR_AUTHENTICATON);
		subTlv.clear();
        return false;
	}

	LogD( F("OK"), true);
    

	
    err_code = hkdf_key_get(HKDF_KEY_TYPE_CONTROL_READ, hapClient->verifyContext.secret, CURVE25519_SECRET_LENGTH, hapClient->encryptionContext.encryptKey);
	if (err_code != 0) {
		LogE( F("[ERROR] HKDF encrpytion key not available!"), true);		    	
		sendErrorTLV(hapClient, HAP_VERIFY_STATE_M4, HAP_ERROR_AUTHENTICATON);
		subTlv.clear();
		return false;
	}


	err_code = hkdf_key_get(HKDF_KEY_TYPE_CONTROL_WRITE, hapClient->verifyContext.secret, CURVE25519_SECRET_LENGTH, hapClient->encryptionContext.decryptKey);
	if (err_code != 0) {
		LogE( F("[ERROR] HKDF decryption key not available!"), true);		    	
		sendErrorTLV(hapClient, HAP_VERIFY_STATE_M4, HAP_ERROR_AUTHENTICATON);
		subTlv.clear();
		return false;
	}

	// ToDo: FREE CONTEXT ??

	LogD( F("Sending response ..."), true);
	TLV8 response;
	response.encode(HAP_TLV_STATE, 1, HAP_VERIFY_STATE_M4);

#if HAP_DEBUG
	response.print();
#endif

	sendResponse(hapClient, &response);	

	subTlv.clear();
	response.clear();
	hapClient->request.clear();
	hapClient->clear();

	// following messages from this client will be encrypted
	hapClient->setEncryped(true);
	hapClient->setId(ios_device_pairing_id);
	hapClient->setAdmin(_accessorySet->getPairings()->isAdmin(ios_device_pairing_id));
	
	LogV("OK", true);
	LogI(">>> Verification with client [" + hapClient->client.remoteIP().toString() + "] complete!", true);

	_eventManager.queueEvent(EventManager::kEventVerifyComplete, HAPEvent());

#if HAP_DEBUG_HEAP            
	LogE("=================== " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " end", true);
    HAPLogger::logFreeHeap(0,0, COLOR_MAGENTA);
#endif
	
	return true;
}


void HAPServer::handleAccessories(HAPClient* hapClient) {

#if HAP_DEBUG_HEAP        
    LogE("+++++++++++++++++++ " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " start", true);
    HAPLogger::logFreeHeap(0,0, COLOR_MAGENTA);
#endif


	LogV( "<<< Handle [" + hapClient->client.remoteIP().toString() + "] -> /accessories ...", false);
	

	// hapClient->setHeader("Content-Type", "application/hap+json");
	// hapClient->setHeader("Host", _accessorySet->modelName());
	// hapClient->setHeader("Connection", "keep-alive");

	// // ToDo: Handle here
	// DynamicJsonDocument doc(3192);
	// deserializeJson(doc, _accessorySet->describe());
	
	// // hapClient->write( (uint8_t*)_accessorySet->describe().c_str(), _accessorySet->describe().length());
	
	// serializeJson(doc, *hapClient);	
	sendEncrypt(hapClient, HTTP_200, _accessorySet->describe(), true);	

	LogV("OK", true);
	hapClient->state = HAP_CLIENT_STATE_IDLE;
	
	hapClient->request.clear();
	hapClient->clear();
	
#if HAP_DEBUG_HEAP    
    LogE("=================== " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " end", true);
    HAPLogger::logFreeHeap(0,0, COLOR_MAGENTA);
#endif
}


void HAPServer::handlePairingsList(HAPClient* hapClient){
	LogV( "<<< Handle client [" + hapClient->client.remoteIP().toString() + "] -> POST /pairings list ...", false);

	if (hapClient->isAdmin() == false){
		LogE("ERROR: Non-Admin controllers are not allowed to call this method!", true);
		sendErrorTLV(hapClient, HAP_PAIR_STATE_M2, HAP_ERROR_AUTHENTICATON);
		return;
	}
	
	TLV8 response;
	response.encode(HAP_TLV_STATE, 1, HAP_PAIR_STATE_M2);

	for (int i=0; i<_accessorySet->getPairings()->size(); i++){

#if HAP_HOMEKIT_PYTHON_COMPATIBLE == 0
		if (i > 0) {
			response.addSeperator();
		}
#endif

		response.encode(HAP_TLV_IDENTIFIER, HAP_PAIRINGS_ID_LENGTH, _accessorySet->getPairings()->pairings[i].id);
		response.encode(HAP_TLV_PUBLIC_KEY, HAP_PAIRINGS_LTPK_LENGTH, _accessorySet->getPairings()->pairings[i].key);
		response.encode(HAP_TLV_PERMISSIONS, 1, _accessorySet->getPairings()->pairings[i].isAdmin);
	}

#if HAP_DEBUG_TLV8	
	response.print();
#endif

	//sendResponse(hapClient, &response);
	uint8_t data[response.size()];
	size_t length = 0;

	response.decode(data, &length);
	sendEncrypt(hapClient, HTTP_200, data, length, true, "application/pairing+tlv8");

	response.clear();
	hapClient->request.clear();
	hapClient->clear();

	hapClient->state = HAP_CLIENT_STATE_IDLE;

	LogV("OK", true);
}


void HAPServer::handlePairingsAdd(HAPClient* hapClient, const uint8_t* identifier, const uint8_t* publicKey, bool isAdmin){
	LogV( "<<< Handle client [" + hapClient->client.remoteIP().toString() + "] -> POST /pairings add ...", false);	


	if (hapClient->isAdmin() == false){
		LogE("ERROR: Non-Admin controllers are not allowed to call this method!", true);
		sendErrorTLV(hapClient, HAP_PAIR_STATE_M2, HAP_ERROR_AUTHENTICATON);
		return;
	}

	TLV8 response;
	response.encode(HAP_TLV_STATE, 1, HAP_PAIR_STATE_M2);

	if (_accessorySet->getPairings()->size() >= HAP_PAIRINGS_MAX) {		
		response.encode(HAP_TLV_ERROR, 1, HAP_ERROR_MAX_PEERS);
	} else {

		bool result = _accessorySet->getPairings()->add(identifier, publicKey, isAdmin);
		
		if (!_accessorySet->getPairings()->save() || (result == false) ) {
			response.encode(HAP_TLV_ERROR, 1, HAP_ERROR_UNKNOWN);
		}
	}

#if HAP_DEBUG_TLV8	
	response.print();
#endif

	// sendResponse(hapClient, &response);
	uint8_t data[response.size()];
	size_t length = 0;

	response.decode(data, &length);
	sendEncrypt(hapClient, HTTP_200, data, length, true, "application/pairing+tlv8");

		
	response.clear();
	hapClient->request.clear();
	hapClient->clear();

	hapClient->state = HAP_CLIENT_STATE_IDLE;
	LogV("OK", true);
}

void HAPServer::handlePairingsRemove(HAPClient* hapClient, const uint8_t* identifier){
	
	LogV( "<<< Handle client [" + hapClient->client.remoteIP().toString() + "] -> POST /pairings remove ...", false);	
	
	bool removeItsOwnPairings = false;

	// id identifier is controllers id, then disonnect 
	if (memcmp(identifier, hapClient->getId(), HAP_PAIRINGS_ID_LENGTH) == 0) {
		Serial.println("removie its own pairings");
		removeItsOwnPairings = true;
	}
	
	// ToDo:
	// According to the spec, only admin controllers can delete pairings.
	// 
	// But this will deny removing ones own pairings
	// i will allow this
	// 
	if ( (hapClient->isAdmin() == false) && (removeItsOwnPairings == false) ) {
		LogE("ERROR: Non-Admin controllers are not allowed to call this method!", true);
		sendErrorTLV(hapClient, HAP_PAIR_STATE_M2, HAP_ERROR_AUTHENTICATON);
		return;
	}

	TLV8 response;	
	response.encode(HAP_TLV_STATE, 1, HAP_PAIR_STATE_M2);


	// if not paired 
	LogD("Removing pairings ...", false);
	if (!_accessorySet->getPairings()->removePairing(identifier)){
		
		LogE("ERROR: No pairings found!", true);
		
		response.encode(HAP_TLV_ERROR, 1, HAP_ERROR_UNKNOWN);
		

		uint8_t data[response.size()];
		size_t length = 0;

		response.decode(data, &length);
		sendEncrypt(hapClient, HTTP_200, data, length, true, "application/pairing+tlv8");
		
		response.clear();
		hapClient->request.clear();
		hapClient->clear();
		hapClient->client.stop();
		return;
	} 

	// ToDo: Check if needed here!?
	_accessorySet->getPairings()->save();

	LogD(" OK", true);

	// send response
	

#if HAP_DEBUG_TLV8	
	response.print();
#endif

	uint8_t data[response.size()];
	size_t length = 0;

	response.decode(data, &length);
	sendEncrypt(hapClient, HTTP_200, data, length, true, "application/pairing+tlv8");

	response.clear();
	hapClient->request.clear();	

	if (removeItsOwnPairings) {
		hapClient->client.stop();
	}
	
	
	hapClient->clear();
	
	// tear down all other pairings if admin removed
	// and update mdns
	if (_accessorySet->isPaired()) {
		
		hapClient->state = HAP_CLIENT_STATE_ALL_PAIRINGS_REMOVED;
		_eventManager.queueEvent(EventManager::kEventAllPairingsRemoved, HAPEvent());

		// update mdns
		updateServiceTxt();
	}	

#if HAP_USE_MBEDTLS_SRP		
	if (_srpInitialized) {
		srp_keypair_delete(_srp->keys);
		srp_session_delete(_srp->ses);
		_srpInitialized = false;
	}		
#else	
	if(_srp) {
		//LogW("Free SRP!", false);
		srp_cleanup(_srp);	
		//LogW("OK", true);
	}
#endif

	LogV("OK", true);
}

void HAPServer::handlePairingsPost(HAPClient* hapClient, uint8_t* bodyData, size_t bodyDataLen){

	
	LogV( "<<< Handle client [" + hapClient->client.remoteIP().toString() + "] -> POST /pairings ...", false);

	TLV8 tlv;
	tlv.encode(bodyData, bodyDataLen);

#if HAP_DEBUG_TLV8	
	tlv.print();
#endif
	
	TLV8Entry *entry = tlv.searchType(tlv._head, HAP_TLV_METHOD); // 0x01

	HAP_TLV_PAIR_TYPE method = (HAP_TLV_PAIR_TYPE) entry->value[0];


	if (method == HAP_TLV_PAIR_ADD) {

		TLV8Entry *entryIdentifier = tlv.searchType(tlv._head, HAP_TLV_IDENTIFIER); // 0x01
		TLV8Entry *entryPublicKey = tlv.searchType(tlv._head, HAP_TLV_PUBLIC_KEY); // 0x03		
		TLV8Entry *entryAdmin = tlv.searchType(tlv._head, HAP_TLV_PERMISSIONS); // 0x0b

		handlePairingsAdd(hapClient, entryIdentifier->value, entryPublicKey->value, *(entryAdmin->value) );

	} else if (method == HAP_TLV_PAIR_REMOVE) {
		TLV8Entry *entryIdentifier = tlv.searchType(tlv._head, HAP_TLV_IDENTIFIER); // 0x01
		handlePairingsRemove(hapClient, entryIdentifier->value);
	} else if (method == HAP_TLV_PAIR_LIST) {
		handlePairingsList(hapClient);
	}

	tlv.clear();
		
	LogV("OK", true);
}


void HAPServer::handleCharacteristicsGet(HAPClient* hapClient){

#if HAP_DEBUG_HEAP        
    LogE("+++++++++++++++++++ " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " start", true);
    HAPLogger::logFreeHeap(0,0, COLOR_MAGENTA);
#endif

	LogV( "<<< Handle client [" + hapClient->client.remoteIP().toString() + "] -> GET /characteristics ...", false);

	String idStr = hapClient->request.params["id"];
	//LogE(idStr, true);

	bool hasParamMeta = false;	
	bool hasParamPerms = false;
	bool hasParamEvent = false;
	bool hasParamType = false;

	for (const auto &p : hapClient->request.params) {

#if HAP_DEBUG
    	LogD("param: " + p.first + " - " + p.second, true);				
#endif	
		if (p.first == "meta" && p.second == "1"){
			hasParamMeta = true;
		}	else if (p.first == "perms" && p.second == "1") {
			hasParamPerms = true;
		}	else if (p.first == "ev" && p.second == "1"){
			hasParamEvent = true;
		}	else if (p.first == "type" && p.second == "1"){
			hasParamType = true;
		}
	}


	//String result = "[";
	DynamicJsonDocument root(HAP_ARDUINOJSON_BUFFER_SIZE);
	JsonArray jsonCharacteristics = root.createNestedArray("characteristics");

	bool errorOccured = false;
	int32_t errorCode = 0;

	do {
		int curPos = 0;
		int endIndex = idStr.indexOf(",");		
		if (endIndex == -1){
			endIndex = idStr.length();		
		}

		String keyPair = idStr.substring(curPos, endIndex); 

		int equalIndex = keyPair.indexOf(".");

		int aid = keyPair.substring(0, equalIndex).toInt();
		int iid = keyPair.substring(equalIndex + 1).toInt();

		
		JsonObject chr = jsonCharacteristics.createNestedObject();
		chr["aid"] = aid;

		characteristics* character = _accessorySet->getCharacteristics(aid, iid);
		if (character) {
			if (character->readable()){
				character->toJson(chr, hasParamType, hasParamPerms, hasParamEvent, hasParamMeta);
				// callback for get value
				if (character->valueGetCallback){
					character->valueGetCallback();
				}
			} else {
				chr["iid"] = iid;
				chr["status"] = String(HAP_STATUS_WRITEONLY_READ);
				errorOccured = true;
			}
			
		} else {
			chr["iid"] = iid;
			chr["status"] = String(HAP_STATUS_RESOURCE_NOT_FOUND);
			errorCode = -1;
			errorOccured = true;
		}
		
		
		idStr = idStr.substring(endIndex + 1); 
	} while ( idStr.length() > 0 );

	String response;			
	serializeJson(root, response);

#if HAP_DEBUG
	serializeJson(root, Serial);
#endif

	if (errorCode == -1){
		// Accessory not found
		sendEncrypt(hapClient, HTTP_400, response, true);
	} else if (errorOccured == false) {
		// everything ok
		sendEncrypt(hapClient, HTTP_200, response, true);	
	} else if (errorOccured == true) {
		// partial ok
    	sendEncrypt(hapClient, HTTP_207, response, true);	
	}

	LogV("OK", true);
	hapClient->state = HAP_CLIENT_STATE_IDLE;

#if HAP_DEBUG_HEAP    
    LogE("=================== " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " end", true);
    HAPLogger::logFreeHeap(0,0, COLOR_MAGENTA);
#endif

}

void HAPServer::handleCharacteristicsPut(HAPClient* hapClient, String body){

#if HAP_DEBUG_HEAP        
    LogE("+++++++++++++++++++ " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " start", true);
    HAPLogger::logFreeHeap(0,0, COLOR_MAGENTA);
#endif

	LogV( "<<< Handle client [" + hapClient->client.remoteIP().toString() + "] ->  PUT /characteristics ...", true);
	
	DynamicJsonDocument root(HAP_ARDUINOJSON_BUFFER_SIZE);
	DeserializationError error = deserializeJson(root, body);

	if (error) {
    	LogE("[ERROR] Parsing put characteristics request failed!", true);
		// ToDo Send Error Response to client
    	return;
  	}

#if HAP_DEBUG	
	serializeJson(root, Serial);
	Serial.println();
#endif
	
	int s = root["characteristics"].as<JsonArray>().size();
	DynamicJsonDocument responseRoot( JSON_ARRAY_SIZE(s) + JSON_OBJECT_SIZE(1) + s * JSON_OBJECT_SIZE(4) ); 	
	JsonArray responseArray = responseRoot.createNestedArray("characteristics");

	bool errorOccured = false;

	
	for( auto jc : root["characteristics"].as<JsonArray>()) {

    	int aid = jc["aid"].as<int>();
    	int iid = jc["iid"].as<int>();

		bool isEvent = false;
		
		characteristics *character = _accessorySet->getCharacteristics(aid, iid);		
		
		JsonObject jsonNewChr = responseArray.createNestedObject();
		jsonNewChr["aid"] = aid;

		if (character) {	
					
			if (jc.as<JsonObject>().containsKey("ev")){
				isEvent = true;
			}

			if (isEvent) {
				if (character->notifiable() ) {
					character->toJson(jsonNewChr);

					hapClient->subscribe(aid, iid, jc["ev"].as<bool>());

					if (jc["ev"].as<bool>()) {
						struct HAPEvent event = HAPEvent(hapClient, aid, iid, character->value());					
						_eventManager.queueEvent( EventManager::kEventNotifyController, event);
					}
					
				} else {
					// char has no event permission
					LogE("[ERROR] - Resource notify not permitted!", true);
					jsonNewChr["iid"] = iid;
					jsonNewChr["status"] = String(HAP_STATUS_NO_NOTIFICATION);
					errorOccured = true;
				}
			} else {

				if (character->writable() ) {
					character->setValue(jc["value"].as<String>());
					// Add to jsonCharacteristics array				
					character->toJson(jsonNewChr);
				} else {
					LogE("[ERROR] - Resource not writable!", true);
					jsonNewChr["iid"] = iid;
					jsonNewChr["status"] = String(HAP_STATUS_READONLY_WRITE);
					errorOccured = true;			    		
				}
			}

						
		} else {
			LogE("[ERROR] - Resource not found!", true);
			jsonNewChr["iid"] = iid;
			jsonNewChr["status"] = String(HAP_STATUS_RESOURCE_NOT_FOUND);
			errorOccured = true;			    		
		}
	}
	
	if (errorOccured){
		String response;
		serializeJson(responseRoot, response);

		sendEncrypt(hapClient, HTTP_207, response, false);
	} else {
		sendEncrypt(hapClient, HTTP_204, "", false);
	}

	hapClient->state = HAP_CLIENT_STATE_IDLE;
	LogV("OK", true);

#if HAP_DEBUG_HEAP    
    LogE("=================== " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " end", true);
    HAPLogger::logFreeHeap(0,0, COLOR_MAGENTA);
#endif
}

void HAPServer::handleEventUpdateConfigNumber( int eventCode, struct HAPEvent eventParam ){
	_accessorySet->configurationNumber++;
	updateServiceTxt();
}


void HAPServer::handleEventRebootNow(int eventCode, struct HAPEvent eventParam){
	LogW("*********************************************************", true);
	LogW("*                                                       *", true);
	LogW("*                 !!! Rebooting now !!!                 *", true);
	LogW("*                                                       *", true);
	LogW("*********************************************************", true);

	delay(2000);
	ESP.restart();
}


void HAPServer::handleEvents( int eventCode, struct HAPEvent eventParam )
{

	// Stopping events
	if (stopEvents() == true) {
		return;
	}

	if (_clients.size() > 0){
		int count = 0;
		int totalEvents = _eventManager.getNumEventsInQueue();
		int noOfEvents = _eventManager.getNumEventCodeInQueue(eventCode);
		struct HAPEvent evParams[noOfEvents + 1];
		int addedToHomekitEvent = 0;

		// add
		if (eventCode == EventManager::kEventNotifyController) {			
			evParams[addedToHomekitEvent++] = eventParam;			
		}
		
		while (!_eventManager.isEventQueueEmpty()){

			int evCode;
			struct HAPEvent evParam;
			if (_eventManager.popEvent(&evCode, &evParam)){

				if (evCode == EventManager::kEventNotifyController) {

										
					evParams[addedToHomekitEvent++] = evParam;								
				} else {
					// Add again to queue
					_eventManager.queueEvent(evCode, evParam);
				}
				count++;
				
				if (count == totalEvents + 1){
					break;
				}
			}
		}

		for (auto& hapClient : _clients) {

			const size_t bufferSize = 512;
			DynamicJsonDocument root(bufferSize);			

			JsonArray jsonCharacteristics = root.createNestedArray("characteristics");	
			String response = "";

			bool isSubcribedToAtLeastOne = false;

			for (int i=0; i < addedToHomekitEvent; i++){

				int aid = evParams[i].aid;
				int iid = evParams[i].iid;


				if ( hapClient.isSubscribed(aid, iid) ) {										
					characteristics *character = _accessorySet->getCharacteristics(aid, iid);
					
					if (character) {

						LogD(">>> Handle event - code: " + String(eventCode) + " aid: " + String(aid) + " iid: " + String(iid) + " - value: ", false);
    					LogD(evParams[i].value, true);

						JsonObject chr = jsonCharacteristics.createNestedObject();
						chr["aid"] = aid;
						character->toJson(chr);
						
						// Serial.print("event json: ");
						// serializeJsonPretty(chr, Serial);
						// Serial.println();

						isSubcribedToAtLeastOne = true;	
					} else {
						LogE(">>> Not notifiable event - code: " + String(eventCode) + " aid: " + String(aid) + " iid: " + String(iid) + " - value: ", false);
    					LogE(evParams[i].value, true);
					}
				}
			}
			
			serializeJson(root, response);

			if (isSubcribedToAtLeastOne) {
#if HAP_DEBUG
				Serial.print("response: ");
				serializeJson(root, Serial);
				Serial.println();
#endif				
				sendEvent(&hapClient, response);
			}
		}

	}

};


bool HAPServer::sendEvent(HAPClient* hapClient, String response){
	LogD(">>> Sending event to client [" + hapClient->client.remoteIP().toString() + "] ...", false);
	if ( hapClient->client.connected() ){				
		sendEncrypt(hapClient, EVENT_200, response, true);	
		LogD(" OK", true);
		return true;		
	} 
	
	LogW("WARNING: No client available to send the event to!", true);		
	return false;

}

bool HAPServer::isPaired(){	
	return _accessorySet->isPaired();
}

void HAPServer::stopPlugins(bool value){
	if (value)
		_stopPlugins = false;
	else
		_stopPlugins = true;
}


void HAPServer::__setFirmware(const char* name, const char* version, const char* rev) {

	if (strlen(name) + 1 - 10 > MAX_FIRMWARE_NAME_LENGTH || strlen(version) + 1 - 10 > MAX_FIRMWARE_VERSION_LENGTH) {
		LogE( F("[ERROR] Either the name or version string is too long"), true);
		return;  // never reached, here for clarity
	}

	// Remove flags
	char ver[20];
	strncpy(ver, version + 5, strlen(version) - 5);
	ver[strlen(version) - 5] = '\0';
}

void HAPServer::__setBrand(const char* brand) {

	if (strlen(brand) + 1 - 10 > MAX_BRAND_LENGTH) {
		LogE(F("[ERROR] The brand string is too long"), true);
		return;  // never reached, here for clarity
	}

	strncpy(_brand, brand + 5, strlen(brand) - 10);
	_brand[strlen(brand) - 10] = '\0';
}


HAPAccessorySet* HAPServer::getAccessorySet(){
	return _accessorySet;
}

void HAPServer::updateConfig(){
	LogD("Updating configuration ...", false);
	

	HAPLogger::setLogLevel(_config.config()["homekit"]["loglevel"].as<uint8_t>());    


	LogD("OK", true);
}

void HAPServer::handleEventUpdatedConfig(int eventCode, struct HAPEvent eventParam){

	LogI("Handle update config event", true);
	const size_t capacity = HAP_ARDUINOJSON_BUFFER_SIZE / 8;
    DynamicJsonDocument doc(capacity);
	
	JsonObject plugins = doc.createNestedObject("plugins");

  	for (auto & plugin : _plugins) {
			
		if (plugin->isEnabled()) {			
        	plugins[plugin->name()] = plugin->getConfig();				
		}			
	} 

	_config.mergeConfig(doc.as<JsonObject>());
	
#if HAP_DEBUG	
	_config.prettyPrintTo(Serial);
#endif

	_config.save();
	updateConfig();	
}


#if HAP_DEBUG && HAP_WEBSERVER_USE_SPIFFS
void HAPServer::listDir(FS &fs, const char * dirname, uint8_t levels) {
    LogD("Listing directory: ", false);
	LogD(dirname, true);

    File root = fs.open(dirname);
    if(!root){
        LogE("ERROR: Failed to open directory", true);
        return;
    }
	
    if(!root.isDirectory()){
        LogW("WaRNING: ", false);
		LogW(dirname, false);
		LogW(" is not a directory", true);
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            LogD("  DIR : ", false);
            LogD(file.name(), true);
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            LogD("  FILE: ", false);
            LogD(file.name(), false);
            LogD("\tSIZE: ", false);
            LogD(file.size(), true);
        }
        file = root.openNextFile();
    }
}
#endif


HAPServer hap;
