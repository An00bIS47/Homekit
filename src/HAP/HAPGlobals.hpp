//
// HAPGlobals.hpp
// Homekit
//
//  Created on: 08.08.2017
//      Author: michael
//
#ifndef HAPGLOBALS_HPP_
#define HAPGLOBALS_HPP_


// ToDo: Use KConfig

/**
 * General
 ********************************************************************/
#define HAP_LOGLEVEL				LogLevel::DEBUG

#ifndef HAP_PIN_CODE
#define HAP_PIN_CODE 				"031-45-712"
#endif

#ifndef HAP_SETUP_ID
#define HAP_SETUP_ID 				"UPFT"
#endif

#define HAP_HOSTNAME_PREFIX			"esp32"
#define HAP_MANUFACTURER			"An00bIS47"
#define HAP_MODELL_NAME				"Huzzah32"
#define HAP_RESET_EEPROM 			0
#define HAP_BUTTON_PIN 				0       // PIN of onboard button 


#define HAP_PIXEL_INDICATOR_ENABLED 1
#define HAP_PIXEL_INDICATOR_PIN		A0		// PIN of the NeoPixel inidicator pin for wifi connection etc
#define HAP_PIXEL_INIDICATOR_BRIGHTNESS 75

/**
 * Homekit Accessory Protocol
 ********************************************************************/
#define HOMEKIT_PROTOKOL_VERSION			"1.0"
#define HOMEKIT_CONFIGURATION_NUMBER 		1		// Internal - keep value at 1
#define HAP_BATTERY_LEVEL_LOW_THRESHOLD		15		// Battery threshold for alert



/**
 * Version
 ********************************************************************/
#define HOMEKIT_VERSION_MAJOR 		0
#define HOMEKIT_VERSION_MINOR 		7
#define HOMEKIT_VERSION_REVISION 	0


/**
 * Debug
 ********************************************************************/
#ifndef HAP_DEBUG
#define HAP_DEBUG 					0
#endif

#ifndef HAP_DEBUG_HEAP
#define HAP_DEBUG_HEAP				0
#endif

#ifndef HAP_DEBUG_TLV8
#define HAP_DEBUG_TLV8				0
#endif

#ifndef HAP_DEBUG_HOMEKIT
#define HAP_DEBUG_HOMEKIT			0
#endif

#ifndef HAP_DEBUG_FAKEGATO
#define HAP_DEBUG_FAKEGATO			0
#endif

#ifndef HAP_DEBUG_FAKEGATO_DETAILED
#define HAP_DEBUG_FAKEGATO_DETAILED 0
#endif

#ifndef HAP_DEBUG_FAKEGATO_SCHEDULE
#define HAP_DEBUG_FAKEGATO_SCHEDULE 1
#endif

#ifndef HAP_DEBUG_PAIRINGS
#define HAP_DEBUG_PAIRINGS			0
#endif

#ifndef HAP_DEBUG_RESPONSES
#define HAP_DEBUG_RESPONSES			0
#endif

#ifndef HAP_DEBUG_ENCRYPTION
#define HAP_DEBUG_ENCRYPTION		0
#endif

#ifndef HAP_DEBUG_EVENTMANAGER		
#define HAP_DEBUG_EVENTMANAGER		0
#endif

#ifndef HAP_DEBUG_KEYSTORE
#define HAP_DEBUG_KEYSTORE			0
#endif

#ifndef HAP_DEBUG_RF24
#define HAP_DEBUG_RF24				0
#endif

#ifndef HAP_DEBUG_WEBSERVER
#define HAP_DEBUG_WEBSERVER 		0
#endif

#ifndef HAP_DEBUG_MIFLORA
#define HAP_DEBUG_MIFLORA			0
#endif

#ifndef HAP_DEBUG_IR
#define HAP_DEBUG_IR				1
#endif

#ifndef HAP_DEBUG_CONFIG
#define HAP_DEBUG_CONFIG			1
#endif

/**
 * Homekit pairing
 ********************************************************************/
