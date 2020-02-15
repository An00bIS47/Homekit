//
// HAPUpdate.cpp
// Homekit
//
//  Created on: 20.08.2017
//      Author: michael
//


#include "HAPUpdate.hpp"

#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include <pgmspace.h>

#include "HAPLogger.hpp"
#include "HAPLimits.hpp"

/*
 *
 * Follow this to create your certificates:
 * https://datacenteroverlords.com/2012/03/01/creating-your-own-ssl-certificate-authority/
 */
#if HAP_UPDATE_SSL_ENABLED
const char * rootCA = "-----BEGIN CERTIFICATE----- \
MIIDYDCCAkigAwIBAgIJALG0wZJ2pi+IMA0GCSqGSIb3DQEBCwUAMEUxCzAJBgNV \
BAYTAkFVMRMwEQYDVQQIDApTb21lLVN0YXRlMSEwHwYDVQQKDBhJbnRlcm5ldCBX \
aWRnaXRzIFB0eSBMdGQwHhcNMTcwODI5MTg1ODMzWhcNMjAwNjE4MTg1ODMzWjBF \
MQswCQYDVQQGEwJBVTETMBEGA1UECAwKU29tZS1TdGF0ZTEhMB8GA1UECgwYSW50 \
ZXJuZXQgV2lkZ2l0cyBQdHkgTHRkMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIB \
CgKCAQEAxV82eUA6s3Il9UbIdPe0ytLfje/5PWIKn198n2TmCDGkdPT+BPuEOERD \
oflaxvHgMLp2M2BAcG4xot4lbMv40pu/klZBpxkumFpJ5zMqsPfJCUCtpnq7yPqi \
INFbxwc1HIfsXXn27aXxgQL+6BmTd6oamSToak7KQha3T3uisPEzgXccH4tZpYRF \
Wp4JLqwIk7nCIwGm1c0Yx0i9zbawD0SKpR2Ks3fNJWbDfklddJet8AuaTkX7nSYL \
S1ZSLT/JdzapzRsy52fuiZKR79xJNlayZexnOOZ2hp+L7BrVTHKVbBsjao5b3n/1 \
lV51tQJ2ngcwO193rySE7oDOc9DhgQIDAQABo1MwUTAdBgNVHQ4EFgQUA8uU5twC \
EfZDn0WGXYkzy3vZ91wwHwYDVR0jBBgwFoAUA8uU5twCEfZDn0WGXYkzy3vZ91ww \
DwYDVR0TAQH/BAUwAwEB/zANBgkqhkiG9w0BAQsFAAOCAQEAVPj+KXI/dsRiksAF \
3e2KhGeqwuc1XG6aN3gXmkcSZKjzklcs+rPEIan5EYFEDtJzlnSeIOwYggUqFXnh \
iwAwoCYL7KMuDtF5Wswjkg1fxtFhk0JE8kukUos6LDWiiGCAkee6ajoEQ5/dExen \
nr5x4h2MylqMhCb0BOCFH72Dpqf1fSEsxPyJI5V6snrZP4kucWGG/w6eCwcOJZuF \
LqDy8iVpXRJ57IDu4/01cudyf0ERqlB1WXs30xMUjEFRwVBAqe2MM/df5iDtGDNV \
1P6SdGOJFkc5jLcF8ZORQlrck+ASjE4BYrTbozIx6hXp6pMSfRgI+nRkr4eZZwdA \
Ulye5w== \
-----END CERTIFICATE-----";

