//
// HAPLogger.cpp
// Homekit
//
//  Created on: 08.08.2017
//      Author: michael
//

#include "HAPLogger.hpp"

#if !defined(__APPLE__)
#include <esp_log.h>
#endif

LogLevel HAPLogger::_logLevel(LogLevel::INFO);
Stream* HAPLogger::_printer(&Serial);

HAPLogger::HAPLogger() {

}

HAPLogger::~HAPLogger() {
	// TODO Auto-generated destructor stub
}

void HAPLogger::logFreeHeap(int clients, int queue, const char* color){
	if (HAPLogger::_logLevel >= LogLevel::DEBUG) {
		_printer->print(color);
#if HAP_NTP_ENABLED		
		_printer->print(HAPServer::timeString() + " ");
#else
		
#if !defined(__APPLE__)					
		_printer->printf("%lu ", millis());		
#else
		printf("%lu ", millis());
#endif

#endif

#if !defined(__APPLE__)		
		_printer->print( "HAPServer->heap [   ] current: ") ;
		_printer->print(ESP.getFreeHeap());
		_printer->print(  " - minimum: ") ;
		_printer->print(xPortGetMinimumEverFreeHeapSize());
#endif		
		_printer->print(  " [clients:") ;
		_printer->print(clients);
		_printer->print(  "]") ;
		_printer->print(  " [queue:") ;
		_printer->print(queue);
		_printer->print(  "]") ;
		_printer->println(COLOR_RESET);
	}
}

void HAPLogger::logInfo(String str, bool newLine){
	if (HAPLogger::_logLevel >= LogLevel::INFO) {
		colorPrint(COLOR_INFO, str.c_str(), newLine);
	}
}

void HAPLogger::logError(String str, bool newLine){
	if (HAPLogger::_logLevel >= LogLevel::ERROR) {
		colorPrint(COLOR_ERROR, str.c_str(), newLine);
	}
}

void HAPLogger::logDebug(String str, bool newLine){
	if (HAPLogger::_logLevel >= LogLevel::DEBUG) {
		colorPrint(COLOR_DEBUG, str.c_str(), newLine);
	}
}

void HAPLogger::logDebug(uint8_t str, bool newLine){
	if (HAPLogger::_logLevel >= LogLevel::DEBUG) {
		colorPrint(COLOR_DEBUG, str, newLine);
	}
}

void HAPLogger::logWarning(String str, bool newLine){
	if (HAPLogger::_logLevel >= LogLevel::WARNING) {
		colorPrint(COLOR_WARNING, str.c_str(), newLine);
	}
}

void HAPLogger::logVerbose(String str, bool newLine){
	if (HAPLogger::_logLevel >= LogLevel::VERBOSE) {
		colorPrint(COLOR_VERBOSE, str.c_str(), newLine);
	}
}

#if !defined(__APPLE__)
void HAPLogger::colorPrint(const char* color, const __FlashStringHelper * text, bool newLine){
	colorPrint(color, (PGM_P)text, newLine);
}
#endif

void HAPLogger::colorPrint(const char* color, int num, bool newLine) {
	_printer->print(color);
	_printer->print(num, DEC);

	if (newLine)
		_printer->println(COLOR_RESET);
	else
		_printer->print(COLOR_RESET);
}

void HAPLogger::colorPrint(const char* color, const char* text, bool newLine) {
	_printer->print(color);
	_printer->print(text);

	if (newLine)
		_printer->println(COLOR_RESET);
	else
		_printer->print(COLOR_RESET);
}

void HAPLogger::checkErrorOK(int err_code) {
	if (err_code != 0) {
		colorPrint(COLOR_ERROR, " ERROR: ", false);
		colorPrint(COLOR_ERROR, err_code, true);
	} else
		colorPrint(COLOR_GREEN, " OK", true);

	//	Serial.print("Free Heap: "); Serial.println(ESP.getFreeHeap());
}

void HAPLogger::checkError(int err_code) {
	if (err_code != 0) {
		colorPrint(COLOR_ERROR, " ERROR: ", false);
		colorPrint(COLOR_ERROR, err_code, true);
	}
}

void HAPLogger::setLogLevel(uint8_t lvl){
	_logLevel = (LogLevel)lvl;
}

void HAPLogger::setLogLevel(LogLevel lvl){
	_logLevel = lvl;
}

LogLevel HAPLogger::getLogLevel(){
	return _logLevel;
}

void HAPLogger::logOK(const char* color) {
	colorPrint(color, "OK", true);
}

void HAPLogger::setPrinter(Stream* printer) {
	_printer = printer;
}

void HAPLogger::flush(){
	_printer->flush();
}

/*
Stream* HAPLogger::stream(){
	return _printer;
}
*/

void HAPLogger::printInfo(){
	_printer->println();
	_printer->println( "These are the color definitions of the log output:");
	LogE( "ERROR   - This is an error message.", true);
	LogW( "WARNING - This is a warning.", true);
	LogI( "INFO    - This is information", true);
	LogV( "VERBOSE - These are verbose infos", true);
	LogD( "DEBUG   - And finally the debug messages", true);
	_printer->println();
}