#define HAP_ALLOW_PAIRING_WHILE_PAIRED 1		// disable for release !!


/**
 * WiFi
 ********************************************************************/
#ifndef HAP_WIFI_MODE_DEFAULT
#define HAP_WIFI_MODE_DEFAULT			4	// 0 = HAPWiFiModeAccessPoint	
#endif										// 1 = HAPWiFiModeMulti
											// 2 = HAPWiFiModeWPS			-> push button only
											// 3 = HAPWiFiModeSmartConfig	-> not working with ios13 ?
											// 4 = HAPWiFiModeBLEProv	
											// 5 = HAPWiFiModeAPProv	

#define HAP_WIFI_CONNECTION_MAX_RETRIES 5	// max retries for connection error 
                                            // before switching back to default mode
											// default: 5
#define HAP_WIFI_CONNECTION_RETRY_DELAY	2000
#define ESP_WIFI_CONNECTION_TIMEOUT		20000

#define HAP_PROVISIONING_ENABLE_BLE		1

#ifndef HAP_PROVISIONING_POP			
#define HAP_PROVISIONING_POP			"abcd1234" 		// Proof of possesion
#endif

#ifndef HAP_PROVISIONING_PREFIX
#define HAP_PROVISIONING_PREFIX			"PROV_"			// Prefix for provisioning
#endif


/**
 * Include WiFi credentials if necessary
 ********************************************************************/
#if HAP_WIFI_DEFAULT_MODE == 1

#include "../WiFiCredentials.hpp"

#ifndef WIFI_SSID
#error	No WiFi SSID defined!
#endif

#ifndef WIFI_PASSWORD
#error	No WiFi Password defined!
#endif

#endif


/**
 * Keystore 
 ********************************************************************/
#define HAP_KEYSTORE_ENABLED				1


#define HAP_KEYSTORE_PARTITION_LABEL	 	"keystore_0"
#define HAP_KEYSTORE_PARTITION_LABEL_ALT 	"keystore_1"
#define HAP_KEYSTORE_STORAGE_LABEL   		"keystore"


/**
 * WebServer 
 ********************************************************************/

#ifndef HAP_ENABLE_WEBSERVER
#define HAP_ENABLE_WEBSERVER		1		// Enable Webinterface
#endif										// Default: 1

// #define HAP_ENABLE_WEBSERVER_CORE_0	0		// Run webserver on core 0 in a seperate task
// 											// default 0 - Still work-in-progress
// 											// currently could causes heap caps failures if used with BLE


#define HAP_WEBSERVER_USE_JWT 		0		// use JWT for access
											// TODO: Proper token signature verification
											// currently not implemented properly

#ifndef HAP_WEBSERVER_USE_SSL
#define HAP_WEBSERVER_USE_SSL		1		// use SSL for WebServer 
#endif										// Default: 1	


#ifndef HAP_WEBSERVER_ADMIN_USERNAME
#define HAP_WEBSERVER_ADMIN_USERNAME	"admin"
#endif


#ifndef HAP_WEBSERVER_ADMIN_PASSWORD
#define HAP_WEBSERVER_ADMIN_PASSWORD	"secret"
#endif


#ifndef HAP_WEBSERVER_API_USERNAME
#define HAP_WEBSERVER_API_USERNAME		"api"
#endif

#ifndef HAP_WEBSERVER_API_PASSWORD
#define HAP_WEBSERVER_API_PASSWORD		"test"
#endif


#define HAP_WEBSERVER_USE_SPIFFS	0
#define HTTPS_DISABLE_SELFSIGNING 	1		// Disable self signed certificate generation on the fly
#define DEBUG_MULTIPART_PARSER 		0		// Enable to debug multipart form parser


#if HAP_DEBUG_WEBSERVER == 1
#define HTTPS_LOGLEVEL 4					// Log Levels:
											// 0 : NONE
											// 1 : + ERROR
											// 2 : + WARNING
											// 3 : + INFO
											// 4 : + DEBUG
#define HTTPS_LOGTIMESTAMP					// Add Timestamp (uptime) to log
#endif