const char * clientCrt = "-----BEGIN CERTIFICATE----- \
MIIDBjCCAe4CCQCMNpjszbmO3DANBgkqhkiG9w0BAQsFADBFMQswCQYDVQQGEwJB \
VTETMBEGA1UECAwKU29tZS1TdGF0ZTEhMB8GA1UECgwYSW50ZXJuZXQgV2lkZ2l0 \
cyBQdHkgTHRkMB4XDTE3MDgyOTE4NTkzMFoXDTE5MDExMTE4NTkzMFowRTELMAkG \
A1UEBhMCQVUxEzARBgNVBAgMClNvbWUtU3RhdGUxITAfBgNVBAoMGEludGVybmV0 \
IFdpZGdpdHMgUHR5IEx0ZDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB \
AKfIkjWDMOw3iszYUZpEwYZUnONoFeasBz2Ijvh0WzuJZCW3DE1KueVDHGNgWkBO \
x2rsECxAILfBo+TFyw39QJF5LmfhOCWNht6SnJsI+ONdk5uAdeJ8KRUhS1pgS26K \
afi703avA8r3P1QBVV75Eq55Mr40DwBNQTbt9XAP+CN4zL9diBfP9N0YedB4HQuw \
+K1KpVtppyCs0miFHAt/x0jB6ApkxmV7mepzMf3GM1TmgNqYhOksTIJItNSw7i9k \
0O7fn8C2qDIhugzqsufoIFLExs5xmIPzIUlKO0HR1SJIr9XtkChMKzLpQYMgo60K \
4uSCwvC8SbM2fzUlAlUapOcCAwEAATANBgkqhkiG9w0BAQsFAAOCAQEAkVLM/3+c \
YFh7IVqNc+3xdMt0rvRTsyTm03NxVw7pNvKmKbc3Djy4Whgp+SI4ku0K5kyq3XbP \
4AKK9/S3GdTIOQzkmodZqr7X8k+uYvQ6/5pHiU2RLicFOjNggvuudtf7YncT0k98 \
Hp+o5MHGl3FqzdhM0nb3G3sXQpTgL0sqmvdVi+M05aoU7ajFKGFeVs9e0T9pw8QX \
5U960+Q1I7r/hU4MFGY8L6T1WxhHmFrwfjZa7QI9Vc7yETnKapwU2wAe7W1Onyg9 \
tXjudG9uFCgadUqpYln4ppPAwsUrS1x1+tfTGOxJi3dDDrSLU8Tv+Q/HdN9j/Xdw \
egSE+/eGczYimQ== \
-----END CERTIFICATE-----";


const char * clientKey = "-----BEGIN RSA PRIVATE KEY----- \
MIIEowIBAAKCAQEAp8iSNYMw7DeKzNhRmkTBhlSc42gV5qwHPYiO+HRbO4lkJbcM \
TUq55UMcY2BaQE7HauwQLEAgt8Gj5MXLDf1AkXkuZ+E4JY2G3pKcmwj4412Tm4B1 \
4nwpFSFLWmBLbopp+LvTdq8Dyvc/VAFVXvkSrnkyvjQPAE1BNu31cA/4I3jMv12I \
F8/03Rh50HgdC7D4rUqlW2mnIKzSaIUcC3/HSMHoCmTGZXuZ6nMx/cYzVOaA2piE \
6SxMgki01LDuL2TQ7t+fwLaoMiG6DOqy5+ggUsTGznGYg/MhSUo7QdHVIkiv1e2Q \
KEwrMulBgyCjrQri5ILC8LxJszZ/NSUCVRqk5wIDAQABAoIBAG4EQYLfREwP226Y \
hWGPpgoMH1ep33qC4LiDe6QEv/HZFBb64REpZ52iFNJA0s0KGw7MPYqWfh2f1nTj \
EVNZ9WWuPTSWZHeTRpVZM2EhT/neWTSE1ketPE8DpuJa8/tGvVgA6RcQiX8kAp11 \
xmHhFlPbWt+HSTLcdV+sRty1S/MNadJkTtwjw46aLBpkBnKeOVOMdXJIpp2Iir/N \
ck2+oHWBxBgsHBJ80Z2scstZak69W34nQhmAQdpP08kTaaPs/YVhMfMsMFHmRELW \
CAqnDwAwhhcuJtRnWvs1eDWXd96zbtkcmIsaHb0f/ff+xCZOQeAgQhN++nV/8O6Z \
Uu+kpYECgYEA1vZge7qptziCxY7F1HSK7qVvlR70GYn6twQH0sLwCMR3M+Ex2Dv/ \
vYinL0bk0ToZ1E6M29m/5p5Ziz6+oCWomT1Lis8/L4+YoLkpLefZqcOE2YT1CWrW \
gvf5ssP69sLV5aTUKE9hwaStrLBgLKeem2pPhpofP2Y0Iw9dLg6voicCgYEAx9B4 \
UFPE3Bpfaut2+vU7qgyWSAJgKUkjO6w2mN7yOlfKO4cEpCVYHeunG9ZTzkZup9gA \
b3qmw8v22MCutz8jVaNxkdKKuNjr6p8z1uh1lbbi0FGd4jdLxIGNtJk4fjLF+SlK \
6YtA2MH8G0QdDql6BwSRYe4OyzX+eq5dzJu6X0ECgYBoQIT6fMTifx2tAlkfeIYI \
x2MeRyzLVgepyuKgbLpNBjtphcXOwQp+uU+bth4y+qKcPJxD448WIaX7yipZMOpb \
p0aDw485WcMD78jw4ojFtHUxqTHNp2rxiaIn+LQ2CLgD77SU+CrI5zzzi3aZMZHS \
ffSjqwuMi9ytRD5EoAq98QKBgCeB4qgPSBc057TeRbItFtTylCw8vAKtwVelH05u \
lnpOrV/DvHkwIRIZa/snD37zkiGBpeRk3eBMzcvhnSoCQ9xE6smoVWFjEpJbVAak \
A8vMU2BZItx9jm+WJgyVVwQsydQQBA3VSMcS4+QIPa6Od+A3WP+B8E7hqCVdoJI6 \
IEmBAoGBAJRI3mMVBF8AmIBMDM7cY+Jf7HXzdKiyiU584kvAQN1KLvDKf4BOi+7j \
fw55TYxZmapbPFudQA0NyOu9OVmMFOzAAc86eo09juZdVBAz19SdxEiv1n1be0pO \
uYBeXTcqYejgCl/R7CH3LGDH4yY7Bz01CwoGEbbWGkWPa/Tqf6Vr \
-----END RSA PRIVATE KEY-----";
#endif


