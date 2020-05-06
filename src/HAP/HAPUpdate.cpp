//
// HAPUpdate.cpp
// Homekit
//
//  Created on: 20.08.2017
//      Author: michael
//
#include "HAPUpdate.hpp"
#include <WiFiClientSecure.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include "HAPLogger.hpp"
#include "HAPBonjour.hpp"


#if HAP_UPDATE_ENABLE_WEB
#include <HTTPUpdate.h>



#if HAP_UPDATE_ENABLE_SSL
#define HAP_UPDATE_HTTP				"https://"
#else
#define HAP_UPDATE_HTTP				"http://"
#endif

#define HAP_UPDATE_WEB_CHECK_URL_PATH		"/api/update"
#define HAP_UPDATE_WEB_DOWNLOAD_URL_PATH	"/api/update"

// ToDo: remove this here 
// ToDo: Add password protection for OTA Update
const char* rootCACertificate = \
"-----BEGIN CERTIFICATE-----\n" \
"MIICvzCCAiCgAwIBAgIBBDAKBggqhkjOPQQDAjBkMRAwDgYDVQQDDAdBQ01FIENB\n" \
"MRAwDgYDVQQIDAdCYXZhcmlhMQswCQYDVQQGEwJERTEPMA0GA1UEBwwGTXVuaWNo\n" \
"MSAwHgYJKoZIhvcNAQkBFhFhY21lQGJvdXJuZS5sb2NhbDAeFw0yMDAyMTUwMDMw\n" \
"NDlaFw00MDAyMTAwMDMwNDlaMB4xHDAaBgNVBAMME3VwZGF0ZV9zZXJ2ZXIubG9j\n" \
"YWwwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDK25Hnbc1PnyORPDSo\n" \
"W0Z5gbRuRuYB9RCM/yBXblaRVt4Rg/6iE/YjFUe8/EKkjzzJtXGdYMMZkF4e99tf\n" \
"FLu8RYHhMocsvgeTm5b0/anvyobWtZ33dU4yBQNN5S2CZ3uuVIGQ94jx3Y8U2o+C\n" \
"1fxuxtU8FanYVn3J3mNGbDD7Q4NF/ZgGWSumScWklk/u/+HO+i4jP0woYx5lypHO\n" \
"O92qqufC/SbHBoAkpBV3Z0bHSg3eu9xRlefv/fNBNZRHZeOh9QZnAROVPEM3nIBj\n" \
"7SIwYJn+72tucyOm6xbueL7kPGi9F214JMg1ReFWwv8ToCoBOE5IVM4lYBBmaTwG\n" \
"3tZRAgMBAAGjPjA8MA4GA1UdDwEB/wQEAwIHgDAqBgNVHSUBAf8EIDAeBggrBgEF\n" \
"BQcDAgYIKwYBBQUHAwEGCCsGAQUFBwMDMAoGCCqGSM49BAMCA4GMADCBiAJCAcMr\n" \
"3iPZctGVRiuT4+gmOC/pRtEW7WeHLFYU4OQoTX0IMQtiFtqMIvaXHnpKQofCnDts\n" \
"2gC56YiRj+ZKz6mFvGnMAkIBVWWkaPOOSubinwN4IynqyvjyMFsmKL2I6PQT0W4o\n" \
"ezhOeWBTYCuEbryvRU4CKhTwiQVjpAZJPG+l2TzqZMhSr5A=\n" \
"-----END CERTIFICATE-----\n";

#endif

HAPUpdate::HAPUpdate()
{
	_isOTADone = false;
	// _contentLength = 0;
	// _isValidContentType = false;
	// _host = "";
	_port = 3001;
	_previousMillis = 0;
	_interval = 0;
	_available = false;
	_remoteInfo = HAPUpdateVersionInfo();
	// _localVersion = nullptr;
}

HAPUpdate::~HAPUpdate() {
	// TODO Auto-generated destructor stub
}