/**
 * Captive Portal 
 ********************************************************************/
#define HAP_CAPTIVE_DNSSERVER_PORT	53
#define HAP_CAPTIVE_AP_IP			"192.168.0.1"
#define HAP_CAPTIVE_AP_SSID			"Homekit Captive Portal"	// ssid of the access point
#define HAP_CAPTIVE_TITLE			"Homekit Captive Portal"	// Title of the webpage for captive portal


/**
 * Fakegato 
 ********************************************************************/

#ifndef HAP_FAKEGATO_BUFFER_SIZE
#define HAP_FAKEGATO_BUFFER_SIZE	768     // Number of history entries for each characteristic 
#endif										// default: 768


#ifndef HAP_FAKEGATO_INTERVAL
#define HAP_FAKEGATO_INTERVAL       300000	// Interval to add entry to history in millis
#endif                                      // EVE app requires at least one entry every 10 mins
											// default: 300000

#ifndef HAP_FAKEGATO_CHUNK_SIZE
#define HAP_FAKEGATO_CHUNK_SIZE     16      // Number of entries sent at once from device to EVE app
#endif										// default: 16



/**
 * OTA + HAP Update Server
 ********************************************************************/

#ifndef HAP_UPDATE_ENABLE_OTA
#define HAP_UPDATE_ENABLE_OTA		1		// Enable ArduinoOTA	
#endif										// Default: 0	

#ifndef HAP_UPDATE_ENABLE_FROM_WEB
#define HAP_UPDATE_ENABLE_FROM_WEB 	0		// Use HAP update server to check
#endif										// if a update is available on the
											// provided webserver
											// Default: 0

#ifndef HAP_UPDATE_OTA_PASSWORD
#define HAP_UPDATE_OTA_PASSWORD		"secret"
#endif

#ifndef HAP_UPDATE_OTA_PORT
#define HAP_UPDATE_OTA_PORT			3232
#endif

#ifndef HAP_UPDATE_WEB_INTERVAL
#define HAP_UPDATE_WEB_INTERVAL		60000	// Interval for web update check in ms
#endif


#if HAP_UPDATE_ENABLE_FROM_WEB
//#define HAP_UPDATE_SERVER_URL 	"192.168.178.151"	
#define HAP_UPDATE_SERVER_HOST 		"homebridge"		// HTTP Server url for updates
#define HAP_UPDATE_SERVER_PORT		3001				// Update Server port
#define HAP_UPDATE_ENABLE_SSL		1					// enable SSL for HAP Update
#endif


#define HAP_UPDATE_TIMEOUT 			2000



/**
 * NTP Settings
 ********************************************************************/
#define HAP_NTP_ENABLED 			1		// Enable SNTP client
											// Default: 1


#if HAP_NTP_ENABLED

#ifndef HAP_NTP_SERVER_URL
#define HAP_NTP_SERVER_URL			"time.euro.apple.com"						// NTP server url
#define HAP_NTP_SERVER_URL_FALLBACK	"pool.ntp.org"
#endif

const char* const HAP_NTP_SERVER_URLS[] = {HAP_NTP_SERVER_URL, HAP_NTP_SERVER_URL_FALLBACK};
#define HAP_NTP_SERVER_URLS_SIZE	2

#ifndef HAP_NTP_TIME_FORMAT
#define HAP_NTP_TIME_FORMAT			"%Y-%m-%d %H:%M:%S"							// strftime format
#endif

// get the timezones here:
// https://remotemonitoringsystems.ca/time-zone-abbreviations.php
#ifndef HAP_NTP_TZ_INFO
#define HAP_NTP_TZ_INFO     		"CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00"		// timezone for berlin
#endif

#endif



/**
 * Event Manager
 ********************************************************************/
#ifndef HAP_EVENTMANAGER_LISTENER_SIZE		
#define HAP_EVENTMANAGER_LISTENER_SIZE	32
#endif

