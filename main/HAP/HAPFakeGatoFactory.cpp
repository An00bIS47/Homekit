// 
// HAPFakeGatoFactory.cpp
// Homekit
//
//  Created on: 29.08.2019
//      Author: michael
//

#include "HAPFakeGatoFactory.hpp"

#include "HAPServer.hpp"
#include "HAPLogger.hpp"


void HAPFakeGatoFactory::handle(bool forced){
    for (auto & gato : _fakegatos) {

		if (gato->isEnabled()) {		
        	gato->handle();
		}			
        
	} 
}

void HAPFakeGatoFactory::registerFakeGato(HAPFakeGato* fakegato, String name, std::function<bool()> callback, uint32_t interval){

    fakegato->begin();
    fakegato->setName(name);
    fakegato->setRefTime(_refTime);    
    fakegato->setInterval(interval);
    fakegato->registerCallback(callback);
    _fakegatos.push_back(fakegato);
    LogD(HAPServer::timeString() + " " + "HAPFakeGatoFactory" + "->" + String(__FUNCTION__) + " [   ] " + "Registered fakegato for: " + name + " (" + String(_fakegatos.size()) + ")", true);      
}

 void HAPFakeGatoFactory::setRefTime(uint32_t refTime){
    //  LogD(HAPServer::timeString() + " " + __CLASS_NAME__ + "->" + String(__FUNCTION__) + " [   ] " + "Setting refTime: " + String(refTime), true);      
     _refTime = refTime;
 }