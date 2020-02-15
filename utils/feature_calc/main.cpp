

// Add new plugins on top
#define HAP_PLUGIN_USE_SWITCH0		1
#define HAP_PLUGIN_USE_SWITCH1		0
#define HAP_PLUGIN_USE_SWITCH2		0
#define HAP_PLUGIN_USE_SWITCH3		0
#define HAP_PLUGIN_USE_SWITCH4		0
#define HAP_PLUGIN_USE_SWITCH5		0
#define HAP_PLUGIN_USE_SWITCH6		0
#define HAP_PLUGIN_USE_SWITCH7		0
#define HAP_PLUGIN_USE_SWITCH8		0
#define HAP_PLUGIN_USE_SWITCH9		0


#define HAP_PLUGIN_USE_SWITCH10		0
#define HAP_PLUGIN_USE_SWITCH11		0
#define HAP_PLUGIN_USE_SWITCH12		0
#define HAP_PLUGIN_USE_SWITCH13		0
#define HAP_PLUGIN_USE_SWITCH14		0
#define HAP_PLUGIN_USE_SWITCH15		0
#define HAP_PLUGIN_USE_SWITCH16		0
#define HAP_PLUGIN_USE_SWITCH17		0
#define HAP_PLUGIN_USE_SWITCH18		0
#define HAP_PLUGIN_USE_SWITCH19		0

#define HAP_PLUGIN_USE_LED			1

#define HAP_PLUGIN_USE_SWITCH		0

#define HAP_PLUGIN_USE_MIFLORA		0	// deprecated !!!

#define HAP_PLUGIN_USE_MIFLORA2		0

#define HAP_PLUGIN_USE_SSD1331		0
#define HAP_PLUGIN_USE_PCA301		0
#define HAP_PLUGIN_USE_NEOPIXEL		0

#define HAP_PLUGIN_USE_INFLUXDB		1

#define HAP_PLUGIN_USE_HYGROMETER	0
#define HAP_PLUGIN_USE_RCSWITCH		1
#define HAP_PLUGIN_USE_DHT			0
#define HAP_PLUGIN_USE_BME280		1	// < last digit of feature number


#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define TEST_LONG 0

#if TEST_LONG
#define HAP_PLUGIN_FEATURE_NUMBER \
STR(HAP_PLUGIN_USE_SWITCH0) \
STR(HAP_PLUGIN_USE_SWITCH1) \
STR(HAP_PLUGIN_USE_SWITCH2) \
STR(HAP_PLUGIN_USE_SWITCH3) \
STR(HAP_PLUGIN_USE_SWITCH4) \
STR(HAP_PLUGIN_USE_SWITCH5) \
STR(HAP_PLUGIN_USE_SWITCH6) \
STR(HAP_PLUGIN_USE_SWITCH7) \
STR(HAP_PLUGIN_USE_SWITCH8) \
STR(HAP_PLUGIN_USE_SWITCH9) \
STR(HAP_PLUGIN_USE_SWITCH10) \
STR(HAP_PLUGIN_USE_SWITCH11) \
STR(HAP_PLUGIN_USE_SWITCH12) \
STR(HAP_PLUGIN_USE_SWITCH13) \
STR(HAP_PLUGIN_USE_SWITCH14) \
STR(HAP_PLUGIN_USE_SWITCH15) \
STR(HAP_PLUGIN_USE_SWITCH16) \
STR(HAP_PLUGIN_USE_SWITCH17) \
STR(HAP_PLUGIN_USE_SWITCH18) \
STR(HAP_PLUGIN_USE_SWITCH19) \
STR(HAP_PLUGIN_USE_LED) \
STR(HAP_PLUGIN_USE_SWITCH) \
STR(HAP_PLUGIN_USE_MIFLORA) \
STR(HAP_PLUGIN_USE_MIFLORA2) \
STR(HAP_PLUGIN_USE_SSD1331) \
STR(HAP_PLUGIN_USE_PCA301) \
STR(HAP_PLUGIN_USE_NEOPIXEL) \
STR(HAP_PLUGIN_USE_INFLUXDB) \
STR(HAP_PLUGIN_USE_HYGROMETER) \
STR(HAP_PLUGIN_USE_RCSWITCH) \
STR(HAP_PLUGIN_USE_DHT) \
STR(HAP_PLUGIN_USE_BME280) 


#else



#define HAP_PLUGIN_FEATURE_NUMBER \
STR(HAP_PLUGIN_USE_LED) \
STR(HAP_PLUGIN_USE_SWITCH) \
STR(HAP_PLUGIN_USE_MIFLORA) \
STR(HAP_PLUGIN_USE_MIFLORA2) \
STR(HAP_PLUGIN_USE_SSD1331) \
STR(HAP_PLUGIN_USE_PCA301) \
STR(HAP_PLUGIN_USE_NEOPIXEL) \
STR(HAP_PLUGIN_USE_INFLUXDB) \
STR(HAP_PLUGIN_USE_HYGROMETER) \
STR(HAP_PLUGIN_USE_RCSWITCH) \
STR(HAP_PLUGIN_USE_DHT) \
STR(HAP_PLUGIN_USE_BME280) 
#endif


#define HEX__(n) 0x##n##LU
#define B8__(x) ((x&0x0000000FLU)?1:0) \
+((x&0x000000F0LU)?2:0) \
+((x&0x00000F00LU)?4:0) \
+((x&0x0000F000LU)?8:0) \
+((x&0x000F0000LU)?16:0) \
+((x&0x00F00000LU)?32:0) \
+((x&0x0F000000LU)?64:0) \
+((x&0xF0000000LU)?128:0)

// User-visible Macros
#define B8(d) ((unsigned char)B8__(HEX__(d)))
#define B16(dmsb,dlsb) (((unsigned short)B8(dmsb)<<8) + B8(dlsb))
#define B32(dmsb,db2,db3,dlsb) \
(((unsigned long)B8(dmsb)<<24) \
+ ((unsigned long)B8(db2)<<16) \
+ ((unsigned long)B8(db3)<<8) \
+ B8(dlsb))



#include <iostream>

int main() 
{
    std::cout << HAP_PLUGIN_FEATURE_NUMBER << std::endl;

    const char* binaryString = HAP_PLUGIN_FEATURE_NUMBER;
    std::cout << "binaryString: " << binaryString << std::endl;


    // convert binary string to integer
	uint64_t value = (uint64_t)strtol(binaryString, NULL, 2);
	std::cout << "value: " << value << std::endl;
	// convert integer to hex string
	char hexString[32]; // long enough for any 32-bit value, 4-byte aligned
	sprintf(hexString, "%llx", value);

	const char* comp = "10000000000100010000100000010101";

	std::cout << "comp: " << comp << std::endl;

	uint64_t compValue = (uint64_t)strtol(comp, NULL, 2);

	uint64_t result = compValue & value;

	

	bool res = result == value;
	
	std::cout << "compare result: " << res << std::endl;
	std::cout << "hexString: " << hexString << std::endl;


	uint64_t number = (uint64_t)strtol(hexString, NULL, 16);
	std::cout << "number: " << number << std::endl;
	
	// 2,164,238,933
	std::cout << B32(00000000,00000000,00001000,00010101) << std::endl;
	


    return 0;
}