#ifndef HAP_EVENTMANAGER_QUEUE_SIZE
#define HAP_EVENTMANAGER_QUEUE_SIZE		16
#endif


#define EVENTMANAGER_LISTENER_LIST_SIZE		HAP_EVENTMANAGER_LISTENER_SIZE
#define EVENTMANAGER_EVENT_QUEUE_SIZE 		HAP_EVENTMANAGER_QUEUE_SIZE


/**
 * Crypto 
 ********************************************************************/
#define HAP_USE_MBEDTLS_HKDF		1		// Use MBEDTLS HDKF
											// Default: 1

#define HAP_USE_MBEDTLS_SRP			1		// if 0 then use WolfSSL SRP
											// Default: 1

#define HAP_USE_MBEDTLS_POLY		1		// if 0 then use WolfSSL ChaCha20 Poly1305											
											// Default: 1

#define HAP_USE_MBEDTLS_CURVE25519 	0		// not working -> use HAP_USE_LIBSODIUM

#define HAP_USE_LIBSODIUM			1		




/**
 * QR Code 
 ********************************************************************/
#define HAP_GENERATE_XHM			1		// Create hash for qr codes
											// Default: 1

#define HAP_PRINT_QRCODE			0		// !!! HAP_GENERATE_XHM must be enabled !!!
											// Print QR code on console
											// Default: 0								

#define HAP_PRINT_QRCODE_SVG		0

#if HAP_PRINT_QRCODE == 1
#undef HAP_GENERATE_XHM
#define HAP_GENERATE_XHM 1
#endif


/**
 * Plugins
 * !!! Add new plugins on top here and 
 *     add them as well on top of the define bellow !!!
 ********************************************************************/

#ifndef HAP_PLUGIN_USE_NIMBLE_MIFLORA
#define HAP_PLUGIN_USE_NIMBLE_MIFLORA 	0		// https://github.com/h2zero/esp-nimble-cpp
#endif

#ifndef HAP_PLUGIN_USE_IR
#define HAP_PLUGIN_USE_IR				0
#endif

#ifndef HAP_PLUGIN_USE_FAN_HONEYWELL
#define HAP_PLUGIN_USE_FAN_HONEYWELL	0
#endif

#ifndef HAP_PLUGIN_USE_RF24
#define HAP_PLUGIN_USE_RF24				0
#endif

#ifndef HAP_PLUGIN_USE_SSD1306
#define HAP_PLUGIN_USE_SSD1306			0
#endif

#ifndef HAP_PLUGIN_USE_LED
#define HAP_PLUGIN_USE_LED				0
#endif

#ifndef HAP_PLUGIN_USE_SSD1331
#define HAP_PLUGIN_USE_SSD1331			0
#endif

#ifndef HAP_PLUGIN_USE_PCA301
#define HAP_PLUGIN_USE_PCA301			0
#endif

#ifndef HAP_PLUGIN_USE_NEOPIXEL
#define HAP_PLUGIN_USE_NEOPIXEL			0
#endif

#ifndef HAP_PLUGIN_USE_INFLUXDB
#define HAP_PLUGIN_USE_INFLUXDB			0
#endif

#ifndef HAP_PLUGIN_USE_HYGROMETER
#define HAP_PLUGIN_USE_HYGROMETER		0
#endif

#ifndef HAP_PLUGIN_USE_RCSWITCH
#define HAP_PLUGIN_USE_RCSWITCH			0
#endif

#ifndef HAP_PLUGIN_USE_DHT
#define HAP_PLUGIN_USE_DHT				0
#endif

#ifndef HAP_PLUGIN_USE_BME280
#define HAP_PLUGIN_USE_BME280			0	// < last digit of feature number
#endif

/**
 * Plugin Logic
 *    
 ********************************************************************/
// Enable IR if Honeywell is enabled
#if HAP_PLUGIN_USE_FAN_HONEYWELL
#undef HAP_PLUGIN_USE_IR
#define HAP_PLUGIN_USE_IR 1
#endif

#if HAP_PLUGIN_USE_NIMBLE_MIFLORA
#define HAP_BLE_ENABLED 1
#endif

