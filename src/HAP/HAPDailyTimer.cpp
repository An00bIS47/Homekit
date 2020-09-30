//
// HAPDailyTimer.cpp
// Homekit
//
//  Created on: 28.09.2020
//      Author: michael
//

#include "HAPDailyTimer.hpp"

// uint8_t HAPDailyTimer::instanceCount = 0;
// HAPDailyTimer* instances[MAX_TIMER_INSTANCES] = {nullptr};


HAPDailyTimer::HAPDailyTimer(uint8_t StartHour, uint8_t StartMinute, uint8_t daysMask, RandomType type, HAPDailyTimerCallback startCallback, uint16_t targetStateStart_){
	autoSync = false;
	startTime.hour = StartHour > 23 ? 23 : StartHour; 
	startTime.minute = StartMinute > 59 ? 59 : StartMinute; 
	endTime.hour = startTime.hour;
	endTime.minute = startTime.minute;
	setDaysActive(daysMask);
	randomType = type;
	offset = 15;
	startTimeCallback = startCallback;
	endTimeCallback = NULL;

	// instances[instanceCount++] = this;

	targetStateStart = targetStateStart_;
	targetStateEnd = NULL;
}


HAPDailyTimer::HAPDailyTimer(uint8_t StartHour, uint8_t StartMinute, EventDays DaysOfTheWeek, RandomType type, HAPDailyTimerCallback startCallback, uint16_t targetStateStart_) {
	autoSync = false;
	startTime.hour = StartHour > 23 ? 23 : StartHour; 
	startTime.minute = StartMinute > 59 ? 59 : StartMinute; 
	endTime.hour = startTime.hour;
	endTime.minute = startTime.minute;
	setDaysActive(dayTemplate[static_cast<int>(DaysOfTheWeek)]);
	randomType = type;
	offset = 15;
	startTimeCallback = startCallback;
	endTimeCallback = NULL;

	// instances[instanceCount++] = this;

	targetStateStart = targetStateStart_;
	targetStateEnd = NULL;
}

HAPDailyTimer::HAPDailyTimer(bool syncOnPowerup, uint8_t StartHour, uint8_t StartMinute, uint8_t EndHour, uint8_t EndMinute, EventDays DaysOfTheWeek, RandomType type, HAPDailyTimerCallback startCallback, uint16_t targetStateStart_, HAPDailyTimerCallback endCallback, uint16_t targetStateEnd_) {
	autoSync = syncOnPowerup;
	startTime.hour = StartHour > 23 ? 23 : StartHour; 
	startTime.minute = StartMinute > 59 ? 59 : StartMinute; 
	endTime.hour = EndHour > 23 ? 23 : EndHour;
	endTime.minute = EndMinute > 59 ? 59 : EndMinute;
	setDaysActive(dayTemplate[static_cast<int>(DaysOfTheWeek)]);
	randomType = type;
	offset = 15;
	startTimeCallback = startCallback;
	endTimeCallback = endCallback;

	// instances[instanceCount++] = this;

	targetStateStart = targetStateStart_;
	targetStateEnd = targetStateEnd_;
}

HAPDailyTimer::HAPDailyTimer(bool syncOnPowerup, uint8_t StartHour, uint8_t StartMinute, uint8_t EndHour, uint8_t EndMinute, uint8_t daysMask, RandomType type, HAPDailyTimerCallback startCallback, uint16_t targetStateStart_, HAPDailyTimerCallback endCallback, uint16_t targetStateEnd_){
	autoSync = syncOnPowerup;
	startTime.hour = StartHour > 23 ? 23 : StartHour; 
	startTime.minute = StartMinute > 59 ? 59 : StartMinute; 
	endTime.hour = EndHour > 23 ? 23 : EndHour;
	endTime.minute = EndMinute > 59 ? 59 : EndMinute;
	setDaysActive(daysMask);
	randomType = type;
	offset = 15;
	startTimeCallback = startCallback;
	endTimeCallback = endCallback;

	// instances[instanceCount++] = this;

	targetStateStart = targetStateStart_;
	targetStateEnd = targetStateEnd_;
}

// int HAPDailyTimer::getInstanceCount(void) const {
//   	return instanceCount;
// }

