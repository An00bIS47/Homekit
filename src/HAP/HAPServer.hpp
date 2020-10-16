//
// HAPServer.hpp
// Homekit
//
//  Created on: 08.08.2017
//      Author: michael
//

#ifndef HAPSERVER_HPP_
#define HAPSERVER_HPP_

#include <Arduino.h>
#include "HAPGlobals.hpp"

#if HAP_WEBSERVER_USE_SPIFFS  
#include <FS.h>
#include <SPIFFS.h>
#endif


#include <WiFiClient.h>
#include <WiFiServer.h>
#include <Preferences.h>
#include <vector>
#include <map>

#include <ArduinoJson.h>

#if HAP_PIXEL_INDICATOR_ENABLED
#include "HAPIndicatorPixel.hpp"
#endif

#include "HAPClient.hpp"
#include "HAPAccessorySet.hpp"
#include "HAPVerifyContext.hpp"
#include "HAPVersion.hpp"
#include "HAPWebServer.hpp"
#include "HAPConfig.hpp"
#include "HAPWiFiHelper.hpp"
#include "HAPTLV8Types.hpp"
#include "HAPPlugins.hpp"
#include "plugins/Plugins.hpp"
#include "HAPFakegatoFactory.hpp"

#include "EventManager.h"

#if HAP_KEYSTORE_ENABLED
#include "HAPKeystore.hpp"
#endif

#define Homekit_setFirmware(name, version, rev) \
const char* __FLAGGED_FW_NAME 		= "\xbf\x84\xe4\x13\x54" name "\x93\x44\x6b\xa7\x75"; \
const char* __FLAGGED_FW_VERSION 	= "\x6a\x3f\x3e\x0e\xe1" version "\xb0\x30\x48\xd4\x1a"; \
const char* __FLAGGED_FW_REV 		= "\x6a\x3f\x3e\x0e\xe2" rev "\xb0\x30\x48\xd4\x1b"; \
hap.__setFirmware(__FLAGGED_FW_NAME, __FLAGGED_FW_VERSION, __FLAGGED_FW_REV);

#define Homekit_setBrand(brand) \
const char* __FLAGGED_BRAND = "\xfb\x2a\xf5\x68\xc0" brand "\x6e\x2f\x0f\xeb\x2d"; \
hap.__setBrand(__FLAGGED_BRAND);

#if HAP_UPDATE_ENABLE_OTA || HAP_UPDATE_ENABLE_FROM_WEB
#include "HAPUpdate.hpp"
#endif





#if HAP_PRINT_QRCODE
#if HAP_GENERATE_XHM
#else
#error HAP_GENERATE_XHM must be enabled!
#endif
#endif	



#if HAP_USE_MBEDTLS_SRP

#include "m_srp.h"
#include "m_srp_internal.h"

typedef struct Srp_ {
	SRPSession *ses;
	const unsigned char * bytes_s; int len_s;
	const unsigned char * bytes_v; int len_v;
	SRPKeyPair *keys;
	const unsigned char * bytes_B; int len_B;
	SRPVerifier *ver;
	char * username;
} Srp;

#define SRP_SALT_LENGTH         16
#define SRP_PUBLIC_KEY_LENGTH   384
#define SRP_PROOF_LENGTH        64
#define SRP_SESSION_KEY_LENGTH  64

#else
#include "srp.h"

#endif


// ToDo: Remove?
static const char HTTP_200[] PROGMEM 					= "HTTP/1.1 200 OK\r\n";
static const char HTTP_204[] PROGMEM 					= "HTTP/1.1 204 No Content\r\n";
static const char HTTP_207[] PROGMEM 					= "HTTP/1.1 207 Multi-Status\r\n";
static const char HTTP_400[] PROGMEM 					= "HTTP/1.1 400 Bad Request\r\n";

static const char HTTP_CONTENT_TYPE_HAPJSON[] PROGMEM 	= "Content-Type: application/hap+json\r\n";
static const char HTTP_CONTENT_TYPE_TLV8[] PROGMEM 		= "Content-Type: application/pairing+tlv8\r\n";

static const char HTTP_KEEP_ALIVE[] PROGMEM		 		= "Connection: keep-alive\r\n";
static const char HTTP_TRANSFER_ENCODING[] PROGMEM		= "Transfer-Encoding: chunked\r\n";

static const char HTTP_CRLF[] PROGMEM 					= "\r\n";

static const char EVENT_200[] PROGMEM 					= "EVENT/1.0 200 OK\r\n";






class HAPServer {
public:
	HAPServer(uint16_t port = 51628, uint8_t maxClients = 8);
	~HAPServer();

	bool begin(bool resume = false);
	void handle();

	static String versionString(){
		return HAPVersion(HOMEKIT_VERSION).toString();
	}
	

	static HAPVersion version() {
		return HAPVersion(HOMEKIT_VERSION);
	}


	void __setFirmware(const char* name, const char* version, const char* rev);
	void __setBrand(const char* brand);

	HAPAccessorySet* getAccessorySet();

#if HAP_NTP_ENABLED
	static String timeString();
	static uint32_t timestamp();
#endif

	
	bool isPaired();	

	static EventManager _eventManager;
protected:

	HAPConfig _config;

	HAPWiFiHelper _wifi;

#if HAP_KEYSTORE_ENABLED
	HAPKeystore	  _keystore;	
#endif

	// struct HAPPairSetup* _pairSetup;
#if HAP_ENABLE_WEBSERVER	
	HAPWebServer* _webserver;
#endif
	
	void updateConfig();

	HAPAccessorySet* _accessorySet;
	std::vector<HAPClient> _clients;
	WiFiServer _server;
	Preferences _preferences;	

#if HAP_NTP_ENABLED	
	static struct tm _timeinfo;
#endif

