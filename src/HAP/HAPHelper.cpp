//
// HAPHelper.cpp
// HomekitAccessoryProtocol
//
//  Created on: 06.08.2017
//      Author: michael
//

#include "HAPHelper.hpp"


#if !defined (__APPLE__)
#include <esp_partition.h>
#include <Print.h>
#endif
//#include <stdlib.h>



//convert hexstring to unsigned char
//returns 0 on success, -1 on error
//data is a buffer of at least len bytes
//hexstring is upper or lower case hexadecimal, NOT prepended with "0x"
uint8_t* HAPHelper::hexToBin(const char* string)
{

	if(string == nullptr)
	{
	   return nullptr;
	}

	size_t slength = strlen(string);

	if(slength % 2 != 0) // must be even
	{
	   return nullptr;
	}
	size_t dlength = slength / 2;

	unsigned char* data = (unsigned char*) malloc(sizeof(unsigned char) * dlength);
	memset(data, 0, dlength);

	size_t index = 0;

	while (index < slength)
	{
		char c = string[index];

		int value = 0;
		if(c >= '0' && c <= '9')
		{
			value = (c - '0');
		}
		else if (c >= 'A' && c <= 'F')
		{
			value = (10 + (c - 'A'));
		}
		else if (c >= 'a' && c <= 'f')
		{
			 value = (10 + (c - 'a'));
		}
		else
		{
			free(data);
			return nullptr;
		}

		data[(index/2)] += value << (((index + 1) % 2) * 4);

		index++;
	}

	return data;
}

void HAPHelper::binToHex(const unsigned char * in, size_t insz, char * out, size_t outsz)
{
	unsigned char * pin = (unsigned char *)in;
	const char * hex = "0123456789ABCDEF";
	char * pout = out;
	for(; pin < in+insz; pout +=2, pin++){
		pout[0] = hex[(*pin>>4) & 0xF];
		pout[1] = hex[ *pin     & 0xF];
		//pout[2] = ':';
		if (pout + 2 - out > outsz){
			/* Better to truncate output string than overflow buffer */
			/* it would be still better to either return a status */
			/* or ensure the target buffer is large enough and it never happen */
			break;
		}
	}
	pout[0] = 0;
}

char* HAPHelper::toHex(const unsigned char* in, size_t insz) {
	char *out;
	out = (char*)malloc(sizeof(char) * (insz * 2) + 1);

	HAPHelper::binToHex(in, insz, out, (insz * 2) + 1);
	return out;
}

// This Chunk of code takes a string and separates it based on a given character 
// and returns The item between the separating character
// String HAPHelper::getValue(String data, char separator, int index)
// {
// 	int found = 0;
// 	int strIndex[] = { 0, -1 };
// 	int maxIndex = data.length() - 1;

// 	for (int i = 0; i <= maxIndex && found <= index; i++) {
// 		if (data.charAt(i) == separator || i == maxIndex) {
// 			found++;
// 			strIndex[0] = strIndex[1] + 1;
// 			strIndex[1] = (i == maxIndex) ? i+1 : i;
// 		}
// 	}
// 	return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
// }

uint8_t HAPHelper::numDigits(const size_t n) {
	if (n < 10) return 1;
	return 1 + numDigits(n / 10);
}


void HAPHelper::arrayPrint(uint8_t* a, int len)
{
	for (int i=0; i<len; i++) {
		if (i != 0 && (i % 0x10) == 0) {
			printf("\n");
		}
		printf("%02X ", a[i]);
	}
	printf("\n");
}

void HAPHelper::prependZeros(char *dest, const char *src, uint8_t width) {
	size_t len = strlen(src);
	size_t zeros = (len > width) ? 0 : width - len;
	memset(dest, '0', zeros);
	strcpy(dest + zeros, src);
}

String HAPHelper::removeBrackets(String str){
	if (str.length() > 2) {
    	str = str.substring(1, str.length()-1);
	}
	return str;	
}


String HAPHelper::wrap(String str) { 
	return String("\"" + str + "\""); 
}

String HAPHelper::wrap(const char *str) { 
	return String("\"" + String(str) + "\""); 
}

String HAPHelper::arrayWrap(String *s, unsigned short len) {
	String result;	
	result = "[";
	
	for (int i = 0; i < len; i++) {
		result += s[i]+",";
	}
	if (len > 0)
		result = result.substring(0, result.length()-1);
	
	result += "]";	
	return result;
}


String HAPHelper::dictionaryWrap(String *key, String *value, unsigned short len) {
	String result;
	
	result += "{";
	
	for (int i = 0; i < len; i++) {
		result += wrap(key[i].c_str())+":"+value[i]+",";
	}
	result = result.substring(0, result.length()-1);
	
	result += "}";
	
	return result;
}





String HAPHelper::printUnescaped(String str) {
	String result;
	result = str;
	result.replace('\n', '\\ n\n');
	result.replace('\r', '\\ r');
	return result;
}

