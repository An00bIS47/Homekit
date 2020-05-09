//
// HAPPluginSSD1306.cpp
// Homekit
//
//  Created on: 29.04.2018
//      Author: michael
//

#include "HAPPluginSSD1306.hpp"
#include "HAPServer.hpp"

#if HAP_PLUGIN_USE_SSD1306

#include "qrcode.h"


#define QR_CODE_VERSION         3   // see the sizes of the qr code version here: https://github.com/ricmoo/QRCode
#define QR_CODE_ZOOM_FACTOR     2

#define QR_CODE_X_OFFSET        35  // (96   -   (29        * 2) ) / 2 = 19
                                    // (128 -    (29        * 2) ) / 2 = 35
                                    // width - (qrcode size * 2)   / 2 = offset

#define QR_CODE_Y_OFFSET        3   // (64   -   (29        * 2) ) / 2 = 3


#define VERSION_MAJOR       0
#define VERSION_MINOR       0
#define VERSION_REVISION    3
#define VERSION_BUILD       2

#define TEXT_OFFSET_X      20
#define TEXT_OFFSET_Y       5

HAPPluginSSD1306::HAPPluginSSD1306(){

    _type = HAP_PLUGIN_TYPE_DISPLAY; // ToDo: deprecated
	_name = "SSD1306";
	_isEnabled = HAP_PLUGIN_USE_SSD1306;
	_interval = HAP_PLUGIN_SSD_INTERVAL;
	_previousMillis = 0;	    

    _updateDisplay = true;
    _currentScreen = 0;
    _numberOfScreens = 0;

    _version.major      = VERSION_MAJOR;
    _version.minor      = VERSION_MINOR;
    _version.revision   = VERSION_REVISION;
    _version.build      = VERSION_BUILD;


    // _tft->setBackground(BLACK);
    // _tft->setForeground(WHITE);
}

HAPPluginSSD1306::~HAPPluginSSD1306(){
    delete _tft;
}

bool HAPPluginSSD1306::begin(){

    // Heltec Lib
    // _tft = new SSD1306Wire(0x3c, SDA_OLED, SCL_OLED, RST_OLED, GEOMETRY_128_64);

    // Thingpulse
    _tft = new SSD1306Wire(0x3c, SDA_OLED, SCL_OLED, GEOMETRY_128_64);

    // OLED reset signal 
    pinMode(RST_OLED,OUTPUT); digitalWrite(RST_OLED, LOW); delay(50); digitalWrite(RST_OLED, HIGH);

    

    _tft->init();
    _tft->flipScreenVertically();

    if (_accessorySet->isPaired()) {
        _tft->setFont(ArialMT_Plain_10);
        _tft->drawString(22, 10, "Homekit started");
    } else {
        displayQRCode();
    }     

    _tft->display();

    
    return true;
}

HAPAccessory* HAPPluginSSD1306::initAccessory(){

	return nullptr;
}

