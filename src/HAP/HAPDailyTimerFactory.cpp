//
// HAPDailyTimerFactory.cpp
// Homekit
//
//  Created on: 28.09.2020
//      Author: michael
//


#include "HAPDailyTimerFactory.hpp"
  

HAPDailyTimerFactory::HAPDailyTimerFactory(){

}

void HAPDailyTimerFactory::handle() {
	for(int i = 0; i < _timers.size(); i++) {
		bool lastState = _timers[i].state;
		_timers[i].state = HAPDailyTimer::isActive(&_timers[i]);
		if(lastState != _timers[i].state) {
			if(_timers[i].state == true) {
				if(_timers[i].startTimeCallback) _timers[i].startTimeCallback(_timers[i].targetStateStart);
			} else {
				if(_timers[i].endTimeCallback) _timers[i].endTimeCallback(_timers[i].targetStateEnd);
			}
		}
	}
}

bool HAPDailyTimerFactory::isActive(uint8_t index){
    return HAPDailyTimer::isActive(&_timers[index]);
}

void HAPDailyTimerFactory::addTimer(HAPDailyTimer dailyTimer){
    _timers.push_back(dailyTimer);
}
    



