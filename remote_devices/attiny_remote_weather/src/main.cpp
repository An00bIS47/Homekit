//
// main.cpp
// Homekit Remote Weather for ATTiny85
//  
//  Created on: 19.05.2020
//      Author: michael
// 
// ATTINY25/45/85 pinout for ARDUINO
//
//                  +-\/-+
// Ain0 (D 5) PB5  1|    |8  Vcc
// Ain3 (D 3) PB3  2|    |7  PB2 (D 2) Ain1
// Ain2 (D 4) PB4  3|    |6  PB1 (D 1) pwm1
//            GND  4|    |5  PB0 (D 0) pwm0
//                  +----+
// 
// Uses 3 Pins NRF24 connection!
// 

#include <avr/eeprom.h>
#include <avr/power.h>      // Power management
#include <avr/sleep.h>      // Sleep Modes
#include <avr/wdt.h>        // Watchdog timer

#include "RF24.h"

#define DEBUG	        // comment out to disable
#define USE_DHT         // comment out to disable and use bme280 i2c
// #define RESET_EEPROM    // comment out to disable


#ifndef RF24_ADDRESS
#define RF24_ADDRESS        "HOMEKIT_RF24"
#endif 

#define RF24_ADDRESS_SIZE   13
#define RF24_PA_LEVEL       RF24_PA_HIGH
#define RF24_DATA_RATE      RF24_250KBPS

#ifndef RF24_ID
#define RF24_ID 0x01
#endif


#define DELAY_INTERVAL 32000 // in ms (advice a power of 8)
// #define DELAY_INTERVAL 16000 // in ms (advice a power of 8)

#define CE_PIN  PB2
#define CSN_PIN PB2 //Since we are using 3 pin configuration we will use same pin for both CE and CSN

uint8_t address[RF24_ADDRESS_SIZE] = RF24_ADDRESS;


#ifdef USE_DHT
#include "dht.h"

#define DHT22_PIN PB3

#else

#ifndef BME280_ADDRESS
#define BME280_ADDRESS 0x76
#endif 

#define TINY_BME280_I2C

#include <TinyBME280.h>
#include <Wire.h>
#endif


#ifdef DEBUG
//    Senden via "SoftwareSerial" - TX an Pin  4 (= Pin3 am Attiny85-20PU)
// Empfangen via "SoftwareSerial" - RX an Pin 99 (Dummy um Hardwarepin zu sparen)
#include <SoftwareSerial.h>

#define SOFTSERIAL_PIN  4   // TX an Pin  4 (= Pin3 am Attiny85-20PU)
SoftwareSerial softSerial(99, SOFTSERIAL_PIN); // RX, TX
#endif


#define eepromBegin() eeprom_busy_wait(); noInterrupts() // Details on https://youtu.be/_yOcKwu7mQA
#define eepromEnd()   interrupts()


const uint8_t EEPROM_SETTINGS_VERSION = 1;

struct EepromSettings
{
    uint8_t     radioId;
    uint32_t    sleepIntervalSeconds;
    uint8_t     measureMode;
    uint8_t     version;
};


struct RadioPacket
{
    uint8_t     radioId;
    uint8_t     type;
    
    uint32_t    temperature;    // temperature
    uint32_t    humidity;       // humidity
    uint16_t    pressure;       // pressure
    
    uint16_t    voltage;       // percentage
};



enum RemoteDeviceType {
    RemoteDeviceTypeWeather    = 0x01,
    RemoteDeviceTypeDHT	       = 0x02,
};


enum MeasureMode
{
	MeasureModeWeatherStation   = 0x01,
	MeasureModeIndoor           = 0x11,
};


enum ChangeType
{
    ChangeRadioId               = 0x00,
    ChangeSleepInterval         = 0x01,
    ChangeMeasureType           = 0x02,
};


struct NewSettingsPacket
{
    enum ChangeType changeType;
    uint8_t         forRadioId;
    uint8_t         newRadioId;
    uint32_t        newSleepIntervalSeconds;
    uint8_t         newMeasureMode;
};


#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif




#ifdef USE_DHT
#else
void setupBME280();
#endif

uint16_t readVcc();
void processSettingsChange(NewSettingsPacket newSettings); // Need to pre-declare this function since it uses a custom struct as a parameter (or use a .h file instead).
void setupRadio();
void saveSettings();
void system_sleep();
void setup_watchdog(int ii);


#ifdef USE_DHT
dht 			_dht;          // Create instance of DHT22 module object
#else
tiny::BME280    _sensor;
#endif

RF24 			_radio(CE_PIN, CSN_PIN);
EepromSettings  _settings;

volatile boolean f_wdt = 1;
volatile uint8_t counter = 0;


