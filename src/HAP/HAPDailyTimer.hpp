//
// HAPDailyTimer.hpp
// Homekit
//
//  Created on: 28.09.2020
//      Author: michael
//
#ifndef HAPDAILYTIMER_HPP
#define HAPDAILYTIMER_HPP

#include <Arduino.h>


//#define HAS_TIMELIB 1   // uncomment if you are using timelib

#ifdef HAS_TIMELIB
#include <TimeLib.h> // see here for more info to replace this: https://esp32.com/viewtopic.php?t=5188
#else
#include <time.h>
#endif

#include <functional>
#include <vector>

typedef std::function<void(uint16_t)> HAPDailyTimerCallback;

// #define MAX_TIMER_INSTANCES 10

enum EventDays{
	SUNDAYS = 0, 
	MONDAYS, 
	TUESDAYS, 
	WEDNESDAYS, 
	THURSDAYS, 
	FRIDAYS, 
	SATURDAYS, 
	WEEKENDS, 
	WEEKDAYS, 
	EVERY_DAY
};

enum RandomType{
	FIXED, 
	RANDOM, 
	RANDOM_START, 
	RANDOM_END
};

const uint8_t dayTemplate[] = {
	/*SMTWTFSS*/   // the bitmask is set with an extra bit for determining off times for days of the week where off time is earlier than on time (i.e. the two values stradle midnight)
	0b10000000,
	0b01000000,
	0b00100000,
	0b00010000,
	0b00001000,
	0b00000100,
	0b00000010,
	0b10000010, // Weekends
	0b01111100, // Weekdays
	0b11111110  // Everyday
};
  
class HAPDailyTimer{
  public:
  
    HAPDailyTimer(uint8_t StartHour, uint8_t StartMinute, uint8_t daysMask, RandomType type, HAPDailyTimerCallback startCallback, uint16_t targetStateStart_);
    HAPDailyTimer(uint8_t StartHour, uint8_t StartMinute, EventDays DaysOfTheWeek, RandomType type, HAPDailyTimerCallback startCallback, uint16_t targetStateStart_);
    
    HAPDailyTimer(bool syncOnPowerup, uint8_t StartHour, uint8_t StartMinute, uint8_t EndHour, uint8_t EndMinute, uint8_t daysMask, RandomType type, HAPDailyTimerCallback startCallback, uint16_t targetStateStart_, HAPDailyTimerCallback endCallback, uint16_t targetStateEnd_);
    HAPDailyTimer(bool syncOnPowerup, uint8_t StartHour, uint8_t StartMinute, uint8_t EndHour, uint8_t EndMinute, EventDays DaysOfTheWeek, RandomType type, HAPDailyTimerCallback startCallback, uint16_t targetStateStart_, HAPDailyTimerCallback endCallback, uint16_t targetStateEnd_);

    void setDaysActive(EventDays days);
    void setDaysActive(uint8_t activeDays);
    uint8_t setRandomDays(uint8_t number_Days);
    void setRandomOffset(uint8_t random_minutes, RandomType randomSetting);
    void setStartTime(uint8_t hour, uint8_t minute);
    void setEndTime(uint8_t hour, uint8_t minute);
    
    bool begin(); 
    uint8_t getDays() const;
    static time_t tmConvert_t(int YYYY, uint8_t MM, uint8_t DD, uint8_t hh, uint8_t mm, uint8_t ss);    
    bool isActive();

    // int getInstanceCount(void) const;
    // static void update();    




#ifndef HAS_TIMELIB
    static time_t now();
    static uint8_t weekday(time_t rawtime = 0);
    static uint8_t day(time_t rawtime= 0);
    static uint8_t month(time_t rawtime= 0);
    static uint16_t year(time_t rawtime= 0);
    static uint8_t hour(time_t rawtime= 0);
    static uint8_t minute(time_t rawtime= 0);
    static uint8_t seconds(time_t rawtime= 0);
#endif

    bool state;                 	// retains last true/false state of timer

    HAPDailyTimerCallback startTimeCallback;
    HAPDailyTimerCallback endTimeCallback;

    uint16_t targetStateStart;
    uint16_t targetStateEnd;

    static bool isActive(HAPDailyTimer* instance);

protected:
    struct TimerTime{ 				// bounded 00:00 to 23:59
      	uint8_t hour;
      	uint8_t minute;
    };

    bool sync();
    
	// void(*startTimeCallback)(); 	//
    // void(*endTimeCallback)();   	//

    

    uint8_t onMask;                	// compact ON days storage
    uint8_t offMask;               	// compact OFF days storage
    
    bool autoSync;              	// will run startTimeCallback if timer is in active state when times are changed or powerup
    TimerTime startTime;        	//
    TimerTime endTime;          	// 
    TimerTime randomStartTime;  	// calculated once daily
    TimerTime randomEndTime;    	// calculated once daily
    RandomType randomType;          // 
    uint8_t currentDay;          	// for comparison of a daily event to randomize the Start and end times
    uint8_t offset;             	// minutes of fuzziness for random Starts and Ends
    
    	

	
    // static HAPDailyTimer* instanceAddress;
    // static uint8_t instanceCount;
};


#endif /* HAPDAILYTIMER_HPP */
