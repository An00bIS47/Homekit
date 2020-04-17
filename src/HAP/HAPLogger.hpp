//
// HAPLogger.hpp
// Homekit
//
//  Created on: 08.08.2017
//      Author: michael
//

#ifndef HAPLOGGER_HPP_
#define HAPLOGGER_HPP_

#include <Arduino.h>
#include <string>

#define COLOR_RESET "\033[0m"

#define COLOR_BLACK 	"\033[0;30m"
#define COLOR_RED 		"\033[0;31m"
#define COLOR_GREEN 	"\033[0;32m"
#define COLOR_YELLOW 	"\033[0;33m"
#define COLOR_BLUE	 	"\033[0;34m"
#define COLOR_MAGENTA 	"\033[0;35m"
#define COLOR_CYAN		"\033[0;36m"
#define COLOR_WHITE		"\033[0;37m"

#define COLOR_INFO 		COLOR_GREEN
#define COLOR_ERROR 	COLOR_RED
#define COLOR_WARNING	COLOR_YELLOW
#define COLOR_DEBUG		COLOR_CYAN
#define COLOR_VERBOSE	COLOR_MAGENTA


#define Heap(X, Y)		HAPLogger::logFreeHeap(X, Y)
#define Log(X, Y, Z) 	HAPLogger::colorPrint(X,Y,Z)

#define LogOK(X)		HAPLogger::logOK(X)

#define LogI(Y, Z) HAPLogger::logInfo(Y,Z)
#define LogE(Y, Z) HAPLogger::logError(Y,Z)
#define LogV(Y, Z) HAPLogger::logVerbose(Y,Z)
#define LogW(Y, Z) HAPLogger::logWarning(Y,Z)
#define LogD(Y, Z) HAPLogger::logDebug(Y,Z)
#define LogFlush() HAPLogger::flush()

#define CheckError(X) HAPLogger::checkError(X)
#define CheckErrorOk(X) HAPLogger::checkErrorOk(X)


inline const char* className(const std::string& prettyFunction)
{
    size_t colons = prettyFunction.find("::");
    if (colons == std::string::npos)
        return "::";
    size_t begin = prettyFunction.substr(0,colons).rfind(" ") + 1;
    size_t end = colons - begin;

    return prettyFunction.substr(begin,end).c_str();
}
#define __CLASS_NAME__ className(__PRETTY_FUNCTION__)


enum LogLevel {
	NO_LOG 	= 0,
	ERROR 	= 1,
	WARNING = 2,
	INFO 	= 3,
	VERBOSE = 4,
	DEBUG 	= 5,
};

class HAPLogger {
public:
	HAPLogger();
	~HAPLogger();

	static void setLogLevel(LogLevel lvl);
	static void setLogLevel(uint8_t lvl);
	static LogLevel getLogLevel();

	static void setPrinter(Stream* printer);
	//static Stream* stream();

	static void printInfo();
	static void logFreeHeap(int clients = 0, int queue = 0, const char* color = COLOR_WARNING);

	static void colorPrint(const char* color, int num, bool newLine);
	static void colorPrint(const char* color, const char* text, bool newLine);
#if !defined(__APPLE__)	
	static void colorPrint(const char* color, const __FlashStringHelper * text, bool newLine);
#endif

	static void logError(String str, bool newLine = true);
	static void logWarning(String str, bool newLine = true);
	static void logInfo(String str, bool newLine = true);
	static void logVerbose(String str, bool newLine = true);
	static void logDebug(String str, bool newLine = true);

	static void logDebug(uint8_t str, bool newLine);

	static void logOK(const char* color);

	static void flush();
	static void checkError(int err_code);
	static void checkErrorOK(int err_code);
private:
	static LogLevel _logLevel;
	static Stream* _printer;

};

#endif /* HAPLOGGER_HPP_ */
