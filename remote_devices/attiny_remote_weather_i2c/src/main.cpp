
#include <Arduino.h>

#include <avr/eeprom.h>
#include <avr/power.h>      // Power management
#include <avr/sleep.h>      // Sleep Modes
#include <avr/wdt.h>        // Watchdog timer

#include "RF24.h"

// #define DEBUG
// #define DEBUG_RF24

#ifdef DEBUG_RF24
#ifndef DEBUG
#define DEBUG
#endif
#endif

#define DELAY_INTERVAL          48000
#define EEPROM_SETTINGS_VERSION 0

#ifndef DEVICE_ID
#define DEVICE_ID               0x01
#endif

#ifndef DEBUG_RF24
#define SDA_PORT PORTB
#define SDA_PIN 3
#define SCL_PORT PORTB
#define SCL_PIN 4
#include <SoftWire.h>
SoftWire sWire = SoftWire();

#define BME280_ADDRESS 0x76
#include "TinyBME280.h"
#endif // DEBUG_RF24

// Sleep macros
#ifndef cbi
    #define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
    #define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif


#ifndef RF24_ADDRESS
#define RF24_ADDRESS        "HOMEKIT_RF24"
#endif 

#define RF24_ADDRESS_SIZE   13

#ifndef RF24_PA_LEVEL
#define RF24_PA_LEVEL       RF24_PA_HIGH
#endif

#ifndef RF24_DATA_RATE
#define RF24_DATA_RATE      RF24_250KBPS
#endif

// RF24 Address
#ifndef RF24_ID
#define RF24_ID DEVICE_ID
#endif



// 
// Pins
// 
#define RF24_CE_PIN     PB2
#define RF24_CSN_PIN    PB2  // Since we are using 3 pin configuration we will use same pin for both CE and CSN
#define BME280_CS_PIN   PB3


// EEPRom Macros
#define eepromBegin() eeprom_busy_wait(); noInterrupts() // Details on https://youtu.be/_yOcKwu7mQA
#define eepromEnd()   interrupts()



struct EepromSettings
{
    uint16_t    radioId;
    uint32_t    sleepInterval;
    uint8_t     measureMode;
    uint8_t     version;
};

struct RadioPacket
{
    uint16_t    radioId;
    uint8_t     type;
    
    int32_t     temperature;    // temperature
    uint32_t    humidity;       // humidity
    uint32_t    pressure;       // pressure
    
    uint16_t    voltage;        // voltage * 100 , e.g. 330 for 3.3 V
};


enum RemoteDeviceType {
    RemoteDeviceTypeWeather    = 0x01,
    RemoteDeviceTypeDHT        = 0x02,
};


enum MeasureMode
{
    MeasureModeWeatherStation   = 0x00, // = measureWeather 0x01,
    MeasureModeIndoor           = 0x01, // = measureIndoor  0x11,
};


enum ChangeType
{
    ChangeTypeNone              = 0x00,
    ChangeRadioId               = 0x01,
    ChangeSleepInterval         = 0x02,
    ChangeMeasureType           = 0x03,
};


struct NewSettingsPacket
{
    uint8_t         changeType; // enum ChangeType
    uint16_t        forRadioId;
    uint16_t        newRadioId;
    uint32_t        newSleepInterval;
    uint8_t         newMeasureMode;
};



#ifdef DEBUG
    #include <SoftwareSerial.h>
    SoftwareSerial softSerial(99, PB4); // RX, TX
#endif // DEBUG


uint16_t readVcc();
void processSettingsChange(NewSettingsPacket newSettings); // Need to pre-declare this function since it uses a custom struct as a parameter (or use a .h file instead).
void setupRadio();
void saveSettings();
void system_sleep();
void setup_watchdog(int ii);



RF24            _radio(RF24_CE_PIN, RF24_CSN_PIN);
uint8_t         address[RF24_ADDRESS_SIZE] = RF24_ADDRESS;

EepromSettings  _settings;


volatile boolean f_wdt = 1;
volatile uint8_t counter = 0;

