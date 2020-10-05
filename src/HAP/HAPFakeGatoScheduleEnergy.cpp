// 
// HAPFakeGatoScheduleEnergy.cpp
// Homekit
//
//  Created on: 23.09.2020
//      Author: michael
//

#include "HAPFakeGatoScheduleEnergy.hpp"
#include <base64.h>
#include "HAPLogger.hpp"
#include "HAPServer.hpp"

HAPFakeGatoScheduleEnergy::HAPFakeGatoScheduleEnergy(){
	_statusLED = 0x00;


	_serialNumber = "";

	_callbackTimerStart = nullptr;
	_callbackTimerEnd   = nullptr;

	_callbackGetRefTime = nullptr;
	_callbackGetTimestampLastActivity = nullptr;
	_callbackGetTimestampLastEntry = nullptr;

	_callbackGetMemoryUsed = nullptr;
	_callbackGetRolledOverIndex = nullptr;
}

HAPFakeGatoScheduleEnergy::HAPFakeGatoScheduleEnergy(String serialNumber, std::function<void(uint16_t)> callbackStartTimer, std::function<void(uint16_t)> callbackEndTimer, std::function<uint32_t(void)> callbackRefTime, std::function<uint32_t(void)> callbackTimestampLastActivity, std::function<uint32_t(void)> callbackTimestampLastEntry){
	_serialNumber = serialNumber;
	_callbackTimerStart = callbackStartTimer;
	_callbackTimerEnd = callbackEndTimer;
	_callbackGetRefTime = callbackRefTime;
	_callbackGetTimestampLastActivity = callbackTimestampLastActivity;
	_callbackGetTimestampLastEntry = callbackTimestampLastEntry;

	// _callbackGetMemoryUsed
	// _callbackGetRolledOverIndex
}

HAPFakeGatoScheduleEnergy::~HAPFakeGatoScheduleEnergy(){

}

void HAPFakeGatoScheduleEnergy::begin(){

}

bool HAPFakeGatoScheduleEnergy::decodeToggleOnOff(uint8_t* data){
	return data[1] & 0x01;
}

void HAPFakeGatoScheduleEnergy::enable(bool on){		
	_timers.enable(on);
}

void HAPFakeGatoScheduleEnergy::setStatusLED(uint8_t mode){	
	_statusLED = mode;
}

bool HAPFakeGatoScheduleEnergy::isEnabled(){
	return _timers.isEnabled();
}

void HAPFakeGatoScheduleEnergy::decodeDays(uint8_t *data){
	uint32_t daysnumber = data[4] | (data[5] << 8) | (data[6] << 16) | (data[7] << 24);
	daysnumber = daysnumber >> 4;	

	_days = HAPFakeGatoScheduleDays(daysnumber);	

#if HAP_DEBUG_FAKEGATO_SCHEDULE
	Serial.printf("M T W T F S S \n");
	Serial.printf("%d %d %d %d %d %d %d \n", _days.mon, _days.tue, _days.wed, _days.thu, _days.fri, _days.sat, _days.sun);
#endif	
}


