//
// HAPHelper.hpp
// HomekitAccessoryProtocol
//
//  Created on: 06.08.2017
//      Author: michael
//

#ifndef HAPHELPER_HPP_
#define HAPHELPER_HPP_

#include <Arduino.h>
#include <ArduinoJson.h>

#include "mbedtls/bignum.h"

class HAPHelper {
public:
	HAPHelper();
	~HAPHelper();

	// static union {
	// 	uint32_t bit32;
	// 	uint8_t bit8[4];
	// } HAPBit32to8Converter;

	
	// static String getValue(String data, char separator, int index) __attribute__ ((deprecated));	
	
	static void binToHex(const unsigned char * in, size_t insz, char * out, size_t outsz) __attribute__ ((deprecated));

	// DEPRECATED functions causing memory leaks	
	static byte* hexToBin(const char* string) __attribute__ ((deprecated));	
	static char* toHex(const unsigned char * in, size_t insz) __attribute__ ((deprecated));	

	static void prependZeros(char *dest, const char *src, uint8_t width); // __attribute__ ((deprecated)); 

	static uint8_t numDigits(const size_t n);
	static void arrayPrint(uint8_t* a, int len);
	
	static String wrap(String str);
	static String wrap(const char *str);
	static String arrayWrap(String *s, unsigned short len);
	static String dictionaryWrap(String *key, String *value, unsigned short len);
	static String removeBrackets(String str);

	static String printUnescaped(String str);

	static bool containsNestedKey(const JsonObject obj, const char* key);
	static void mergeJson(JsonDocument& dst, const JsonObject& src);
	

	static void mpi_print(const char* tag, const mbedtls_mpi* x);
	static void array_print(const char* tag, const unsigned char* buf, int len);

	static void getPartionTableInfo();

	static bool isValidFloat(String tString);
	static bool isValidNumber(String str);


	
};

#endif /* HAPHELPER_HPP_ */