uint16_t readVcc()
{
    //power_adc_enable() ;
    // Details on http://provideyourown.com/2012/secret-arduino-voltmeter-measure-battery-voltage/

    ADMUX = _BV(MUX3) | _BV(MUX2); // Select internal 1.1V reference for measurement.
    delay(2);                      // Let voltage stabilize.
    ADCSRA |= _BV(ADSC);           // Start measuring.
    while (ADCSRA & _BV(ADSC));    // Wait for measurement to complete.
    uint16_t adcReading = ADC;
    uint16_t vcc = (1.1 * 1024.0 / adcReading) * 100; // Note the 1.1V reference can be off +/- 10%, so calibration is needed.

    //power_adc_disable();
    return vcc;
}

    
void setup() {

#ifdef DEBUG    
    // put your setup code here, to run once:
    softSerial.begin(9600);
    delay(5000);
    softSerial.println(F("Starting ATTiny RF24 Test"));
#endif // DEBUF


    // Load settings from eeprom.
    eepromBegin();

#ifdef RESET_EEPROM
    
    eeprom_write_block(0xFF, 0, sizeof(_settings));
#endif // RESET_EEPROM

    
    eeprom_read_block(&_settings, 0, sizeof(_settings));
    eepromEnd();

    if (_settings.version == EEPROM_SETTINGS_VERSION) {
#ifdef DEBUG
        softSerial.println(F("OK"));
#endif // DEBUG        
    } else {
        // The settings version in eeprom is not what we expect, so assign default values.
#ifdef DEBUG        
        softSerial.println(F("ERROR -> Assigning defaults"));
#endif // DEBUG

        _settings.radioId = RF24_ID;        
        _settings.sleepInterval = DELAY_INTERVAL;        
        _settings.measureMode = MeasureModeWeatherStation;        
        _settings.version = EEPROM_SETTINGS_VERSION;
        saveSettings();
    }

#ifndef DEBUG_RF24    
    sWire.begin();
    delay(500);

    BME280setup(sWire, BME280_ADDRESS);
    

#endif // DEBUG_RF24
    setup_watchdog(9); // approximately 8 seconds sleep
}

void loop() {

    if (f_wdt==1) { 
        f_wdt=0; 

        setupRadio();                        // Re-initialize the radio.
        
        RadioPacket radioData;
      
        radioData.type = RemoteDeviceTypeWeather;
        radioData.radioId = _settings.radioId;
        

#ifdef DEBUG                            
        softSerial.print(F("Reading BME280 ..."));
#endif // DEBUG


#ifndef DEBUG_RF24    
        radioData.temperature   = BME280temperature(sWire);;
        radioData.pressure      = BME280pressure(sWire);
        radioData.humidity      = BME280humidity(sWire);
#else
        radioData.temperature   = 1000;
        radioData.humidity      = 1100;
        radioData.pressure      = 12000;
#endif // DEBUG_RF24


#ifdef DEBUG                            
        softSerial.println(F("OK"));
#endif // DEBUG
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
#endif // DEBUG

        if (!_radio.write( &radioData, sizeof(RadioPacket) ) ) { //Send data to 'Receiver' every x seconds                        
#ifdef DEBUG
            softSerial.println(F("Sending failed! :("));        
#endif // DEBUG
        } else {
            if ( _radio.isAckPayloadAvailable() ) {
                NewSettingsPacket newSettingsData;
                _radio.read(&newSettingsData, sizeof(NewSettingsPacket));

#ifdef DEBUG
                softSerial.print(F("Received new settings for: "));
                softSerial.println(newSettingsData.forRadioId, HEX);
                softSerial.print(F("   changeType: "));
                softSerial.println(newSettingsData.changeType, HEX);

                softSerial.print(F("   newRadioId: "));
                softSerial.println(newSettingsData.newRadioId, HEX);

                softSerial.print(F("   newSleepInterval: "));
                softSerial.println(newSettingsData.newSleepInterval);

                softSerial.print(F("   newMeasureMode: "));
                softSerial.println(newSettingsData.newMeasureMode, HEX);

                softSerial.print(F("   sizeOf NewSettingsPacket: "));
                softSerial.println(sizeof(NewSettingsPacket));
                
#endif // DEBUG

                if (newSettingsData.forRadioId == _settings.radioId) {
#ifdef DEBUG 
                    softSerial.println(F("Received new settings. Changing settings!"));
#endif // DEBUG                  
                    processSettingsChange(newSettingsData);        

#ifdef DEBUG 
                    softSerial.print(F("Sending updated settings ..."));
#endif // DEBUG
                    // Send new updated settings
                    if (!_radio.write( &_settings, sizeof(EepromSettings) ) ) { //Send data to 'Receiver' every x seconds                                
#ifdef DEBUG 
                        softSerial.println(F("Failed to send settings :("));
#endif // DEBUG

#ifdef DEBUG 
                    } else {
                        softSerial.println(F("Failed to send updated settings :("));
#endif // DEBUG                     
                    }
#ifdef DEBUG    
                } else {             
                    softSerial.print(F("Ignoring settings change! Request for radioId: "));
                    softSerial.println(newSettingsData.forRadioId);
#endif // DEBUG
                }

#ifdef DEBUG                   
            } else {              
                softSerial.println(F("Acknowledge but no data "));
#endif // DEBUG            
            }
        }
  

    
#ifdef DEBUG        
        softSerial.print(F("Sleep for: "));    
        softSerial.println(_settings.sleepInterval);    
#endif // DEBUG

        _radio.powerDown();                  // Put the radio into a low power state.        
    }

    // delay(5000);
    system_sleep();
    
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
    uint8_t times = _settings.sleepInterval / 8000;

    if (counter >= times ) {
        f_wdt = 1;  // set global flag
        counter = 0;
    }    
}