#if HAP_BLE_ENABLED
// disable HTTPS if BLE is enabled
#undef HAP_WEBSERVER_USE_SSL
#define HAP_WEBSERVER_USE_SSL 0

// reduce fakegato buffer
#undef HAP_FAKEGATO_BUFFER_SIZE
#define HAP_FAKEGATO_BUFFER_SIZE 512
#endif

// shut off pixel indicator if using neo pixel
#if HAP_PLUGIN_USE_NEOPIXEL
#undef HAP_PIXEL_INDICATOR_ENABLED
#define HAP_PIXEL_INDICATOR_ENABLED 0
#endif

/**
 * Plugins feature rev calculation
 * !!! Add new plugins on top here !!!     
 ********************************************************************/
#define HAP_PLUGIN_FEATURE_NUMBER \
STR(HAP_PLUGIN_USE_NIMBLE_MIFLORA) \
STR(HAP_PLUGIN_USE_IR) \
STR(HAP_PLUGIN_USE_FAN_HONEYWELL) \
STR(HAP_PLUGIN_USE_RF24) \
STR(HAP_PLUGIN_USE_SSD1306) \
STR(HAP_PLUGIN_USE_LED) \
STR(HAP_PLUGIN_USE_SSD1331) \
STR(HAP_PLUGIN_USE_PCA301) \
STR(HAP_PLUGIN_USE_NEOPIXEL) \
STR(HAP_PLUGIN_USE_INFLUXDB) \
STR(HAP_PLUGIN_USE_HYGROMETER) \
STR(HAP_PLUGIN_USE_RCSWITCH) \
STR(HAP_PLUGIN_USE_DHT) \
STR(HAP_PLUGIN_USE_BME280) 

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)




/**
 * Options - Do not edit !!!
 ********************************************************************/
#define HAP_BUFFERED_SEND			1		// Only used for TLV8 responses
											// Send all data via wifi in *one* response
											// Default: 1
											// Not yet working without buffered send :(
	

#define HAP_LONG_UUID				0		// Use long uuid as type in accessory json
											// Default: 0
											


#define HAP_MINIMAL_PLUGIN_INTERVAL	1000	// Minimal plugin handle interval in ms
											// Default: 1000
											// ToDo: Nedded ??




/**
 * Limits 
 * !! Do not edit !!!
 ********************************************************************/
#if HAP_BUFFERED_SEND
#define HAP_BUFFER_SEND_SIZE		3192	// 3192 max ?
#endif

#define HAP_ARDUINOJSON_BUFFER_SIZE 4096	//(ESP.getMaxAllocHeap() - 4096)
#define HAP_ENCRYPTION_BUFFER_SIZE 	(ESP.getMaxAllocHeap() - 4096)	// 16384

#define HAP_PAIRINGS_MAX			16		// Number of available pairings 
											// Default: 16

#define HAP_STRING_LENGTH_MAX		64		// Max length of strings for config validation

#define MAX_BRAND_LENGTH 			32
#define MAX_FIRMWARE_NAME_LENGTH 	32
#define MAX_FIRMWARE_VERSION_LENGTH	16		 // sizeof("1000.1000.1000.1000");


/**
 * SRP 
 * !! Do not edit !!!
 ********************************************************************/
#define SRP_TEST					0		// Test SRP - keep disabled !



/**
 * Arduino + EXP32 specific
 * !! Do not edit !!!
 ********************************************************************/
#ifndef ARDUINO_ARCH_ESP32
#define ARDUINO_ARCH_ESP32			1
#endif

#ifndef ESP32
#define ESP32						1
#endif


/**
 * Keysizes
 * Do not edit!
 ********************************************************************/
#ifndef CURVE25519_KEY_LENGTH
#define CURVE25519_KEY_LENGTH       	32
#endif

#ifndef CURVE25519_SECRET_LENGTH
#define CURVE25519_SECRET_LENGTH    	32
#endif