void HAPUpdate::begin(HAPConfig* config) {

#if HAP_UPDATE_ENABLE_OTA	

	// Disable mDNS cause we use our own!
	ArduinoOTA.setMdnsEnabled(false);

	// Hostname defaults to esp3232-[MAC]
	ArduinoOTA.setHostname(config->config()["hostname"]);

	// Port defaults to 3232
	ArduinoOTA.setPort(config->config()["update"]["ota"]["port"].as<uint16_t>());

	// No authentication by default
	if (config->config()["update"]["ota"]["password"].as<String>().length() > 0) {
		ArduinoOTA.setPassword(config->config()["update"]["ota"]["password"].as<const char*>());
	}
	
	// Password can be set with it's md5 value as well
	// MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
	// ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

	ArduinoOTA.onStart([]() {
		String type;
		if (ArduinoOTA.getCommand() == U_FLASH)
			type = "sketch";
		else // U_SPIFFS
			type = "filesystem";

		// NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
		LogV("Start updating " + String(type), true);
	});
	ArduinoOTA.onEnd([]() {
		LogV( F("\nUpdate complete!"), true);
	});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
	});
	ArduinoOTA.onError([](ota_error_t error) {
		LogE( F("ERROR: Updated failed! - "), false);
		if (error == OTA_AUTH_ERROR) 			LogE( F("Auth Failed"), true);
		else if (error == OTA_BEGIN_ERROR) 		LogE( F("Begin Failed"), true);
		else if (error == OTA_CONNECT_ERROR) 	LogE( F("Connect Failed"), true);
		else if (error == OTA_RECEIVE_ERROR) 	LogE( F("Receive Failed"), true);
		else if (error == OTA_END_ERROR) 		LogE( F("End Failed"), true);
	});

	ArduinoOTA.begin();

	// set ArduinoOTA mDNS
	mDNSExt.enableArduino(config->config()["update"]["ota"]["port"].as<uint16_t>(), (config->config()["update"]["ota"]["password"].as<String>().length() > 0));

#endif	
	// Delay first update check for 3 seconds
	_previousMillis = (millis() + HAP_UPDATE_WEB_INTERVAL) - 3000;
}

void HAPUpdate::handle() {

#if HAP_UPDATE_ENABLE_OTA	
	ArduinoOTA.handle();
#endif

#if HAP_UPDATE_ENABLE_FROM_WEB
	if ( millis() - _previousMillis >= _interval) {
	    // save the last time you blinked the LED
	    _previousMillis = millis();
	    _available = checkUpdateAvailable();

	    // if (_available) {
	    // 	execWebupdate();
	    // }
	}
#endif

}


bool HAPUpdate::updateAvailable(){
	return _available;
}


#if HAP_UPDATE_ENABLE_FROM_WEB
bool HAPUpdate::checkUpdateAvailable(){
	
	String url = String(HAP_UPDATE_HTTP) + _host + String(HAP_UPDATE_WEB_CHECK_URL_PATH);	
	LogD("Local version: " + HAPVersion(HOMEKIT_VERSION).toString(), true);

	uint32_t rev = (uint32_t)strtol(HAP_PLUGIN_FEATURE_NUMBER, NULL, 2);
	char hexString[20]; // long enough for any 32-bit value, 4-byte aligned
	sprintf(hexString, "%x", rev);

	const size_t capacity = JSON_OBJECT_SIZE(5);
	DynamicJsonDocument doc(capacity);

	doc["name"] = "Homekit";
	doc["version"] = HAPVersion(HOMEKIT_VERSION).toString();
	doc["feature_rev"] = hexString;
	doc["brand"] = "An00bIS47";

	String postData;
	serializeJson(doc, postData);

	LogD("Post data: " + postData, true);	
	
	_http = new HTTPClient();
	
	_http->begin(_host, _port , "/api/update", rootCACertificate); //HTTPS example connection
	_http->addHeader("Content-Type", "application/json");

	int httpCode = _http->POST((uint8_t*)postData.c_str(), postData.length());
	
	if (httpCode > 0) {
		LogD("Got response code: " + String(httpCode), true);
		//file found at server --> on unsucessful connection code will be -1
		if (httpCode == HTTP_CODE_OK) {
			String payload = _http->getString();
			LogD("Got payload: " + payload, true);
						
			// // Parse response
			const size_t capResp = JSON_OBJECT_SIZE(6) + 256;
			DynamicJsonDocument resp(capResp);
			deserializeJson(resp, payload);

			_remoteInfo.version = HAPVersion(resp["version"].as<const char*>());
			_remoteInfo.md5 = resp["md5"].as<const char*>();			
			_remoteInfo.featureRev = (uint32_t)strtol(resp["feature_rev"].as<const char*>(), NULL, 16);
			_remoteInfo.size = resp["firmware_size"].as<uint32_t>();

			LogI("Update available : " + _remoteInfo.version.toString(), true);

			_available = true;
		} else if (httpCode == HTTP_CODE_NO_CONTENT) {
			_available = false;
		}
	} else {
		LogE("ERROR: Check online update failed! Response code: " + _http->errorToString(httpCode), true);
	}
	_http->end();

	delete _http;
	
	return _available;
}

