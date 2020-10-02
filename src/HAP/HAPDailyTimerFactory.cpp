//
// HAPDailyTimerFactory.cpp
// Homekit
//
//  Created on: 28.09.2020
//      Author: michael
//


#include "HAPDailyTimerFactory.hpp"
  

HAPDailyTimerFactory::HAPDailyTimerFactory(){
	_isEnabled = false;
}

void HAPDailyTimerFactory::handle() {

	if (_isEnabled) {
		for (int i = 0; i < _timers.size(); i++) {
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
}

bool HAPDailyTimerFactory::isActive(uint8_t index){
    return HAPDailyTimer::isActive(&_timers[index]);
}

void HAPDailyTimerFactory::enable(bool on){
    _isEnabled = on;
}

bool HAPDailyTimerFactory::isEnabled(){
	return _isEnabled;
}

void HAPDailyTimerFactory::addTimer(HAPDailyTimer dailyTimer){
	_timers.push_back(dailyTimer);
}
    



