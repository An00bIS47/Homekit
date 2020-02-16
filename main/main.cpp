//
// main.cpp
// Homekit
//
//  Created on: 08.08.2017
//      Author: michael
//
#include <Arduino.h>

#include "HAP/HAPLogger.hpp"
#include "HAP/HAPServer.hpp"
#include "HAP/HAPGlobals.hpp"

#include "HAP/HAPHelper.hpp"
#include "HAP/HAPVersion.hpp"



#if HAP_ENABLE_WEBSERVER_CORE_0
#include "HAP/HAPWebServer.hpp"
HAPWebServer* _webserver;
#endif



// unsigned long previousMillis = 0;

// const long interval = 1000;

#if HAP_ENABLE_WEBSERVER_CORE_0
void taskWebserver( void * parameter )
{
	_webserver = new HAPWebServer();
	_webserver->setAccessorySet(hap.getAccessorySet());
	_webserver->begin();

    while( true ){
        _webserver->handle();
        delay(1);		
    }
 
    // Serial.println("Ending task 1");
    // vTaskDelete( NULL );
}
#endif


void setup(){

	Serial.begin(115200);

	// Imprint infos to firmware
	Homekit_setFirmware("Homekit", HOMEKIT_VERSION, HOMEKIT_FEATURE_REV);
	Homekit_setBrand(HAP_MANUFACTURER);

	LogI( F("Starting Homekit "), false);
	LogI( hap.versionString() + String( " ..."), true);
	LogI( F("Log level: "), false);
	LogI( String(HAPLogger::getLogLevel() ), true);

	// Start homekit
	hap.begin();

#if HAP_ENABLE_WEBSERVER_CORE_0
	xTaskCreatePinnedToCore(
					taskWebserver,   /* Function to implement the task */
                    "coreTask", /* Name of the task */
                    8192,      	/* Stack size in words */
                    NULL,       /* Task input parameter */
                    1,          /* Priority of the task */
                    NULL,       /* Task handle. */
                    0);  		/* Core where the task should run */
#endif
	
}

void loop(){

	hap.handle();

}