bool HAPDailyTimer::begin() {
  	return sync();
}

void HAPDailyTimer::setDaysActive(EventDays days) {
	onMask = dayTemplate[static_cast<int>(days)];
	time_t now_time = now();
	if(tmConvert_t(year(now_time), month(now_time), day(now_time), startTime.hour, startTime.minute, 0) > tmConvert_t(year(now_time), month(now_time), day(now_time), endTime.hour, endTime.minute, 0)){
		offMask = onMask >> 1;
	} else {
		offMask = onMask;
	}
	(void)sync();
}

void HAPDailyTimer::setDaysActive(uint8_t activeDays) {
	onMask = activeDays;
	time_t now_time = now();
	if(tmConvert_t(year(now_time), month(now_time), day(now_time), startTime.hour, startTime.minute, 0) > tmConvert_t(year(now_time), month(now_time), day(now_time), endTime.hour, endTime.minute, 0))
	{
		offMask = onMask >> 1;
	} else {
		offMask = onMask;
	}
	(void)sync();
}

void HAPDailyTimer::setRandomOffset(uint8_t random_minutes, RandomType type) {
	offset = random_minutes > 59 ? 59 : random_minutes;
	if (offset == 0) {
		randomType = FIXED;
	} else {
		randomType = type;
	}
}

void HAPDailyTimer::setStartTime(uint8_t hour, uint8_t minute){
	startTime.hour = hour > 23 ? 23 : hour; 
	startTime.minute = minute > 59 ? 59 : minute; 
	setDaysActive(onMask);
	(void)sync();
}

void HAPDailyTimer::setEndTime(uint8_t hour, uint8_t minute){
	endTime.hour = hour > 23 ? 23 : hour;
	endTime.minute = minute > 59 ? 59 : minute;
	setDaysActive(onMask);
	(void)sync();
}


uint8_t HAPDailyTimer::setRandomDays(uint8_t number_Days){
	randomSeed(now() + micros());
	uint8_t mask = 0;
	uint8_t array[8] = {0};
	for (int i = 0; i < number_Days; i++) {
		array[i] = 1;
	}
	for(int i = 0; i < 7; i++) {
		uint8_t index = random(i, 7);
		uint8_t temp = array[i];
		array[i] = array[index];
		array[index] = temp;
	}
	
	for (int i = 0; i < 7; i++) {
		mask |= (array[i] << i);
	}
	onMask = mask << 1;
	(void)sync();

	return onMask;
}

uint8_t HAPDailyTimer::getDays() const {
  	return onMask;
}

bool HAPDailyTimer::sync(){
	bool currentState = isActive(this);
	if(currentState && autoSync){
		startTimeCallback(targetStateStart);
	}
	return state = currentState;
}

// void HAPDailyTimer::update()
// {
// 	for(int i = 0; i < instanceCount; i++) {
// 		bool lastState = instances[i]->state;
// 		instances[i]->state = isActive(instances[i]);
// 		if(lastState != instances[i]->state) {
// 			if(instances[i]->state == true) {
// 				if(instances[i]->startTimeCallback) instances[i]->startTimeCallback(instances[i]->targetStateStart);
// 			} else {
// 				if(instances[i]->endTimeCallback) instances[i]->endTimeCallback(instances[i]->targetStateEnd);
// 			}
// 		}
// 	}
// }

bool HAPDailyTimer::isActive(){
  	return isActive(this);
}