HAPUpdate::HAPUpdate()
{
	_isOTADone = false;
	_contentLength = 0;
	_isValidContentType = false;
	_host = "";
	_port = 5000;
	_previousMillis = 0;
	_interval = 0;
	_available = false;
	_localVersion = nullptr;
}

HAPUpdate::~HAPUpdate() {
	// TODO Auto-generated destructor stub
}

void HAPUpdate::begin(const char* local_hostname) {

#if HAP_UPDATE_ENABLE_OTA	
	// Port defaults to 3232
	// ArduinoOTA.setPort(3232);

	// Hostname defaults to esp3232-[MAC]
	ArduinoOTA.setHostname(local_hostname);

	// No authentication by default
	// ArduinoOTA.setPassword("admin");

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
#endif

#if HAP_UPDATE_ENABLE_SSL
	// Set SSL Options
	_wifiClient.setCACert(rootCA);
	_wifiClient.setCertificate(clientCrt); 		// for client verification
	_wifiClient.setPrivateKey(clientKey);		// for client verification
#endif
}

void HAPUpdate::handle() {

#if HAP_UPDATE_ENABLE_OTA	
	ArduinoOTA.handle();
#endif

#if HAP_UPDATE_ENABLE_FROM_WEB
	if ( millis() - _previousMillis >= _interval) {
	    // save the last time you blinked the LED
	    _previousMillis = millis();
	    _available = checkUpdateAvailable(_localVersion);

	    if (_available) {
	    	execWebupdate();
	    }
	}
#endif

}


bool HAPUpdate::updateAvailable(){
	return _available;
}