void HAPPluginSSD1306::handleEvents(int eventCode, struct HAPEvent eventParam){

    if (_isEnabled) {        

        LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Handle event [code:" + String(eventCode) + "]", true);

        _tft->setFont(ArialMT_Plain_10);

        if (eventCode == EventManager::kEventPairingStep1){        
            _tft->clear();
            _tft->drawString(TEXT_OFFSET_Y, TEXT_OFFSET_X, "Pairing Step 1!");
            updateProgressbar(0);
            _tft->display();
            _updateDisplay = true;
            _previousMillis = millis();
        } 

        else if (eventCode == EventManager::kEventPairingStep2){        
            _tft->clear();
            _tft->drawString(TEXT_OFFSET_Y, TEXT_OFFSET_X, "Pairing Step 2!");
            updateProgressbar(25);
            _tft->display();
            _updateDisplay = true;
            _previousMillis = millis();
        }

        else if (eventCode == EventManager::kEventPairingStep3){        
            _tft->clear();
            _tft->drawString(TEXT_OFFSET_Y, TEXT_OFFSET_X, "Pairing Step 3!");
            updateProgressbar(50);
            _tft->display();
            _updateDisplay = true;
            _previousMillis = millis();
        }

        else if (eventCode == EventManager::kEventPairingStep4){        
            _tft->clear();
            _tft->drawString(TEXT_OFFSET_Y, TEXT_OFFSET_X, "Pairing Step 4!");
            updateProgressbar(75);
            _tft->display();
            _updateDisplay = true;
            _previousMillis = millis();
        }

        else if (eventCode == EventManager::kEventPairingComplete){        
            _tft->clear();
            _tft->drawString(TEXT_OFFSET_Y, TEXT_OFFSET_X, "Pairing complete!");
            updateProgressbar(100);
            _tft->display();
            _updateDisplay = true;
            _previousMillis = millis();
        }

        

        else if (eventCode == EventManager::kEventVerifyStep1){        
            _tft->clear();
            _tft->drawString(TEXT_OFFSET_Y, TEXT_OFFSET_X, "Verify Step 1!");
            updateProgressbar(0);
            _tft->display();
            _updateDisplay = true;
            _previousMillis = millis();
        }

        else if (eventCode == EventManager::kEventVerifyStep2){        
            _tft->clear();
            _tft->drawString(TEXT_OFFSET_Y, TEXT_OFFSET_X, "Verify Step 2!");
            updateProgressbar(50);
            _tft->display();
            _updateDisplay = true;
            _previousMillis = millis();
        }

        else if (eventCode == EventManager::kEventVerifyComplete){        
            _tft->clear();
            _tft->drawString(TEXT_OFFSET_Y, TEXT_OFFSET_X, "Verify complete!");
            updateProgressbar(100);
            _tft->display();
            _updateDisplay = true;
            _previousMillis = millis();
        }

        else if (eventCode == EventManager::kEventErrorOccurred){        
            _tft->clear();
            _tft->drawString(TEXT_OFFSET_Y, TEXT_OFFSET_X, "ERROR");            
            _tft->display();
            _updateDisplay = true;
            _previousMillis = millis();
        }

        else if (eventCode == EventManager::kEventRebootNow){        
            _tft->clear();
            _tft->drawString(TEXT_OFFSET_Y, TEXT_OFFSET_X, "!!! REBOOTING NOW !!!");            
            _tft->display();
            _updateDisplay = true;
            _previousMillis = millis();
            
        }

        else if ( (eventCode == EventManager::kEventHomekitStarted) || (eventCode == EventManager::kEventAllPairingsRemoved) ) {        
            if (!_accessorySet->isPaired()){
                _tft->clear();
                displayQRCode();
                _tft->display();   
                _updateDisplay = false;                                             
            }
            setupScreens();
            // _previousMillis = millis() - 1800;
        }

        
    }

}


void HAPPluginSSD1306::handleImpl(bool forced){
    
    LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Handle plguin [" + String(_interval) + "]", true);
    
    if (_updateDisplay) {            
        _tft->clear();
    }

    if ( (_accessorySet->isPaired() ) && (_numberOfScreens > 0) ) {
        struct screenInfo s = _screenMap[_currentScreen];

        // Serial.println("_numberOfScreens: " + String(_numberOfScreens));
        // Serial.println("acc Name: " + s.accessoryName);
        // Serial.println("name: " + s.name);
        // Serial.println("aid: " + String(s.aid));
        // Serial.println("iid: " + String(s.iid));
                
        characteristics* curChar = _accessorySet->getCharacteristics(s.aid, s.iid);

        _tft->setFont(ArialMT_Plain_16);
        _tft->drawString(32, 32, curChar->value());
        _tft->display();   
        
        _updateDisplay = true;
        _currentScreen++;

        if (_currentScreen >= _numberOfScreens){
            _currentScreen = 0;
        }

    } else if (_numberOfScreens == 0) {
        setupScreens();
    }
    
}

void HAPPluginSSD1306::displayQRCode(){

    LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Display QR code", true);
    //
    // Show qr code for pairing
    //
    QRCode _qrcode;        
    uint8_t qrcodeData[qrcode_getBufferSize(QR_CODE_VERSION)];
    qrcode_initText(&_qrcode, qrcodeData, QR_CODE_VERSION, ECC_MEDIUM, _accessorySet->xhm());        


#if 0
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

    int k = 0;         
    for (uint8_t y = 0; y < _qrcode.size; y++) {
        int n = 0;
        for (uint8_t x = 0; x < _qrcode.size; x++) {
            if (qrcode_getModule(&_qrcode, x, y) ) {                    
                _tft->drawRect(x + QR_CODE_X_OFFSET + (n * QR_CODE_ZOOM_FACTOR) - n, y + QR_CODE_Y_OFFSET + (k * QR_CODE_ZOOM_FACTOR) - k, QR_CODE_ZOOM_FACTOR, QR_CODE_ZOOM_FACTOR );
            } else {
                //_tft->setPixel(x + QR_CODE_X_OFFSET + (n * QR_CODE_ZOOM_FACTOR) - n, y + QR_CODE_Y_OFFSET + (k * QR_CODE_ZOOM_FACTOR) - k); //, QR_CODE_ZOOM_FACTOR, QR_CODE_ZOOM_FACTOR );
            }
            n++;
        }            
        k++;            
    }
}