bool HAPDailyTimer::isActive(HAPDailyTimer* instance) {
  if (instance->currentDay != weekday() && instance->randomType) // once a day, generate new random offsets
  {
    randomSeed(now() + micros());
    if (instance->randomType == RANDOM  || instance->randomType == RANDOM_START)
    {
      int hrs = instance->startTime.hour * 60 + instance->startTime.minute;
      hrs += constrain(random(-1 * instance->offset, instance->offset), 1, (24 * 60) - 1);
      instance->randomStartTime.minute = hrs % 60;
      instance->randomStartTime.hour = hrs / 60;
    }
    if (instance->randomType == RANDOM || instance->randomType == RANDOM_END)
    {
      int hrs = instance->endTime.hour * 60 + instance->endTime.minute;
      hrs += constrain(random(-1 * instance->offset, instance->offset), 1, (24 * 60) - 1);
      instance->randomEndTime.minute = hrs % 60;
      instance->randomEndTime.hour = hrs / 60;
    }
    instance->currentDay = weekday();
  }
  time_t now_time = now();
  time_t on_time = tmConvert_t(year(now_time), month(now_time), day(now_time), (instance->randomType == RANDOM || instance->randomType == RANDOM_START) ? instance->randomStartTime.hour : instance->startTime.hour, (instance->randomType == RANDOM || instance->randomType == RANDOM_START) ? instance->randomStartTime.minute : instance->startTime.minute, /*second(now_time)*/ 0);
  time_t off_time = tmConvert_t(year(now_time), month(now_time), day(now_time), (instance->randomType == RANDOM || instance->randomType == RANDOM_END) ? instance->randomEndTime.hour : instance->endTime.hour, (instance->randomType == RANDOM || instance->randomType == RANDOM_END) ? instance->randomEndTime.minute : instance->endTime.minute, /*second(now_time));*/ 0);
  uint8_t weekDay = weekday(now_time);
  uint8_t today = 0b00000001 << (8 - weekDay);
  if (today & dayTemplate[SUNDAYS])
  {
    today |= 0b00000001;
  }
  if ((today & instance->onMask) && (today & instance->offMask))  // if it is supposed to turn both on and off today
  {
    if (on_time < off_time)
    {
      return (now_time > on_time && now_time < off_time);
    }
    else if (off_time < on_time)
    {
      return (now_time > on_time || now_time < off_time);
    }
//    else 
//    {
//      return false;
//    }
    else if(on_time == off_time) // single edge event
    {
      if( now_time == on_time)
      {
        return true;
      }
    }
    return false;
  }
  else if (today & instance->onMask) // if it is supposed to turn only on today
  {
    if (on_time < off_time)
    {
      return (now_time > on_time && now_time < off_time);
    }
    else
    {
      return (now_time > on_time);
    }
  }
  else if (today & instance->offMask)  // if it is supposed to turn only off today
  {
    return now_time < off_time;
  }
  else // if 
  {
    return false;
  }
}

time_t HAPDailyTimer::tmConvert_t(int YYYY, uint8_t MM, uint8_t DD, uint8_t hh, uint8_t mm, uint8_t ss) {

#if HAS_TIMELIB
	tmElements_t tmSet;
	tmSet.Year = YYYY - 1970;
	tmSet.Month = MM;
	tmSet.Day = DD;
	tmSet.Hour = hh;
	tmSet.Minute = mm;
	tmSet.Second = ss;
	return makeTime(tmSet);

#else

	time_t rawtime;
  	struct tm * timeinfo;	
	time ( &rawtime );
  	timeinfo = localtime( &rawtime );
  	timeinfo->tm_year = YYYY - 1900;
  	timeinfo->tm_mon = MM - 1;
  	timeinfo->tm_mday = DD;
	timeinfo->tm_hour = hh;
	timeinfo->tm_min = mm;
	timeinfo->tm_sec = ss;
	return mktime(timeinfo);
#endif	/* HAS_TIMELIB */
}


#ifndef HAS_TIMELIB
time_t HAPDailyTimer::now(){
	time_t t;
	return time(&t);
}

uint8_t HAPDailyTimer::weekday(time_t rawtime){	
  	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	return timeinfo->tm_wday + 1;	// = days since sunday -> timelib sunday = 1
}

uint8_t HAPDailyTimer::month(time_t rawtime){	
  	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	return timeinfo->tm_mon + 1;
}

uint8_t HAPDailyTimer::day(time_t rawtime){	
  	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	return timeinfo->tm_mday;
}

uint16_t HAPDailyTimer::year(time_t rawtime){	
  	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	return timeinfo->tm_year + 1900;
}

uint8_t HAPDailyTimer::hour(time_t rawtime) {
  	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	return timeinfo->tm_hour;
}

uint8_t HAPDailyTimer::minute(time_t rawtime){
  	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	return timeinfo->tm_min;
}

uint8_t HAPDailyTimer::seconds(time_t rawtime){
  	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	return timeinfo->tm_sec;
}
#endif /* HAS_TIMELIB */