bool HAPUpdate::checkUpdateAvailable(HAPVersion* localVersion){

	String url = "/update";

	LogD("\nLocal version: " + localVersion->toString(), true);
	LogD("Connecting to: " + _host + url, true);


	_available = false;

	// Set localVersion if not set
	if (_localVersion == nullptr) {
		_localVersion = localVersion;
	}

	int contentLength = 0;
	bool isValidContentType = false;

	if (_wifiClient.connect(_host.c_str(), _port)) {


		LogD("Fetching firmware info: " + url, true);

		_wifiClient.print(String("GET ") + url + " HTTP/1.1\r\n" +
				"Host: " + _host + "\r\n" +
				"Cache-Control: no-cache\r\n" +
				"Connection: close\r\n\r\n");

		// Check what is being sent
		//    Serial.print(String("GET ") + url + " HTTP/1.1\r\n" +
		//                 "Host: " + host + "\r\n" +
		//                 "Cache-Control: no-cache\r\n" +
		//                 "Connection: close\r\n\r\n");

		
		unsigned long timeout = millis();
	    while (_wifiClient.available() == 0) {
	        if (millis() - timeout > 5000) {
	            LogW(">>> Update Timeout !", true);
	            _wifiClient.stop();
	            return false;
        	}
    	}

		while (_wifiClient.available()) {
			String line = _wifiClient.readStringUntil('\n');
			line.trim();

			if (!line.length()) {
				//headers ended
				break; // and get the OTA started
			}

			if (line.startsWith("HTTP/1.1")) {
				if (line.indexOf("200") < 0) {
					LogE("Got a non 200 status code from server. Exiting web update.", true);
					break;
				}
			}

			// extract headers here
			// Start with content length
			if (line.startsWith("Content-Length: ")) {
				contentLength = atoi((getHeaderValue(line, "Content-Length: ")).c_str());
				//Serial.println("Got " + String(contentLength) + " bytes from server");
			}

			// Next, the content type
			if (line.startsWith("Content-Type: ")) {
				String contentType = getHeaderValue(line, "Content-Type: ");
				//Serial.println("Got " + String(contentType) + " payload.");
				if (contentType == "application/json") {
					isValidContentType = true;
				}
			}
		}
	} else {
		LogE("ERROR: Connection to " + _host + " failed. Please check your setup", true);
		// retry??
		// execOTA();
	}

	LogD("contentLength : " + String(contentLength) + ", isValidContentType : " + String(isValidContentType), true);

	// check contentLength and content type
	if (contentLength && isValidContentType) {
		char res[contentLength];
		_wifiClient.readBytes(res, contentLength);

		const size_t bufferSize = JSON_OBJECT_SIZE(4) + 4 + 2 + MAX_FIRMWARE_NAME_LENGTH + 7 + 2 + MAX_FIRMWARE_VERSION_LENGTH + 5 + 2 + MAX_BRAND_LENGTH + 3 + 2 + 32;

		DynamicJsonDocument root(bufferSize);		
		DeserializationError error = deserializeJson(root, res);

		if (error) {
			LogE( F("ERROR: Parsing Firmware failed"), true);
			return false;
		}

		const char* name = root["name"].as<char*>();
		const char* version = root["version"].as<char*>();
		const char* brand = root["brand"].as<char*>();
		const char* md5 = root["md5"].as<char*>();

		_remoteVersion = HAPVersion(version);
		//_remoteVersion.toString();

		LogD("Remote name:    " + String(name), true);
		LogD("Remote version: " + String(version), true);
		LogD("Remote brand:   " + String(brand), true);
		LogD("Remote md5:     " + String(md5), true);

		if (*localVersion < _remoteVersion) {
			LogI("Update available!", true);
			_available = true;

			strncpy(_expectedMd5, md5, 32);

			return true;
		} else {
			return false;
		}


	} else {
		LogW( F("There was no content in the response"), true);
		_wifiClient.flush();
	}

	return false;
}