void HAPFakeGatoScheduleEnergy::decodePrograms(uint8_t* data){
	// clear all old programs and timers
	clear();
	// ToDo: update to daily timer vector
	_timers.clear();	

	uint8_t programCount = data[1];
	programCount = programCount - 1;

	uint8_t pcount = 0;
	uint8_t curIndex = 7;

	for (uint8_t i = 0; i < programCount; i++) {
		
		HAPFakeGatoScheduleProgramEvent program;
		program.id = pcount;

		uint16_t timerCount = (data[curIndex] | (data[curIndex+1] << 8)) >> 7;
		
		// printf("%x %x\n", data[curIndex], data[curIndex+1]);
		// printf("timerCount: %d\n", timerCount);

		curIndex = curIndex + 2;


		for (uint8_t j = 0; j < timerCount; ++j){
			// printf("%x%x", data[curIndex], data[curIndex+1]);

			uint16_t timer = data[curIndex] | (data[curIndex+1] << 8);

			// printf("timer: %d: %X \n", timer, timer);

			HAPFakeGatoScheduleTimerEvent tEvent;

			tEvent.state  = (timer & 0x1F) >> 2;
			tEvent.type   = static_cast<HAPFakeGatoScheduleTimerType>( ((timer & 0x1F) & 0x02 ) >> 1);

			if ((timer & 0x1F) == 1 || (timer & 0x1F) == 5) {
							
				tEvent.minute = (timer >> 5) % 60;		
				tEvent.hour   = ((timer >> 5) - tEvent.minute) / 60;

				// printf("offset:: %d\n", ((timer >> 5) * 60) );
				tEvent.offset = ((timer >> 5) * 60); 			

#if HAP_DEBUG_FAKEGATO_SCHEDULE
				Serial.printf("hour: %d, min: %d, offset: %d state: %d type: %d \n", tEvent.hour, tEvent.minute, tEvent.offset, tEvent.state, tEvent.type);
#endif
			} else if ((timer & 0x1F) == 7 || (timer & 0x1F) == 3) {
				tEvent.sunrise = static_cast<HAPFakeGatoScheduleSunriseType>(((timer >> 5) & 0x01));    // 1 = sunrise, 0 = sunset

				tEvent.offset  = ((timer >> 6) & 0x01 ? ~((timer >> 7) * 60) + 1 : (timer >> 7) * 60);   // offset from sunrise/sunset (plus/minus value)

#if HAP_DEBUG_FAKEGATO_SCHEDULE
				Serial.printf("sunrise: %d, offset: %d state: %d type: %d \n", tEvent.sunrise, tEvent.offset, tEvent.state, tEvent.type);
#endif				
			}

			
			program.timerEvents.push_back(tEvent);
			curIndex = curIndex + 2;
		}

		

		_programEvents.push_back(program);
		pcount++;
	}


	programTimers();
	// for (int i = 0; i < _programEvents.size(); i++){
	// 	printf("   %d:\n", _programEvents[i].id + 1);

	// 	for (int j = 0; j < _programEvents[i].timerEvents.size(); ++j){
	// 		HAPFakeGatoScheduleTimerEvent tEvent = _programEvents[i].timerEvents[j];

	// 		if (tEvent.type == TIME) {
	// 			printf("   >>> hour: %d, min: %d, offset: %d state: %d type: %d \n", tEvent.hour, tEvent.minute, tEvent.offset, tEvent.state, tEvent.type);
	// 		} else {
	// 			printf("   >>> sunrise: %d, offset: %d state: %d type: %d \n", tEvent.sunrise, tEvent.offset, tEvent.state, tEvent.type);
	// 		}
	// 	}
	// }
	
}


uint32_t HAPFakeGatoScheduleEnergy::encodeTimerCount(uint8_t timerCount){
	uint32_t result = 0;
	if (timerCount < 3) {
		result = (timerCount * 128) + timerCount;
	} else {
		result = (timerCount * 128) + 3;
	}
	return result;
}


uint8_t HAPFakeGatoScheduleEnergy::encodeProgramCount(uint8_t programCount){
	return programCount + 1;
}


void HAPFakeGatoScheduleEnergy::encodePrograms(uint8_t* data, size_t *dataSize){
	// uint8_t programCount = data[1] | data[2] << 8;
	uint8_t programCount = _programEvents.size();
	// printf("%0x\n", programCount);

	uint8_t totalTimerCount = 0;
	
	uint16_t timerCount[programCount];

	for (uint8_t i = 0; i < _programEvents.size(); i++){
		timerCount[i] = _programEvents[i].timerEvents.size();
		totalTimerCount += timerCount[i];
	}
	
	// printf("%d\n", totalTimerCount);
	// uint8_t dataSize = 1 + 1 + 5;	
	*dataSize = 1 + 1 + 5;
	*dataSize += (programCount * 2) + (totalTimerCount * 2);
	// printf("%d\n", dataSize);

	// uint8_t data[dataSize];
	if (data == nullptr) {
		return;
	}

	memset(data, 0x00, *dataSize);

	data[ 0 ] 	= 0x05;
	data[ 1 ] 	= (programCount + 1);

	uint8_t curIndex = 7;
	for (int i = 0; i < programCount; i++){

		// printf("timerCount %d\n", timerCount[i]);

		uint8_t addition = 0;
		if (timerCount[i] < 3){
			addition = timerCount[i];
		} else {
			addition = 3;
		}

		data[curIndex++] = ((timerCount[i] << 7) & 0xFF) + addition;		
		data[curIndex++] = ((timerCount[i] << 7 ) >> 8);		

		for (int j = 0; j < _programEvents[i].timerEvents.size(); j++){

			HAPFakeGatoScheduleTimerEvent tEvent = _programEvents[i].timerEvents[j];

			uint16_t timerData = 0;			
			
			tEvent.state == true ? timerData += 4 : timerData += 0;

			// tEvent.type  == TIME ? timerData += 1 : timerData += 3;
			if (tEvent.type  == TIME){
				timerData += (tEvent.offset / 60) << 5;
				timerData += 1;
			} else {
				// printf("offset: %d - %02X \n", tEvent.offset, tEvent.offset);

				if (tEvent.offset < 0) {
					timerData += (~((tEvent.offset / 60) << 7) + 1 + 0x40);
					// printf("negatte: %d\n", timerData);

				} else {
					timerData += (tEvent.offset / 60) << 7;
				}

				timerData += 0x03 + ((uint8_t)tEvent.sunrise * 0x1F) + (uint8_t)tEvent.sunrise;					
			}
		
			data[curIndex++] = (timerData & 0xFF);		
			data[curIndex++] = (timerData >> 8);
		
			// printf("%d - %d %02X %02X %02X\n", tEvent.offset, timerData, timerData, (timerData & 0xFF), (timerData >> 8));
		}
	}
}


