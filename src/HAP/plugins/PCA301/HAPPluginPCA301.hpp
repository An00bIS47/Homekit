//
// HAPPluginPCA301.hpp
// Homekit
//
//  Created on: 19.08.2019
//      Author: michael
//

#ifndef HAPPLUGINPCA301_HPP_
#define HAPPLUGINPCA301_HPP_

#include <Arduino.h>
#include <SPI.h>
#include <vector>
#include <algorithm>

#include "HAPPlugins.hpp"
#include "HAPLogger.hpp"
#include "HAPAccessory.hpp"
#include "HAPFakeGato.hpp"

#include "HAPPluginPCA301Device.hpp"

#include "funky_rfm69.h"
#include "pca301_rfm69.h"

#define NODEID           24

#define RF_MAX   		(RFM69_MAXDATA + 5)    // maximum transmit / receive buffer: 3 header + data + 2 crc bytes
#define RF_FREQ_BASE     868000         // frequency base

// #define PROGNAME         "pcaSerial"
// #define PROGVERS         "10.1"


// 	Arduino UNO
// 		  INT SS  MOS MIS SCK
//  Sparkfun ESP Thing
//        17  2   23  19  18
//  ESP32 Thing Plus / Huzzah32 
// 		  A1  A5  18  19  5			=> 		A5 == GPIO 4
// 									=>		A1 == GPIO 25
// 
//  | |	  |	  |	  |	  |	  |	  |	  |  |
//  | 3V3 D2  D10 D11 D12 D13 GND 5V |  
//  |     IRQ SEL SDI SDO SCK		 |
//  |                                |
//  |  -=========-      -=========-  |
//  |  -=========-      -=========-  |
//  |  -=========-      -=========-  |
//  |                                |
//  |                                |
//  |                                |
//  |                                |
//  |                                |
//  |                                |
//  |                           ANT -|
//  |________________________________|
// 
// 

#define RFM69_IS_HW                 false
#define PCA301_FREQ_CARRIER_KHZ     868950 
#define PCA301_BITRATE_BS           6631	

//  Sparkfun ESP32 Thing Plus / Huzzah32 
// 		  A1  A5  18  19  5			=> 		A5 == GPIO 4
// 									=>		A1 == GPIO 25
#define PCA301_PIN_SPI_CLK          SCK 	// 5		// SPI CLOCK PIN				(18)			(18)			(5)
#define PCA301_PIN_SPI_MISO         MISO 	// 19		// SPI MISO PIN					(19)			(19)			(19)
#define PCA301_PIN_SPI_MOSI         MOSI	// 18 		// SPI MOSI PIN					(23)			(23)			(18)
#define PCA301_PIN_SPI_SS           A5		// 4		// sPI Slave seclect PIN		(5)				(2)				(4)
#define PCA301_PIN_INT              A1		// 25		// Interrupt PIN 				(2)				(17)			(25)


#define PCA301_PACKET_LENGTH 		10

class HAPPluginPCA301: public HAPPlugin {
public:
	
	HAPPluginPCA301();
	HAPAccessory* initAccessory() override;

	bool begin();

	// void setPowerState(int iid, bool oldValue, bool newValue);
	// void setPowerCurrent(int iid, float oldValue, float newValue);
	// void setPowerTotal(int iid, float oldValue, float newValue);

	// String getValue(int iid);

	void handleImpl(bool forced=false);
	void identify(int iid, bool oldValue, bool newValue);
	
	HAPConfigValidationResult validateConfig(JsonObject object);
	JsonObject getConfigImpl();
	void setConfigImpl(JsonObject root);

	// void handleEvents(int eventCode, struct HAPEvent eventParam);
	// void handleRoot(HTTPRequest * req, HTTPResponse * res);

	void sendDeviceCallback(uint32_t devId, char cmd_);
	
private:	

	

	int indexOfDevice(HAPPluginPCA301Device* device);

	std::vector<HAPPluginPCA301Device*>	_devices;

	static uint8_t pca301_sync_values[2]; //= { 0x2d, 0xd4 }; /**< sync word values */

	uint32_t rfm69_center_freq; // = 868950;    // center frequency
	char cmd;
	String freq;
	byte value;
	byte stack[RFM69_MAXDATA+4];
	byte top;
	byte sendLen;
	byte pBuf[RFM69_MAXDATA];

	// struct_pcaConf pcaConf;

	uint16_t eeprom_crc;                     	// eeprom crc
	uint16_t rfm69_crc; // = 0;                 // running crc value
	uint8_t  rfm69_buf[RF_MAX];               	// recv/xmit buf, including hdr & crc bytes
	uint8_t  rxfill; // = 0;                    // RX fill level
	uint8_t  rfm69_len; // = 7;                 // fixed calculation value
	
	// ToDo: remove unneccessary 
	uint8_t  numDev;                      		// devices in use __deprecated__
  	uint16_t pollIntv;                    		// polling intervall in 1/10th of seconds for regular devices
  	uint16_t deadIntv;                    		// retry intervall in 1/10th of seconds for dead devices
  	uint8_t  quiet;  							// surpress wrong messages received
	uint16_t crc;								// crc calcualtion


	void configAccessory(uint8_t devPtr);

	static void pca301_board_init();
	static void pca301_rfm69_init();

	void pca301serial_loop_pre();
	bool pca301serial_loop();
	void pca301serial_setup();

	void handleInput (char c);
	void showHelp();
	

	void pcaTask();
	void setNextTX (uint32_t devId, uint8_t nextTX);
	void analyzePacket();
	void sendDevice(uint8_t devPtr, char cmd);
	int getDevice(uint32_t devId);
	
	
	void modifyConf(volatile uint8_t value);
	void reportConf(uint8_t repMode);
	bool loadConf();
	void saveConf();
	void eraseConf();
	void fillConf();

	static void showByte (byte value);
	static void showNibble(byte nibble);	
	static uint32_t mem2devId(volatile uint8_t * data);
	static uint32_t mem2long(volatile uint8_t * data);
	static uint16_t mem2word(volatile uint8_t * data);
	// static void displayVersion(uint8_t newline);
	static uint16_t hexToUInt16(String hexString);
	static uint16_t crc16_pca301_update(uint16_t crc, uint8_t data);

	// unsigned long 		_interval;
	// unsigned long 		_previousMillis;

	// EventManager*	_eventManager;
	// MemberFunctionCallable<HAPPlugin> listenerMemberFunctionPlugin;

};



REGISTER_PLUGIN(HAPPluginPCA301)

#endif /* HAPPLUGINPCA301_HPP_ */ 