#ifndef ED25519_PUBLIC_KEY_LENGTH
#define ED25519_PUBLIC_KEY_LENGTH    	32
#endif

#ifndef ED25519_PRIVATE_KEY_LENGTH
#define ED25519_PRIVATE_KEY_LENGTH    	64
#endif

#ifndef ED25519_SIGN_LENGTH
#define ED25519_SIGN_LENGTH    			64
#endif

#ifndef HAP_ENCRYPTION_NONCE_SIZE
#define HAP_ENCRYPTION_NONCE_SIZE 		12		// Don't change!
#endif

#ifndef HAP_ENCRYPTION_HMAC_SIZE
#define HAP_ENCRYPTION_HMAC_SIZE		16		// Don't change!
#endif

#ifndef HAP_ENCRYPTION_KEY_SIZE
#define HAP_ENCRYPTION_KEY_SIZE			32		// Don't change!
#endif

#ifndef HAP_ENCRYPTION_AAD_SIZE
#define HAP_ENCRYPTION_AAD_SIZE 		2
#endif

#ifndef CHACHA20_POLY1305_AEAD_KEYSIZE
#define CHACHA20_POLY1305_AEAD_KEYSIZE      32
#endif

#ifndef CHACHA20_POLY1305_AUTH_TAG_LENGTH
#define CHACHA20_POLY1305_AUTH_TAG_LENGTH	16
#endif

#ifndef CHACHA20_POLY1305_NONCE_LENGTH
#define CHACHA20_POLY1305_NONCE_LENGTH      12
#endif

#ifndef HKDF_KEY_LEN
#define HKDF_KEY_LEN      CHACHA20_POLY1305_AEAD_KEYSIZE
#endif

#ifndef HAP_PAIRINGS_ID_LENGTH
#define HAP_PAIRINGS_ID_LENGTH 		36
#endif

#define HAP_PAIRINGS_LTPK_LENGTH 	ED25519_PUBLIC_KEY_LENGTH
#define HAP_PAIRINGS_LTSK_LENGTH 	ED25519_PRIVATE_KEY_LENGTH



/**
 * Plugin configuration
 ********************************************************************/

/**
 * MiFlora
 *****************************/
#ifndef HAP_PLUGIN_MIFLORA_MAX_DEVICES
#define HAP_PLUGIN_MIFLORA_MAX_DEVICES     			10
#endif

#ifndef HAP_PLUGIN_MIFLORA_SCAN_DURATION
#define HAP_PLUGIN_MIFLORA_SCAN_DURATION    		3       // Max duration of BLE scan (in seconds)
#endif

#ifndef HAP_PLUGIN_MIFLORA_SCAN_INTERVAL
#define HAP_PLUGIN_MIFLORA_SCAN_INTERVAL    		300000
#endif

#ifndef HAP_PLUGIN_MIFLORA_RETRY
#define HAP_PLUGIN_MIFLORA_RETRY       				5		// number of retries to connect to sensor
#endif

#ifndef HAP_PLUGIN_MIFLORA_ENABLE_SCANNER
#define HAP_PLUGIN_MIFLORA_ENABLE_SCANNER 			1		// enable ble scanner to detect mifloras
#endif

#ifndef HAP_PLUGIN_MIFLORA_ENABLE_HISTORY
#define HAP_PLUGIN_MIFLORA_ENABLE_HISTORY			1		// download history from sensor and provide it via fakegato
#endif

#ifndef HAP_PLUGIN_MIFLORA_HISTORY_BATCH_SIZE
#define HAP_PLUGIN_MIFLORA_HISTORY_BATCH_SIZE		7		// Number of history entries to catch in one try
#endif
															// keep this below 8
/**
 * Hygrometer
 *****************************/
#ifndef HAP_PLUGIN_HYGROMTER_REFERENCE
#define HAP_PLUGIN_HYGROMTER_REFERENCE      		2550    // value if put in a glass of water
#endif

#ifndef HAP_PLUGIN_HYGROMETER_PIN_VCC
#define HAP_PLUGIN_HYGROMETER_PIN_VCC       		A6	// 14
#endif