void HAPFakeGatoScheduleEnergy::clear(){
	for (int i = 0; i < _programEvents.size(); i++){
		_programEvents[i].timerEvents.clear();
	}

	_programEvents.clear();
}

void HAPFakeGatoScheduleEnergy::fromJson(JsonObject &root){
	
	if (root["timer"].isNull()) return;

	enable(root["timer"]["enabled"].as<bool>());
	_days = HAPFakeGatoScheduleDays(root["timer"]["days"].as<uint32_t>());
	
	String dataStr = root["timer"]["programs"].as<String>();
	
	uint8_t data[dataStr.length() / 2];
	HAPHelper::hexToBin(data, dataStr.c_str(), dataStr.length());
	
	decodePrograms(data);
	
}


JsonObject HAPFakeGatoScheduleEnergy::toJson(){
	/*
		"days": uint32_t,
		"programs": "0503....."
	*/
	const size_t capacity = 256;
	DynamicJsonDocument doc(capacity);

	if (_programEvents.size() == 0) {
	    doc.shrinkToFit();
    	return doc.as<JsonObject>();
	}
	
	doc["enabled"] = isEnabled();
	doc["days"] = _days.daysnumber();

	size_t dataSize = 0;
	encodePrograms(nullptr, &dataSize);
	uint8_t data[dataSize];
	encodePrograms(data, &dataSize);

	// HAPHelper::array_print("CONFIG SAVE data", data, dataSize);

	char hexString[(dataSize * 2) + 1];
	HAPHelper::binToHex(data, dataSize, hexString, (dataSize * 2) + 1);

	doc["programs"] = hexString;
	return doc.as<JsonObject>();
}