void HAPHelper::array_print(const char* tag, const unsigned char* buf, int len)
{
    printf("=== [%s] (len:%d) ===\n", tag, len);
	int need_lf=1;
    for (int i=0; i<len; i++) {
        if ((i & 0xf) == 0xf) {
	        printf("%02X\n", buf[i]);
			need_lf=0;
		} else {
	        printf("%02X ", buf[i]);
			need_lf=1;
		}
    }
    if (need_lf){
		printf("\n======\n");
	} else {
		printf("======\n");
	}
}

void HAPHelper::mpi_print(const char* tag, const mbedtls_mpi* x)
{
    int len_x = mbedtls_mpi_size(x);
    unsigned char* num = (unsigned char*) malloc(sizeof(unsigned char) * len_x);
    mbedtls_mpi_write_binary(x, num, len_x);
	HAPHelper::array_print(tag,num,len_x);
    free(num);
}


#if !defined (__APPLE__)

bool HAPHelper::containsNestedKey(const JsonObject obj, const char* key) {
	for (const JsonPair pair : obj) {
		if (!strcmp(pair.key().c_str(), key))
			return true;

		if (containsNestedKey(pair.value().as<JsonObject>(), key)) 
			return true;
	}

	return false;
}


void HAPHelper::mergeJson(JsonDocument& dst, const JsonObject& src) {
    for (auto p : src) {
        dst[p.key()] = p.value();
    } 
}

void HAPHelper::getPartionTableInfo()
{
	// Get Partitionsizes
	size_t ul;
	esp_partition_iterator_t _mypartiterator;
	const esp_partition_t *_mypart;
	ul = spi_flash_get_chip_size();
	Serial.print("Flash chip size: ");
	Serial.println(ul);
	Serial.println("Partition table:");
	_mypartiterator = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, NULL);
	if (_mypartiterator)
	{
		Serial.println("App Partition table:");
		// do
		// {
		// 	_mypart = esp_partition_get(_mypartiterator);
		// 	printf("Type: %02x SubType %02x Address 0x%06X Size 0x%06X Encryption %i Label %s\r\n", _mypart->type, _mypart->subtype, _mypart->address, _mypart->size, _mypart->encrypted, _mypart->label);
		// } while (_mypartiterator = (esp_partition_next(_mypartiterator)));
		while (_mypartiterator != NULL)
		{
			_mypart = esp_partition_get(_mypartiterator);
			printf("Type: %02x SubType %02x Address 0x%06X Size 0x%06X Encryption %i Label %s\r\n", _mypart->type, _mypart->subtype, _mypart->address, _mypart->size, _mypart->encrypted, _mypart->label);
			_mypartiterator = esp_partition_next(_mypartiterator);
		}
	}
	esp_partition_iterator_release(_mypartiterator);
	_mypartiterator = esp_partition_find(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, NULL);
	if (_mypartiterator)
	{
		Serial.println("Data Partition table:");
		// do
		// {
		// 	_mypart = esp_partition_get(_mypartiterator);
		// 	printf("Type: %02x SubType %02x Address 0x%06X Size 0x%06X Encryption %i Label %s\r\n", _mypart->type, _mypart->subtype, _mypart->address, _mypart->size, _mypart->encrypted, _mypart->label);
		// } while (_mypartiterator = (esp_partition_next(_mypartiterator)));
		while (_mypartiterator != NULL)
		{
			_mypart = esp_partition_get(_mypartiterator);
			printf("Type: %02x SubType %02x Address 0x%06X Size 0x%06X Encryption %i Label %s\r\n", _mypart->type, _mypart->subtype, _mypart->address, _mypart->size, _mypart->encrypted, _mypart->label);
			_mypartiterator = esp_partition_next(_mypartiterator);
		}
	}
	esp_partition_iterator_release(_mypartiterator);
}
#endif


bool HAPHelper::isValidFloat(String tString) {
	String tBuf;
	bool decPt = false;
	
	if(tString.charAt(0) == '+' || tString.charAt(0) == '-') tBuf = &tString[1];
	else tBuf = tString;  

	for(int x=0;x<tBuf.length();x++)
	{
		if(tBuf.charAt(x) == '.') {
		if(decPt) return false;
		else decPt = true;  
		}    
		else if(tBuf.charAt(x) < '0' || tBuf.charAt(x) > '9') return false;
	}
	return true;
}

bool HAPHelper::isValidNumber(String str){
   bool isNum=false;
   for(uint8_t i=0;i<str.length();i++)
   {

#if defined(__APPLE__)
   		isNum = isdigit(str.charAt(i)) || str.charAt(i) == '+' || str.charAt(i) == '.' || str.charAt(i) == '-';
#else   	
       isNum = isDigit(str.charAt(i)) || str.charAt(i) == '+' || str.charAt(i) == '.' || str.charAt(i) == '-';
#endif 
       if(!isNum) return false;
   }
   return isNum;
}