	std::vector<std::unique_ptr<HAPPlugin>> _plugins;
	HAPFakeGatoFactory _fakeGatoFactory;
	// 
	// Event handler
	// 
	void handleEvents( int eventCode, struct HAPEvent eventParam );
	void handleEventUpdateConfigNumber( int eventCode, struct HAPEvent eventParam );
	void handleEventUpdatedConfig(int eventCode, struct HAPEvent eventParam);
	void handleEventRebootNow(int eventCode, struct HAPEvent eventParam);

	void handleEventConfigReset(int eventCode, struct HAPEvent eventParam);
	void handleEventDeleteAllPairings(int eventCode, struct HAPEvent eventParam);

	bool stopEvents();
	void stopEvents(bool value);

	// 
	// Event Member callbacks
	//  ToDo: Rename 
    MemberFunctionCallable<HAPServer> listenerUpdateConfigNumber;
	MemberFunctionCallable<HAPServer> listenerConfigUpdated;
	MemberFunctionCallable<HAPServer> listenerNotificaton;	
	MemberFunctionCallable<HAPServer> listenerRebootNow;	

	MemberFunctionCallable<HAPServer> listenerConfigReset;	
	MemberFunctionCallable<HAPServer> listenerDeleteAllPairings;	

#if HAP_UPDATE_ENABLE_OTA || HAP_UPDATE_ENABLE_FROM_WEB 	
	HAPUpdate _updater;
#endif



	//
	// Homekit HTTP paths
	//

	//
	// Pairing
	//

	// Pair-Setup states
	bool handlePairSetupM1(HAPClient* hapClient);
	bool handlePairSetupM3(HAPClient* hapClient);
	bool handlePairSetupM5(HAPClient* hapClient);


	// Pair-Verify states
	bool handlePairVerifyM1(HAPClient* hapClient);
	bool handlePairVerifyM3(HAPClient* hapClient);


	// /accessories
	void handleAccessories(HAPClient* hapClient);

	// /characteristics
	void handleCharacteristicsGet(HAPClient* hapClient);	
	void handleCharacteristicsPut(HAPClient* hapClient, String body);	

	// pairings
	void handlePairingsPost(HAPClient* hapClient, uint8_t* bodyData, size_t bodyDataLen);
	void handlePairingsRemove(HAPClient* hapClient, const uint8_t* identifier);
	void handlePairingsList(HAPClient* hapClient);
	void handlePairingsAdd(HAPClient* hapClient, const uint8_t* identifier, const uint8_t* publicKey, bool isAdmin);

	// Identify
	void handleIdentify(HAPClient* hapClient);


	// 
	// Plugin handling
	// ToDo: currently unused
	// 
	void stopPlugins(bool value);
	bool startPlugin(std::unique_ptr<HAPPlugin> plugin);
	
	// Callbacks
	void handleAllPairingsRemoved();

private:

#if HAP_DEBUG && HAP_WEBSERVER_USE_SPIFFS
	static void listDir(FS &fs, const char * dirname, uint8_t levels);
#endif

	bool _isInPairingMode;

	uint8_t _homekitFailedLoginAttempts;

	String _curLine;
	uint16_t _port;

	unsigned long _minimalPluginInterval;
	unsigned long _previousMillis;

#if HAP_DEBUG	
	unsigned long _previousMillisHeap;
#endif
	struct HAPLongTermContext* _longTermContext;

#if HAP_USE_MBEDTLS_SRP
	Srp* _srp;
	bool _srpInitialized;
#else	
	void* _srp;
#endif
	char _brand[MAX_BRAND_LENGTH];

	bool _stopEvents;	
	bool _stopPlugins;

	//
	// Bonjour
	//
	bool updateServiceTxt();


	//
	// HTTP
	// 

	// parsing
	void processIncomingRequest(HAPClient* hapClient);
	void processIncomingEncryptedRequest(HAPClient* hapClient);

	void processIncomingLine(HAPClient* hapClient, String line);
	void processPathParameters(HAPClient* hapClient, String line, int curPos);

	void parseRequest(HAPClient* hapClient, const char* msg, size_t msg_len, uint8_t** out, int* outLen);
	bool handlePath(HAPClient* hapClient, uint8_t* bodyData, size_t bodyDataLen);

	// Connection states
	void handleClientState(HAPClient* hapClient);
	void handleClientAvailable(HAPClient* hapClient);
	void handleClientDisconnect(HAPClient hapClient);


	//
	// Sending responses
	// 
	bool sendResponse(HAPClient* hapClient, TLV8* response, bool chunked = true, bool closeConnection = false);
	bool sendEncrypt(HAPClient* hapClient, String httpStatus, String plainText, bool chunked = true);
	bool sendEncrypt(HAPClient* hapClient, String httpStatus, const uint8_t* bytes, size_t length, bool chunked, const char* ContentType);


	void sendErrorTLV(HAPClient* hapClient, uint8_t state, uint8_t error);

	bool sendEvent(HAPClient* hapClient, String response);

#if HAP_PIXEL_INDICATOR_ENABLED
	// ToDo: Pixel Indicator
	HAPIndicatorPixel _pixelIndicator;
#endif

	// 
	// Button
	//
	static void taskButtonRead(void* pvParameters);
	void callbackClick();
	void callbackDoubleClick();
	void callbackHold();
	void callbackLongHold();

	//
	// TLV8 Encoding 
	//
	static bool encode(HAPClient* hapClient);
	
	const char* __HOMEKIT_SIGNATURE;
};

extern HAPServer hap;

#endif /* HAPSERVER_HPP_ */