String HAPFakeGatoScheduleEnergy::buildScheduleString(){
    TLV8 tlv;
    
    tlv.encode(0x00, {0x24, 0x00});
    tlv.encode(0x03, {0xB8, 0x04});

    // Serial Number
    // tlv.encode(0x04, {0x42, 0x56, 0x31, 0x32, 0x4A, 0x31, 0x41, 0x30, 0x37, 0x32, 0x31, 0x32});
	tlv.encode(HAP_FAKEGATO_SCHEDULE_TYPE_SERIALNUMBER, _serialNumber.length(), (uint8_t*)_serialNumber.c_str());    
	
	// Number of history entries
    // tlv.encode(0x06, {0xFB, 0x0A});
	ui16_to_ui8 memoryUsed;
	if (_callbackGetMemoryUsed != nullptr) {
		memoryUsed.ui16 = _callbackGetMemoryUsed();
	} else {
		memoryUsed.ui16 = 0;
	}
	
	tlv.encode(HAP_FAKEGATO_SCHEDULE_TYPE_USED_MEMORY, 2, memoryUsed.ui8); 
	
	// Number of rolled over index    
    // tlv.encode(0x07, {0x0C, 0x10, 0x00, 0x00});
	ui32_to_ui8 rolledOverIndex;
	if (_callbackGetRolledOverIndex != nullptr) {
		rolledOverIndex.ui32 = _callbackGetRolledOverIndex();
	} else {
		rolledOverIndex.ui32 = 0;
	}
	tlv.encode(HAP_FAKEGATO_SCHEDULE_TYPE_ROLLED_OVER_INDEX, 4, rolledOverIndex.ui8); 

    tlv.encode(0x0B, {0x00, 0x00});
    tlv.encode(0x05, {0x00});
    tlv.encode(0x02, {0x90, 0x27, 0x00, 0x00});
    tlv.encode(0x5F, {0x00, 0x00, 0x00, 0x00});
    tlv.encode(0x19, {0x96, 0x00});
    tlv.encode(0x14, {0x03});
    tlv.encode(0x0F, {0x00, 0x00, 0x00, 0x00});

    // Programs
    //tlv.encode(0x45, {0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0x01, 0x3C, 0x05, 0x96});
	size_t dataSize = 0;
	encodePrograms(nullptr, &dataSize);
	uint8_t data[dataSize];
	encodePrograms(data, &dataSize);
	tlv.encode(HAP_FAKEGATO_SCHEDULE_TYPE_PROGRAMS, dataSize, data);

    // Days
    // tlv.encode(0x46, {0x05, 0x15, 0x1C, 0x2C, 0x9F, 0x24, 0x49, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});	
	uint8_t daysHex[84];
	memset(daysHex, 0, 84);

	uint32_t daysNo = _days.daysnumber();
	daysHex[0] = 0x05;  // ?? 
	daysHex[1] = 0x15;  // ??
	daysHex[2] = 0x1C;	// ??
	daysHex[3] = 0x2C;	// ??
	daysHex[4] = daysNo;
	daysHex[5] = daysNo >>  8;
	daysHex[6] = daysNo >> 16;
	daysHex[7] = daysNo >> 24;

	tlv.encode(HAP_FAKEGATO_SCHEDULE_TYPE_DAYS, 84, daysHex);

    // Commands			 						
    // tlv.encode(0x44, {0x05, 0x0C, 0x00, 0x05, 0x03, 0x3C, 0x00, 0x00, 0x00, 0x32, 0xC2, 0x42, 0x42, 0xA1, 0x93, 0x34, 0x41});
	// 						    |	 
	//						    +-> 0x0C = OFF -- 0xOD = ON 
	if (_timers.isEnabled()) {
		tlv.encode(HAP_FAKEGATO_SCHEDULE_TYPE_COMMAND_TOGGLE_SCHEDULE, {0x05, 0x0D, 0x00, 0x05, 0x03, 0x3C, 0x00, 0x00, 0x00, 0x32, 0xC2, 0x42, 0x42, 0xA1, 0x93, 0x34, 0x41});
	} else {
		tlv.encode(HAP_FAKEGATO_SCHEDULE_TYPE_COMMAND_TOGGLE_SCHEDULE, {0x05, 0x0C, 0x00, 0x05, 0x03, 0x3C, 0x00, 0x00, 0x00, 0x32, 0xC2, 0x42, 0x42, 0xA1, 0x93, 0x34, 0x41});
	}

    // ??
    tlv.encode(0x47, {0x05, 0x73, 0x1B, 0x45, 0x1C, 0xDF, 0x1C, 0xB8, 0x1D, 0xB4, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x00});	
    tlv.encode(0x48, {0x05, 0x00, 0x00, 0x00, 0x00, 0x00});
    tlv.encode(0x4A, {0x05, 0x00, 0x00, 0x00, 0x00, 0x00});
    tlv.encode(0x1A, {0x00, 0x00, 0x00, 0x00});
    
    // Status LED
    // tlv.encode(0x60, {0x64});
	tlv.encode(HAP_FAKEGATO_SCHEDULE_TYPE_STATUS_LED, 1, _statusLED);

    // last activity On switch ?
    // tlv.encode(0xD0, {0x99, 0x6C, 0x21, 0x00});
	ui32_to_ui8 secsLastAct;
	if ((_callbackGetTimestampLastActivity != nullptr) && (_callbackGetRefTime != nullptr)) {
		secsLastAct.ui32 = _callbackGetTimestampLastActivity() - _callbackGetRefTime();
	} else {
		secsLastAct.ui32 = 0;
	}    
	tlv.encode(0xD0, 4, secsLastAct.ui8); // offset ?

#if HAP_DEBUG_FAKEGATO_SCHEDULE	
	// HAPHelper::array_print("secsLastAct", secsLastAct.ui8, 4);
#endif

    //  EVE Time
	//tlv.encode(0x9B, {0xFB, 0x2C, 0x19, 0x00}); // offset ?
	ui32_to_ui8 secs;
	if ((_callbackGetTimestampLastEntry != nullptr) && (_callbackGetRefTime != nullptr)) {
		secs.ui32 = _callbackGetTimestampLastEntry() - _callbackGetRefTime();
	} else {
		secs.ui32 = 0;
	}    
	tlv.encode(0x9B, 4, secs.ui8); // offset ?

#if HAP_DEBUG_FAKEGATO_SCHEDULE	
	// HAPHelper::array_print("secs", secs.ui8, 4);
#endif
    
    // ending bytes?
    // tlv.encode(0xD2, {});
    uint8_t endBytes[2] = {0xD2, 0x00};

    size_t decodedLen = 0;	
	uint8_t out[tlv.size() + 2];

	tlv.decode(out, &decodedLen);
    
    // attach endingBytes
    memcpy(out + decodedLen, endBytes, 2);
    decodedLen = decodedLen + 2;

#if HAP_DEBUG_FAKEGATO_SCHEDULE	
    HAPHelper::array_print("tlv", out, decodedLen);
#endif

	return base64::encode(out, decodedLen);
}