// set system into the sleep state 
// system wakes up when wtchdog is timed out
void system_sleep() {    

    cbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter OFF


    // mcucr1 = MCUCR | _BV(BODS) | _BV(BODSE);  //turn off the brown-out detector
    // mcucr2 = mcucr1 & ~_BV(BODSE);
    // MCUCR = mcucr1;
    // MCUCR = mcucr2;

    // sleep_bod_disable();
    set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
    sleep_enable();

    sleep_mode();                        // System sleeps here

    sleep_disable();                     // System continues execution here when watchdog timed out 
    

    sbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter ON
    
}





void setupRadio(){

#ifdef DEBUG        
        softSerial.print(F("Setup RF24 ..."));    
#endif // DEBUG

    if (_radio.begin() ){  
#ifdef DEBUG                            // Start up the radio            
        softSerial.println(F("OK - RF24 wiring OK!"));
#endif // DEBUG 


        _radio.setPALevel(RF24_PA_LEVEL);   // You can set it as minimum or maximum depending on the distance between the transmitter and receiver.
        _radio.setDataRate(RF24_DATA_RATE);
        // _radio.setAutoAck(1);            // Ensure autoACK is enabled
        _radio.enableAckPayload();
        _radio.setRetries(5,5);             // delay, count
                                            // 5 gives a 1500 µsec delay which is needed for a 32 byte ackPayload
        _radio.openWritingPipe(address);    // Write to device address 'HOMEKIT_RF24'

// #ifdef DEBUG  
//         _radio.printDetails();
// #endif

#ifdef DEBUG                                
    } else {
        softSerial.println(F("ERROR: RF24 not found! Please check wiring!")); 
#endif // DEBUG          
    }  
}



void processSettingsChange(NewSettingsPacket newSettings) {

    if (newSettings.changeType == ChangeTypeNone) {
#ifdef DEBUG        
        softSerial.print(F("Changing type None!"));        
#endif // DEBUG 
        return;
    } else if (newSettings.changeType == ChangeRadioId) {
#ifdef DEBUG        
        softSerial.print(F("Changing radioId to: "));
        softSerial.println(newSettings.newRadioId, HEX);
#endif // DEBUG 
        _settings.radioId = newSettings.newRadioId;
        setupRadio();
    } else if (newSettings.changeType == ChangeSleepInterval) {
#ifdef DEBUG        
        softSerial.print(F("Changing sleep interval to: "));
        softSerial.println(newSettings.newSleepInterval);
#endif // DEBUG 
        _settings.sleepInterval = newSettings.newSleepInterval;
    } else if (newSettings.changeType == ChangeMeasureType) {
        // sendMessage(F("Changing temperature"));
        // sendMessage(F(" correction"));
#ifdef DEBUG        
        softSerial.print(F("Changing measure mode to: "));
        softSerial.println(newSettings.newMeasureMode, HEX);
#endif // DEBUG           
        _settings.measureMode = newSettings.newMeasureMode;
        if (_settings.measureMode == MeasureModeWeatherStation){
            BME280setSampling(sWire, MODE_FORCED,
						SAMPLING_X1,    // temperature
						SAMPLING_X1,    // pressure
						SAMPLING_X1,    // humidity
						FILTER_OFF);
        } else if (_settings.measureMode == MeasureModeIndoor){
            BME280setSampling(sWire, MODE_NORMAL,
                    SAMPLING_X2,  // temperature
                    SAMPLING_X16, // pressure
                    SAMPLING_X1,  // humidity
                    FILTER_X16,
                    STANDBY_MS_0_5);
        }
        
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
#endif // DEBUG  
    } else {
#ifdef DEBUG        
        softSerial.println(F("Settings saved!"));
#endif // DEBUG 
        eepromBegin();
        eeprom_write_block(&_settings, 0, sizeof(_settings));
        eepromEnd();
    }
}