#ifndef HAP_PLUGIN_HYGROMETER_PIN_ADC
#define HAP_PLUGIN_HYGROMETER_PIN_ADC       		A7	// 32
#endif

#ifndef HAP_HYGROMETER_LEAK_SENSOR_ENABLED
#define HAP_HYGROMETER_LEAK_SENSOR_ENABLED  		1		// enable leak sensor for hygrometer
#endif

#ifndef HAP_HYGROMETER_LEAK_PERCENTAGE
#define HAP_HYGROMETER_LEAK_PERCENTAGE				35		// Level when a "leak" notification is triggered
#endif


/**
 * DHT
 *****************************/
#ifndef HAP_PLUGIN_DHT_USE_DUMMY
#define HAP_PLUGIN_DHT_USE_DUMMY 	0
#endif

#ifndef HAP_PLUGIN_DHT_USE_PRESSURE
#define HAP_PLUGIN_DHT_USE_PRESSURE 0
#endif

#ifndef DHTPIN
#define DHTPIN 		A8 	// 15
#endif

#ifndef DHTTYPE
#define DHTTYPE    	DHT22
#endif

/**
 * BME280
 *****************************/
#ifndef HAP_PLUGIN_BME280_USE_DUMMY
#define HAP_PLUGIN_BME280_USE_DUMMY		0	// if 0 then use real sensor, 
#endif  									// if 1 then use random values without any real sensor connected

#ifndef SEALEVELPRESSURE_HPA
#define SEALEVELPRESSURE_HPA 			(1013.25)
#endif


/**
 * Fan Honeywell
 *****************************/
#ifndef HAP_PLUGIN_IR_DEVICE_FREQUENCY
#define HAP_PLUGIN_IR_DEVICE_FREQUENCY 		38000
#endif

#ifndef HAP_PLUGIN_HONEYWELL_DELAY_SEND
#define HAP_PLUGIN_HONEYWELL_DELAY_SEND 	300  // in m
#endif


/**
 * IR
 *****************************/
#ifndef HAP_PLUGIN_IR_SEND_PIN
#define HAP_PLUGIN_IR_SEND_PIN      		14
#endif

#ifndef HAP_PLUGIN_IR_SEND_PIN
#define HAP_PLUGIN_IR_ENABLE_RECV 			1		
#endif

#ifndef HAP_PLUGIN_IR_ENABLE_RECV 
#define HAP_PLUGIN_IR_ENABLE_RECV 			1
#endif


#if HAP_PLUGIN_IR_ENABLE_RECV 

#ifndef HAP_PLUGIN_IR_RECV_PIN
#define HAP_PLUGIN_IR_RECV_PIN				A2
#endif

#ifndef HAP_PLUGIN_IR_RECEIVE_BUFFER_SIZE
#define HAP_PLUGIN_IR_RECEIVE_BUFFER_SIZE	1024
#endif

#ifndef HAP_PLUGIN_IR_RECEIVE_TIMEOUT
#define HAP_PLUGIN_IR_RECEIVE_TIMEOUT		150 // ms
#endif

#ifndef HAP_PLUGIN_IR_RECEIVE_TIMER
#define HAP_PLUGIN_IR_RECEIVE_TIMER			2
#endif

#endif /* HAP_PLUGIN_IR_ENABLE_RECV */


/**
 * NeoPixel
 *****************************/
#ifndef HAP_PLUGIN_NEOPIXEL_NUM_LEDS
#define HAP_PLUGIN_NEOPIXEL_NUM_LEDS 1
#endif

#ifndef HAP_PLUGIN_NEOPIXEL_DATA_PIN
#define HAP_PLUGIN_NEOPIXEL_DATA_PIN A5	
#endif

/**
 * RCSwitch
 *****************************/
#ifndef HAP_PLUGIN_RCSWITCH_PIN
#define HAP_PLUGIN_RCSWITCH_PIN        27  // GPIO27
#endif


#endif /* HAPGLOBALS_HPP_ */