void HAPFakeGatoScheduleEnergy::programTimers() {
	
	for (size_t i = 0; i < _programEvents.size(); i++){
		
#if HAP_DEBUG_FAKEGATO_SCHEDULE		
		Serial.printf("M T W T F S S \n");
		Serial.printf("%d %d %d %d %d %d %d \n", _days.mon, _days.tue, _days.wed, _days.thu, _days.fri, _days.sat, _days.sun);
#endif		
		
		/*SMTWTFSS*/
		uint8_t daysMask = 0;
		daysMask  = (_days.sun == (i+1)) << 7;
		daysMask |= (_days.mon == (i+1)) << 6;
		daysMask |= (_days.tue == (i+1)) << 5;
		daysMask |= (_days.wed == (i+1)) << 4;
		daysMask |= (_days.thu == (i+1)) << 3;
		daysMask |= (_days.fri == (i+1)) << 2;
		daysMask |= (_days.sat == (i+1)) << 1;
		//0b10000000,

#if HAP_DEBUG_FAKEGATO_SCHEDULE		
		Serial.printf("daysMask: %d\n", daysMask);
#endif	

		for (size_t j = 0; j < _programEvents[i].timerEvents.size(); j++){
			// add dailytimer here !!			
			HAPDailyTimer timer(_programEvents[i].timerEvents[j].hour,
								_programEvents[i].timerEvents[j].minute,
								daysMask,
								FIXED,
								std::bind(&HAPFakeGatoScheduleEnergy::callbackTimerStart, this, std::placeholders::_1),
								// [](uint16_t i){Serial.println("Serial Print Timer just Fired!");},
								(uint16_t)_programEvents[i].timerEvents[j].state);

			// timer.setDaysActive(daysMask);
			timer.begin();					
			_timers.addTimer(timer);
		}
	} 
}

void HAPFakeGatoScheduleEnergy::callbackTimerStart(uint16_t state){
#if HAP_DEBUG_FAKEGATO_SCHEDULE
	LogI(HAPServer::timeString() + " " + "HAPFakeGatoScheduleEnergy" + "->" + String(__FUNCTION__) + " [   ] " + "Timed action: START", true);
#endif	
	if (_callbackTimerStart) _callbackTimerStart(state);
}

void HAPFakeGatoScheduleEnergy::callbackTimerEnd(uint16_t state){
#if HAP_DEBUG_FAKEGATO_SCHEDULE	
	LogI(HAPServer::timeString() + " " + "HAPFakeGatoScheduleEnergy" + "->" + String(__FUNCTION__) + " [   ] " + "Timed action: END", true);
#endif	
	if (_callbackTimerEnd)  _callbackTimerEnd(state);
}