void HAPUpdate::execWebupdate() {
	int contentLength = 0;
	bool isValidContentType = false;

	String url = "/update/firmware";
	LogD("Connecting to: " + _host + url, true);
	// Connect to S3
	if (_wifiClient.connect(_host.c_str(), _port)) {
		// Connection Succeed.
		// Fecthing the bin
		LogD("Fetching bin: " + url, true );

		// Get the contents of the bin file
		_wifiClient.print(String("GET ") + url + " HTTP/1.1\r\n" +
				"Host: " + _host + "\r\n" +
				"Cache-Control: no-cache\r\n" +
				"Connection: close\r\n\r\n");

		// Check what is being sent
		//    Serial.print(String("GET ") + bin + " HTTP/1.1\r\n" +
		//                 "Host: " + host + "\r\n" +
		//                 "Cache-Control: no-cache\r\n" +
		//                 "Connection: close\r\n\r\n");

		unsigned long timeout = millis();
		while (_wifiClient.available() == 0) {
			delay(1);
			if (millis() - timeout > HAP_UPDATE_TIMEOUT) {
				LogW( F("Client Timeout !"), true);
				_wifiClient.stop();
				return;
			}
			yield();
		}
		// Once the response is available,
		// check stuff

		/*
       Response Structure
        HTTP/1.1 200 OK
        x-amz-id-2: NVKxnU1aIQMmpGKhSwpCBh8y2JPbak18QLIfE+OiUDOos+7UftZKjtCFqrwsGOZRN5Zee0jpTd0=
        x-amz-request-id: 2D56B47560B764EC
        Date: Wed, 14 Jun 2017 03:33:59 GMT
        Last-Modified: Fri, 02 Jun 2017 14:50:11 GMT
        ETag: "d2afebbaaebc38cd669ce36727152af9"
        Accept-Ranges: bytes
        Content-Type: application/octet-stream
        Content-Length: 357280
        Server: AmazonS3

        {{BIN FILE CONTENTS}}
		 */
		while (_wifiClient.available()) {
			// read line till /n
			String line = _wifiClient.readStringUntil('\n');
			// remove space, to check if the line is end of headers
			line.trim();

			// if the the line is empty,
			// this is end of headers
			// break the while and feed the
			// remaining `client` to the
			// Update.writeStream();
			if (!line.length()) {
				//headers ended
				break; // and get the OTA started
			}

			// Check if the HTTP Response is 200
			// else break and Exit Update
			if (line.startsWith("HTTP/1.1")) {
				if (line.indexOf("200") < 0) {
					LogD( F("Got a non 200 status code from server. Exiting OTA Update."), true);
					break;
				}
			}

			// extract headers here
			// Start with content length
			if (line.startsWith("Content-Length: ")) {
				contentLength = atoi((getHeaderValue(line, "Content-Length: ")).c_str());
				LogD("Got " + String(contentLength) + " bytes from server",  true);
			}

			// Next, the content type
			if (line.startsWith("Content-Type: ")) {
				String contentType = getHeaderValue(line, "Content-Type: ");
				LogD("Got " + contentType + " payload.", true);
				if (contentType == "application/octet-stream") {
					isValidContentType = true;
				}
			}
		}
	} else {
		// Connect to S3 failed
		// May be try?
		// Probably a choppy network?
		LogE("Connection to " + String(_host) + " failed. Please check your setup", true);
		// retry??
		// execWebupdate();
	}

	// Check what is the contentLength and if content type is `application/octet-stream`
	LogD("contentLength : " + String(contentLength) + ", isValidContentType : " + String(isValidContentType), true);

	// check contentLength and content type
	if (contentLength && isValidContentType) {
		// Check if there is enough to OTA Update
		bool canBegin = Update.begin(contentLength);


		Update.setMD5(_expectedMd5);

		// If yes, begin
		if (canBegin) {
			LogI( F("Begin OTA Web Update. This may take 2 - 5 mins to complete.\nThings might be quite for a while.. Patience !!!"), true);
			// No activity would appear on the Serial monitor
			// So be patient. This may take 2 - 5mins to complete

			Update.onProgress([](unsigned int progress, unsigned int total) {
				Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
			});

			size_t written = Update.writeStream(_wifiClient);

			if (written == contentLength) {
				LogD("Written : " + String(written) + "bytes successfully", true);
			} else {
				LogW("Written only : " + String(written) + "/" + String(contentLength) + ". Retry?", true );
				// retry??
				// execOTA();
			}

			if (Update.end()) {

				Serial.println( "md5 of firmware binary: " + Update.md5String() );

				//Serial.println("OTA done!");
				if (Update.isFinished()) {
					LogI( F("Update successfully completed. Rebooting."), true);
					ESP.restart();
				} else {
					LogE( F("Update not finished? Something went wrong!"), true);
				}
			} else {
				LogE("[ERROR] Error code #: " + String(Update.getError()), true);
			}
		} else {
			// not enough space to begin OTA
			// Understand the partitions and
			// space availability
			LogE( F("Not enough space to begin OTA"), true);
			_wifiClient.flush();
		}
	} else {
		LogE( F("There was no content in the response"), true);
		_wifiClient.flush();
	}
}

// Utility to extract header value from headers
String HAPUpdate::getHeaderValue(String header, String headerName) {
	return header.substring(strlen(headerName.c_str()));
}