uint16_t readVcc()
{
    power_adc_enable() ;
    // Details on http://provideyourown.com/2012/secret-arduino-voltmeter-measure-battery-voltage/

    ADMUX = _BV(MUX3) | _BV(MUX2); // Select internal 1.1V reference for measurement.
    delay(2);                      // Let voltage stabilize.
    ADCSRA |= _BV(ADSC);           // Start measuring.
    while (ADCSRA & _BV(ADSC));    // Wait for measurement to complete.
    uint16_t adcReading = ADC;
    uint16_t vcc = (1.1 * 1024.0 / adcReading) * 100; // Note the 1.1V reference can be off +/- 10%, so calibration is needed.

    power_adc_disable();
    return vcc;
}


void setup() {
    
    // Enable the power bus.
    // pinMode(PIN_POWER_BUS, OUTPUT);
    // digitalWrite(PIN_POWER_BUS, LOW);    

#ifdef DEBUG
    softSerial.begin(9600);
    
    delay(3000);

    softSerial.println(F("Starting Homekit Remote"));

    softSerial.print(F("Reading settings ..."));
#endif

    // Load settings from eeprom.
    eepromBegin();

#ifdef RESET_EEPROM
    
    eeprom_write_block(0xFF, 0, sizeof(_settings));
#endif

    
    eeprom_read_block(&_settings, 0, sizeof(_settings));
    eepromEnd();

    if (_settings.version == EEPROM_SETTINGS_VERSION) {
#ifdef DEBUG
        softSerial.println(F("OK"));
#endif                
    } else {
        // The settings version in eeprom is not what we expect, so assign default values.
#ifdef DEBUG        
        softSerial.println(F("ERROR -> Assigning defaults"));
#endif        

        _settings.radioId = RF24_ID;        
        _settings.sleepIntervalSeconds = DELAY_INTERVAL;        
        _settings.measureMode = MeasureModeWeatherStation;        
        _settings.version = EEPROM_SETTINGS_VERSION;
        saveSettings();
    }

#ifdef USE_DHT
#else
     
    setupBME280();
#endif

    // 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
    // 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec
    setup_watchdog(9); // approximately 8 seconds sleep
}

void setupRadio()
{


#ifdef DEBUG        
        softSerial.print(F("Setup RF24 ..."));    
#endif  


    if (_radio.begin() ){  
#ifdef DEBUG                            // Start up the radio            
        softSerial.println(F("OK - RF24 wiring OK!"));
#endif        
        _radio.setPALevel(RF24_PA_LEVEL);   // You can set it as minimum or maximum depending on the distance between the transmitter and receiver.
        _radio.setDataRate(RF24_DATA_RATE);
        _radio.setAutoAck(1);               // Ensure autoACK is enabled
        _radio.setRetries(15,15);           // Max delay between retries & number of retries
        _radio.openWritingPipe(address);    // Write to device address 'HOMEKIT_RF24'
#ifdef DEBUG                                
    } else {
        softSerial.println(F("ERROR: RF24 not found! Please check wiring!")); 
#endif               
    }  
}


#ifdef USE_DHT
#else
void setupBME280(){

	_sensor.beginI2C(BME280_ADDRESS); 

	if (_settings.measureMode == MeasureModeWeatherStation) {

		// set mode forced
    	_sensor.setMode(MeasureModeWeatherStation);

		// Oversampling settings: pressure *1, temperature *1, humidity *1    
	    _sensor.setHumidityOverSample(1);
	    _sensor.setTempOverSample(1);
	    _sensor.setPressureOverSample(1);
	    
	    // IIR filter settings: filter off
	    _sensor.setFilter(0);
    } else if (_settings.measureMode == MeasureModeIndoor) {

		// set mode to normal
    	_sensor.setMode(MeasureModeIndoor);

		//	normal mode, 16x pressure / 2x temperature / 1x humidity oversampling,");
		// Serial.println("0.5ms standby period, filter 16x");
	    _sensor.setHumidityOverSample(1);
	    _sensor.setTempOverSample(1);
	    _sensor.setPressureOverSample(1);
	    _sensor.setStandbyTime(0);

	    // IIR filter settings: filter off
	    _sensor.setFilter(16);
    }
}
#endif


