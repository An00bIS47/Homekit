//
// HAPDailyTimerFactory.hpp
// Homekit
//
//  Created on: 28.09.2020
//      Author: michael
//
#ifndef HAPDAILYTIMERFACTORY_HPP
#define HAPDAILYTIMERFACTORY_HPP

#include <Arduino.h>
#include <functional>
#include <vector>

#include "HAPDailyTimer.hpp"
  
class HAPDailyTimerFactory{
public:
    HAPDailyTimerFactory();

    void handle();
    bool isActive(uint8_t index);
    
    void enable(bool on);
    bool isEnabled();

    void addTimer(HAPDailyTimer dailyTimer);
    
    inline void clear(){
        _timers.clear();
    }

    inline size_t size(){
        return _timers.size();
    }

protected:
    bool _isEnabled;
    std::vector<HAPDailyTimer> _timers;
};


#endif /* HAPDAILYTIMER_HPP */
