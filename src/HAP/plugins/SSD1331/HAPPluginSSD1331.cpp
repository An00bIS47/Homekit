//
// HAPPluginSSD1331.cpp
// Homekit
//
//  Created on: 29.04.2018
//      Author: michael
//


#include "HAPPluginSSD1331.hpp"
#include "HAPServer.hpp"

#if HAP_PLUGIN_USE_SSD1331


#include "qrcode.h"
#include "_fonts/homekit36.c"

#define QR_CODE_VERSION         3   // see the sizes of the qr code version here: https://github.com/ricmoo/QRCode
#define QR_CODE_ZOOM_FACTOR     2
#define QR_CODE_X_OFFSET        19  // (96   -   (29        * 2) ) / 2 = 19
                                    // width - (qrcode size * 2)   / 2 = offset

#define QR_CODE_Y_OFFSET        3   // (64   -   (29        * 2) ) / 2 = 3


#define VERSION_MAJOR       0
#define VERSION_MINOR       0
#define VERSION_REVISION    3
#define VERSION_BUILD       2



HAPPluginSSD1331::HAPPluginSSD1331(){

    _type = HAP_PLUGIN_TYPE_DISPLAY;
	_name = "SSD1331";
	_isEnabled = HAP_PLUGIN_USE_SSD1331;
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

HAPPluginSSD1331::~HAPPluginSSD1331(){
    delete _tft;
}

bool HAPPluginSSD1331::begin(){

    _tft = new SSD_13XX(__CS, __DC, __RST);
    _tft->begin();
    return true;
}

HAPAccessory* HAPPluginSSD1331::initAccessory(){

	return nullptr;
}

void HAPPluginSSD1331::handleEvents(int eventCode, struct HAPEvent eventParam){

    if (_isEnabled) {
        LogD("Handle event: [" + String(__PRETTY_FUNCTION__) + "]", true);	

        if (eventCode == EventManager::kEventPairingComplete){        
            _tft->clearScreen();
            _tft->setTextScale(0);
            _tft->setCursor(CENTER, CENTER);
            _tft->print("Pairing complete!");
            _updateDisplay = true;
        }

        if (eventCode == EventManager::kEventHomekitStarted){        
            if (!_accessorySet->isPaired()){
                _tft->clearScreen();
                displayQRCode();   
                _updateDisplay = false;                             
            }
            setupScreens();
        }
    }

}


void HAPPluginSSD1331::handleImpl(bool forced){
    
    LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Handle plguin [" + String(_interval) + "]", true);
    
    if (_updateDisplay) {            
        _tft->clearScreen();
    }

    if ( (_accessorySet->isPaired()) && (_numberOfScreens > 0) ) {
        struct screenInfo s = _screenMap[_currentScreen];

        // Serial.println("_numberOfScreens: " + String(_numberOfScreens));
        // Serial.println("acc Name: " + s.accessoryName);
        // Serial.println("name: " + s.name);
        // Serial.println("aid: " + String(s.aid));
        // Serial.println("iid: " + String(s.iid));
                
        characteristics* curChar = _accessorySet->getCharacteristics(s.aid, s.iid);

        _tft->setFont(&Homekit36);              //this will load the font
        _tft->setTextScale(1);
        _tft->setCharSpacing(1);                //add an extra pixel between chars
        _tft->setCursor(CENTER, CENTER);        // center text
        _tft->println(curChar->value() );
        
        
        _tft->setInternalFont();                //now switch to internal font
        _tft->setTextScale(1);

        _updateDisplay = true;
        _currentScreen++;

        if (_currentScreen >= _numberOfScreens){
            _currentScreen = 0;
        }

    } else if (_numberOfScreens == 0) {
        setupScreens();
    }
    
}

void HAPPluginSSD1331::displayQRCode(){

    LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Display QR code", true);
    //
    // Show qr code for pairing
    //
    QRCode _qrcode;        
    uint8_t qrcodeData[qrcode_getBufferSize(QR_CODE_VERSION)];
    qrcode_initText(&_qrcode, qrcodeData, QR_CODE_VERSION, ECC_MEDIUM, _accessorySet->xhm());        
        
    int k = 0;         
    for (uint8_t y = 0; y < _qrcode.size; y++) {
        int n = 0;
        for (uint8_t x = 0; x < _qrcode.size; x++) {
            if (qrcode_getModule(&_qrcode, x, y) ) {                    
                _tft->drawRect(x + QR_CODE_X_OFFSET + (n * QR_CODE_ZOOM_FACTOR) - n, y + QR_CODE_Y_OFFSET + (k * QR_CODE_ZOOM_FACTOR) - k, QR_CODE_ZOOM_FACTOR, QR_CODE_ZOOM_FACTOR, WHITE, WHITE, true );
            } else {
                _tft->drawRect(x + QR_CODE_X_OFFSET + (n * QR_CODE_ZOOM_FACTOR) - n, y + QR_CODE_Y_OFFSET + (k * QR_CODE_ZOOM_FACTOR) - k, QR_CODE_ZOOM_FACTOR, QR_CODE_ZOOM_FACTOR, BLACK, BLACK, true );
            }
            n++;
        }            
        k++;            
    }
}

void HAPPluginSSD1331::addEventListener(EventManager* eventManager){

    if (_isEnabled) {
        LogD(HAPServer::timeString() + " " + _name + "->" + String(__FUNCTION__) + " [   ] " + "Add listerner to event manager", true);

        _listenerMemberFunctionPlugin.mObj = this;
        _listenerMemberFunctionPlugin.mf = &HAPPlugin::handleEvents;
        
        // Add listener to event manager
        _eventManager = eventManager;
        // for accessory notifications and values
        _eventManager->addListener( EventManager::kEventNotifyAccessory, &_listenerMemberFunctionPlugin );
        _eventManager->addListener( EventManager::kEventPairingComplete, &_listenerMemberFunctionPlugin );
        _eventManager->addListener( EventManager::kEventHomekitStarted, &_listenerMemberFunctionPlugin );
        // _eventManager->addListener( EventManager::kEventNotifyController, &_listenerMemberFunctionPlugin );		
    }
    
}

void HAPPluginSSD1331::setupScreens(){
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


HAPConfigValidationResult HAPPluginSSD1331::validateConfig(JsonObject object){
    return HAPPlugin::validateConfig(object);
}

JsonObject HAPPluginSSD1331::getConfigImpl(){
    DynamicJsonDocument doc(1);
	return doc.as<JsonObject>();
}

void HAPPluginSSD1331::setConfigImpl(JsonObject root){

}

#endif