void HAPUpdate::execWebupdate() {

	    WiFiClientSecure client;
		client.setCACert(rootCACertificate);

		// Reading data over SSL may be slow, use an adequate timeout
		client.setTimeout(12000 / 1000); // timeout argument is defined in seconds for setTimeout

		// The line below is optional. It can be used to blink the LED on the board during flashing
		// The LED will be on during download of one buffer of data from the network. The LED will
		// be off during writing that buffer to flash
		// On a good connection the LED should flash regularly. On a bad connection the LED will be
		// on much longer than it will be off. Other pins than LED_BUILTIN may be used. The second
		// value is used to put the LED on. If the LED is on with HIGH, that value should be passed
		// httpUpdate.setLedPin(LED_BUILTIN, HIGH);

		// t_httpUpdate_return ret = httpUpdate.update(client, "https://homebridge/api/update");
		// Or:

		String query = "/api/update?";
		query += "name=" + String("Homekit") + "&";
		query += "brand=" + String(HAP_MANUFACTURER) + "&";
		query += "version=" + onlineVersion() + "&";		
		query += "feature_rev=" + String(_remoteInfo.featureRev, 16);		
		
		// ToDo: urlencode query		
		t_httpUpdate_return ret = httpUpdate.update(client, HAP_UPDATE_SERVER_HOST, HAP_UPDATE_SERVER_PORT, query);

		switch (ret) {
			case HTTP_UPDATE_FAILED:
				Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
				break;

			case HTTP_UPDATE_NO_UPDATES:
				Serial.println("HTTP_UPDATE_NO_UPDATES");
				break;

			case HTTP_UPDATE_OK:
				Serial.println("HTTP_UPDATE_OK");
				break;
		}

// 	int contentLength = 0;
// 	bool isValidContentType = false;

// 	String url = "/update/firmware";
// 	LogD("Connecting to: " + _host + url, true);
// 	// Connect to S3
// 	if (_wifiClient.connect(_host.c_str(), _port)) {
// 		// Connection Succeed.
// 		// Fecthing the bin
// 		LogD("Fetching bin: " + url, true );

// 		// Get the contents of the bin file
// 		_wifiClient.print(String("GET ") + url + " HTTP/1.1\r\n" +
// 				"Host: " + _host + "\r\n" +
// 				"Cache-Control: no-cache\r\n" +
// 				"Connection: close\r\n\r\n");

// 		// Check what is being sent
// 		//    Serial.print(String("GET ") + bin + " HTTP/1.1\r\n" +
// 		//                 "Host: " + host + "\r\n" +
// 		//                 "Cache-Control: no-cache\r\n" +
// 		//                 "Connection: close\r\n\r\n");

// 		unsigned long timeout = millis();
// 		while (_wifiClient.available() == 0) {
// 			delay(1);
// 			if (millis() - timeout > HAP_UPDATE_TIMEOUT) {
// 				LogW( F("Client Timeout !"), true);
// 				_wifiClient.stop();
// 				return;
// 			}
// 			yield();
// 		}
// 		// Once the response is available,
// 		// check stuff

// 		/*
//        Response Structure
//         HTTP/1.1 200 OK
//         x-amz-id-2: NVKxnU1aIQMmpGKhSwpCBh8y2JPbak18QLIfE+OiUDOos+7UftZKjtCFqrwsGOZRN5Zee0jpTd0=
//         x-amz-request-id: 2D56B47560B764EC
//         Date: Wed, 14 Jun 2017 03:33:59 GMT
//         Last-Modified: Fri, 02 Jun 2017 14:50:11 GMT
//         ETag: "d2afebbaaebc38cd669ce36727152af9"
//         Accept-Ranges: bytes
//         Content-Type: application/octet-stream
//         Content-Length: 357280
//         Server: AmazonS3

//         {{BIN FILE CONTENTS}}
// 		 */
// 		while (_wifiClient.available()) {
// 			// read line till /n
// 			String line = _wifiClient.readStringUntil('\n');
// 			// remove space, to check if the line is end of headers
// 			line.trim();

// 			// if the the line is empty,
// 			// this is end of headers
// 			// break the while and feed the
// 			// remaining `client` to the
// 			// Update.writeStream();
// 			if (!line.length()) {
// 				//headers ended
// 				break; // and get the OTA started
// 			}

// 			// Check if the HTTP Response is 200
// 			// else break and Exit Update
// 			if (line.startsWith("HTTP/1.1")) {
// 				if (line.indexOf("200") < 0) {
// 					LogD( F("Got a non 200 status code from server. Exiting OTA Update."), true);
// 					break;
// 				}
// 			}

// 			// extract headers here
// 			// Start with content length
// 			if (line.startsWith("Content-Length: ")) {
// 				contentLength = atoi((getHeaderValue(line, "Content-Length: ")).c_str());
// 				LogD("Got " + String(contentLength) + " bytes from server",  true);
// 			}

// 			// Next, the content type
// 			if (line.startsWith("Content-Type: ")) {
// 				String contentType = getHeaderValue(line, "Content-Type: ");
// 				LogD("Got " + contentType + " payload.", true);
// 				if (contentType == "application/octet-stream") {
// 					isValidContentType = true;
// 				}
// 			}
// 		}
// 	} else {
// 		// Connect to S3 failed
// 		// May be try?
// 		// Probably a choppy network?
// 		LogE("Connection to " + String(_host) + " failed. Please check your setup", true);
// 		// retry??
// 		// execWebupdate();
// 	}

// 	// Check what is the contentLength and if content type is `application/octet-stream`
// 	LogD("contentLength : " + String(contentLength) + ", isValidContentType : " + String(isValidContentType), true);

// 	// check contentLength and content type
// 	if (contentLength && isValidContentType) {
// 		// Check if there is enough to OTA Update
// 		bool canBegin = Update.begin(contentLength);


// 		Update.setMD5(_expectedMd5);

// 		// If yes, begin
// 		if (canBegin) {
// 			LogI( F("Begin OTA Web Update. This may take 2 - 5 mins to complete.\nThings might be quite for a while.. Patience !!!"), true);
// 			// No activity would appear on the Serial monitor
// 			// So be patient. This may take 2 - 5mins to complete

// 			Update.onProgress([](unsigned int progress, unsigned int total) {
// 				Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
// 			});

// 			size_t written = Update.writeStream(_wifiClient);

// 			if (written == contentLength) {
// 				LogD("Written : " + String(written) + "bytes successfully", true);
// 			} else {
// 				LogW("Written only : " + String(written) + "/" + String(contentLength) + ". Retry?", true );
// 				// retry??
// 				// execOTA();
// 			}

// 			if (Update.end()) {

// 				Serial.println( "md5 of firmware binary: " + Update.md5String() );

// 				//Serial.println("OTA done!");
// 				if (Update.isFinished()) {
// 					LogI( F("Update successfully completed. Rebooting."), true);
// 					ESP.restart();
// 				} else {
// 					LogE( F("Update not finished? Something went wrong!"), true);
// 				}
// 			} else {
// 				LogE("[ERROR] Error code #: " + String(Update.getError()), true);
// 			}
// 		} else {
// 			// not enough space to begin OTA
// 			// Understand the partitions and
// 			// space availability
// 			LogE( F("Not enough space to begin OTA"), true);
// 			_wifiClient.flush();
// 		}
// 	} else {
// 		LogE( F("There was no content in the response"), true);
// 		_wifiClient.flush();
// 	}
}

#endif

// Utility to extract header value from headers
// String HAPUpdate::getHeaderValue(String header, String headerName) {
// 	return header.substring(strlen(headerName.c_str()));
// }