void HAPPluginSSD1306::addEventListener(EventManager* eventManager){

    if (_isEnabled) {
        LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Add listerner to event manager", true);

        _listenerMemberFunctionPlugin.mObj = this;
        _listenerMemberFunctionPlugin.mf = &HAPPlugin::handleEvents;
        
        // Add listener to event manager
        _eventManager = eventManager;
        // eventManager->setDefaultListener(&_listenerMemberFunctionPlugin);

        // for accessory notifications and values
        // _eventManager->addListener( EventManager::kEventNotifyAccessory,    &_listenerMemberFunctionPlugin );
        // _eventManager->addListener( EventManager::kEventNotifyController,   &_listenerMemberFunctionPlugin );		

        _eventManager->addListener( EventManager::kEventHomekitStarted,     &_listenerMemberFunctionPlugin );
        
        _eventManager->addListener( EventManager::kEventPairingStep1,       &_listenerMemberFunctionPlugin );		
        _eventManager->addListener( EventManager::kEventPairingStep2,       &_listenerMemberFunctionPlugin );			
        _eventManager->addListener( EventManager::kEventPairingStep3,       &_listenerMemberFunctionPlugin );			
        _eventManager->addListener( EventManager::kEventPairingStep4,       &_listenerMemberFunctionPlugin );		
        _eventManager->addListener( EventManager::kEventPairingComplete,    &_listenerMemberFunctionPlugin );	
        
        _eventManager->addListener( EventManager::kEventVerifyStep1,        &_listenerMemberFunctionPlugin );			
        _eventManager->addListener( EventManager::kEventVerifyStep2,        &_listenerMemberFunctionPlugin );
        _eventManager->addListener( EventManager::kEventVerifyComplete,     &_listenerMemberFunctionPlugin );			

        
        _eventManager->addListener( EventManager::kEventErrorOccurred,      &_listenerMemberFunctionPlugin );			
        _eventManager->addListener( EventManager::kEventRebootNow,          &_listenerMemberFunctionPlugin );			
    }
    
}

void HAPPluginSSD1306::setupScreens(){
    LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Setup screens ... ", false);
    int n = 0;
    
    // first accessory is bridge -> don't write
    for (int i=1; i < _accessorySet->numberOfAccessory(); i++){
        HAPAccessory* curAcc = _accessorySet->accessoryAtIndex(i);
        // Serial.print("Acc: " + curAcc->getName() + " - ");			
        // Serial.println(curAcc->numberOfService());

        String accName = curAcc->name();						
        
        // first service is info service -> don't write
        for (int j=1; j < curAcc->numberOfService(); j++){
            HAPService* curSer = curAcc->serviceAtIndex(j);
            
            String name;


            for (int k=0; k < curSer->numberOfCharacteristics(); k++){
                characteristics *curChar = curSer->characteristicsAtIndex(k);

                if (curChar->type == HAP_CHARACTERISTIC_NAME){
                    // serviceName is first -> don't write into db
                    // Serial.println("-- " + curChar->value());

                    name = curChar->value();	

                    //Serial.println(name);	
                } else if (curChar->typeString == HAP_SERVICE_FAKEGATO_HISTORY) {
                    // Exclude fakegato history service (all others are hidden)                                            				
                } else {
                    // all other chars -> write to db
                    // Serial.println("-- " + curChar->value());

                    if (name == ""){
                        name = characteristicsName(curChar->type);
                    }


                    if ( curChar->notifiable() && !curChar->hidden() ){
                        struct screenInfo s;
                        s.aid = i + 1;
                        s.iid = curChar->iid;
                        s.name = name;
                        s.accessoryName = accName;

                        _screenMap[n++] = s;
                    }
                }					

            }
            
        }
    }

    _numberOfScreens = n;
    LogD("OK -> Number of screens: " + String(n), true);
}


HAPConfigValidationResult HAPPluginSSD1306::validateConfig(JsonObject object){
    return HAPPlugin::validateConfig(object);
}

JsonObject HAPPluginSSD1306::getConfigImpl(){
    DynamicJsonDocument doc(1);
	return doc.as<JsonObject>();
}

void HAPPluginSSD1306::setConfigImpl(JsonObject root){

}

void HAPPluginSSD1306::updateProgressbar(uint8_t percentage){
    _tft->drawProgressBar(3, 52, 122, 12, percentage);
}

#endif