void loop() {

    if (f_wdt==1) { 
        f_wdt=0; 

        setupRadio();                        // Re-initialize the radio.
        
        RadioPacket radioData;

#ifdef USE_DHT        
        radioData.type = RemoteDeviceTypeDHT;
#else
        radioData.type = RemoteDeviceTypeWeather;
#endif

        radioData.radioId = _settings.radioId;

        int status = -7;

#ifdef USE_DHT
#ifdef DEBUG                            // Start up the radio            
        softSerial.print(F("Reading DHT22 ..."));
#endif 
        do {
            status = _dht.read22(DHT22_PIN);
            radioData.temperature = (_dht.temperature * 100);
            radioData.humidity = (_dht.humidity * 100);
            radioData.pressure = 0;

            if (status != DHTLIB_OK) {
                delay(1000);
            }

        } while(status != DHTLIB_OK);

#ifdef DEBUG                            
	    softSerial.println(F("OK"));
#endif 

#else
#ifdef DEBUG                            
	    softSerial.print(F("Reading BME280 ..."));
#endif 

        radioData.temperature   = _sensor.readFixedTempC();
        radioData.humidity      = _sensor.readFixedHumidity();
        radioData.pressure      = _sensor.readFixedPressure();
#ifdef DEBUG                            
        softSerial.println(F("OK"));
#endif 
#endif

        radioData.voltage       = readVcc();


#ifdef DEBUG
        softSerial.println(F("Sensor values:"));
        softSerial.print(F("  Temp: "));
        softSerial.println(radioData.temperature);
        softSerial.print(F("  Hum:  "));
        softSerial.println(radioData.humidity);
        softSerial.print(F("  Pres: "));
        softSerial.println(radioData.pressure);
        softSerial.print(F("  Voltage: "));
        softSerial.println(radioData.voltage);

        softSerial.print(F("Size of struct: "));
        softSerial.println( sizeof(struct RadioPacket) );
#endif
    
     
        if (!_radio.write( &radioData, sizeof(RadioPacket) ) ) { //Send data to 'Receiver' every 60 seconds        
#ifdef DEBUG
        softSerial.println(F("Sending failed! :("));        
#endif
        } 
        

        if (_radio.available()) {        	
            NewSettingsPacket newSettingsData;
            _radio.read(&newSettingsData, sizeof(NewSettingsPacket));

            if (newSettingsData.forRadioId == _settings.radioId) {
#ifdef DEBUG        
        	    softSerial.println(F("Received new settings! Changing settings!"));
#endif         	
                processSettingsChange(newSettingsData);
            } else {
#ifdef DEBUG        
        	    softSerial.print(F("Ignoring settings change! Request for radioId: "));
        	    softSerial.println(newSettingsData.forRadioId);
#endif    	            
            }
        }   

    
#ifdef DEBUG        
        softSerial.print(F("Sleep for: "));    
        softSerial.println(_settings.sleepIntervalSeconds);    
#endif  

        _radio.powerDown();                  // Put the radio into a low power state.        
    }

    system_sleep();
    // sleep(_settings.sleepIntervalSeconds);
}





void processSettingsChange(NewSettingsPacket newSettings) {

    if (newSettings.changeType == ChangeRadioId) {
#ifdef DEBUG        
        softSerial.print(F("Changing radioId to: "));
        softSerial.println(newSettings.newRadioId, HEX);
#endif    	
        _settings.radioId = newSettings.newRadioId;
        setupRadio();
    } else if (newSettings.changeType == ChangeSleepInterval) {
#ifdef DEBUG        
        softSerial.print(F("Changing sleep interval to: "));
        softSerial.println(newSettings.newSleepIntervalSeconds);
#endif
        _settings.sleepIntervalSeconds = newSettings.newSleepIntervalSeconds;
    } else if (newSettings.changeType == ChangeMeasureType) {
        // sendMessage(F("Changing temperature"));
        // sendMessage(F(" correction"));
#ifdef DEBUG        
        softSerial.print(F("Changing measure mode to: "));
        softSerial.println(newSettings.newMeasureMode, HEX);
#endif    	        
        _settings.measureMode = newSettings.newMeasureMode;
    }

    saveSettings();
}


void saveSettings()
{
    EepromSettings settingsCurrentlyInEeprom;

    eepromBegin();
    eeprom_read_block(&settingsCurrentlyInEeprom, 0, sizeof(settingsCurrentlyInEeprom));
    eepromEnd();

    // Do not waste 1 of the 100,000 guaranteed eeprom writes if settings have not changed.
    if (memcmp(&settingsCurrentlyInEeprom, &_settings, sizeof(_settings)) == 0) {        
#ifdef DEBUG          
        softSerial.println(F("Skipped eeprom save, no change"));
#endif        
    } else {
#ifdef DEBUG        
        softSerial.println(F("Settings saved!"));
#endif  
        eepromBegin();
        eeprom_write_block(&_settings, 0, sizeof(_settings));
        eepromEnd();
    }
}


void setup_watchdog(int ii) {

    uint8_t bb;
    if (ii > 9 ) ii=9;
    bb=ii & 7;
    if (ii > 7) bb|= (1<<5);
    bb|= (1<<WDCE);


    MCUSR &= ~(1<<WDRF);
    // start timed sequence
    WDTCR |= (1<<WDCE) | (1<<WDE);
    // set new watchdog timeout value
    WDTCR = bb;
    WDTCR |= _BV(WDIE);
}

// Watchdog Interrupt Service / is executed when watchdog timed out
ISR(WDT_vect) {
    counter++;
    uint8_t times = _settings.sleepIntervalSeconds / 8000;

    if (counter >= times ) {
        f_wdt = 1;  // set global flag
        counter = 0;
    }    
}

// set system into the sleep state 
// system wakes up when wtchdog is timed out
void system_sleep() {    

    cbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter OFF

    set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
    sleep_enable();

    sleep_mode();                        // System sleeps here

    sleep_disable();                     // System continues execution here when watchdog timed out 
  
    sbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter ON
    
}