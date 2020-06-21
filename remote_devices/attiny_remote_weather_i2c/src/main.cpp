
// 
// Platformio is using SpenceKonde's ATTinyCore
// https://github.com/SpenceKonde/ATTinyCore
// 
// 
// ATMEL ATTINY 25/45/85 / ARDUINO
//
//                   +-\/-+
//  Ain0 (D 5) PB5  1|    |8  Vcc
//  Ain3 (D 3) PB3  2|    |7  PB2 (D 2) Ain1
//  Ain2 (D 4) PB4  3|    |6  PB1 (D 1) pwm1
//             GND  4|    |5  PB0 (D 0) pwm0
//                   +----+
// 
// 
// Power Saving Tips
// 
// - add a capacitor of 100-200µF in parallel with your battery. 
//   Ceramic is better, but I have no problem with nodes using electrolytic capacitors. 
//   This will help when there is a peak power consumption from the radio. 
//   If you do not put one, voltage will drop quickly and that's probably what is triggering reboot loop 
//   on one of your nodes: maybe radio is less efficient and needs to resend more messages. 
// 
// - I've noticed that power usage on the nRF modules shoots up if I leave any inputs (CSN, SCK, MOSI) floating.  
//   With my circuit above CSN will never float, but SCK and MOSI should be pulled high or low when not in use, 
//   especially for battery-powered devices.
// 
// - For low power applications, before entering sleep, remember to turn off the ADC 
//   (ADCSRA&=(~(1<<ADEN))) - otherwise it will waste ~270uA
// 
// Gotchas:
// 
// - You cannot use the Pxn notation (ie, PB2, PA1, etc) to refer to pins 
//   these are defined by the compiler-supplied headers, and not to what an arduino user would expect. 
//   To refer to pins by port and bit, use PIN_xn (ex, PIN_B2); these are #defined to the Arduino pin number 
//   for the pin in question, and can be used wherever digital pin numbers can be used
// 
// - All ATTiny chips (as well as the vast majority of digital integrated circuits) require a 0.1uF ceramic capacitor 
//   between Vcc and Gnd for decoupling; this should be located as close to the chip as possible 
//   (minimize length of wires to cap). Devices with multiple Vcc pins, or an AVcc pin, 
//   should use a cap on those pins too. Do not be fooled by poorly written tutorials or guides that omit these. 
//   Yes, I know that in some cases (ex, the x5 series) the datasheet doesn't mention these 
//   but other users as well as myself have had problems when it was omitted on a t85.
// 
// - When using I2C on anything other than the ATTiny48/88 you must use an I2C pullup resistor 
//   on SCL and SDA (if there isn't already one on the I2C device you're working with 
//   many breakout boards include them). 4.7k or 10k is a good default value. 
//   On parts with real hardware I2C, the internal pullups are used, and this is sometimes good enough 
//   to work without external pullups; this is not the case for devices without hardware I2C 
//   (all devices supported by this core except 48/88) - the internal pullups can't be used here, 
//   so you must use external ones. That said, for maximum reliability, you should always use external pullups,
//   even on the t48/88, as the internal pullups are not as strong as the specification requires.
// 

#include <Arduino.h>

#include <avr/eeprom.h>
#include <avr/power.h>      // Power management
#include <avr/sleep.h>      // Sleep Modes
#include <avr/wdt.h>        // Watchdog timer

#include "RF24.h"

const char FIRMWARE_VERSION[6] = "1.0.3";


// #define DEBUG                        
#define DEBUG_RF24                   // uncomment to debug RF24 - BME280 will be disabled
#define RESET_EEPROM                    // uncomment to reset EEPRom at startup
// #define USE_BATTERY_CHECK_INTERVAL   // uncomment to check battery interval only every X cycles
#define USE_QUICK_TIMER              // uncomment for 8 sec sleep


#ifndef DEVICE_ID
#define DEVICE_ID               0x01
#endif


#ifdef DEBUG_RF24
#ifndef DEBUG
#define DEBUG
#endif
#endif

// time until the watchdog wakes the mc in seconds
#define WATCHDOG_TIME           8   // 1, 2, 4 or 8
 
// after how many watchdog wakeups we should collect and send the data
#ifdef DEBUG_RF24
    #define WATCHDOG_WAKEUPS_TARGET 1   // 8 * 1 =  8 seconds between each data collection
    #define EEPROM_SETTINGS_VERSION 0
#else
    #ifdef USE_QUICK_TIMER
        #define WATCHDOG_WAKEUPS_TARGET 1   // 8 * 6 = 48 seconds between each data collection
        #define EEPROM_SETTINGS_VERSION 2
    #else
        #define WATCHDOG_WAKEUPS_TARGET 6   // 8 * 6 = 48 seconds between each data collection
        #define EEPROM_SETTINGS_VERSION 1
    #endif
#endif


#ifdef USE_BATTERY_CHECK_INTERVAL
    #define BAT_CHECK_INTERVAL      5        // after how many data collections we should get the battery status
    uint16_t batteryVoltage = 0;
    uint8_t batteryCheckCounter = BAT_CHECK_INTERVAL;
#endif


// 
// BME280
// 
#ifndef DEBUG_RF24
    #define SDA_PORT PORTB
    #define SDA_PIN 3
    #define SCL_PORT PORTB
    #define SCL_PIN 4

    #include <SoftWire.h>
    SoftWire sWire = SoftWire();

    #ifndef BME280_ADDRESS
    #define BME280_ADDRESS 0x76
    #endif

    #include "TinyBME280.h"
#endif // DEBUG_RF24

// 
// Sleep macros
// 
#ifndef cbi
    #define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
    #define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif


// 
// RF24
// 
#ifndef RF24_ADDRESS
    #define RF24_ADDRESS        "HOMEKIT_RF24"
#endif 

#ifndef RF24_ADDRESS_SIZE
    #define RF24_ADDRESS_SIZE   13
#endif

#ifndef RF24_PA_LEVEL
    #define RF24_PA_LEVEL       RF24_PA_MIN
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
#define RF24_CE_PIN     PIN_B2  // PB2 
#define RF24_CSN_PIN    PIN_B2  // Since we are using 3 pin configuration we will use same pin for both CE and CSN


// 
// EEPRom Macros
// 
#define eepromBegin() eeprom_busy_wait(); noInterrupts() // Details on https://youtu.be/_yOcKwu7mQA
#define eepromEnd()   interrupts()

// Utility macro
#define adc_enable()  (ADCSRA |= (1 << ADEN))
#define adc_disable() (ADCSRA &= ~(1 << ADEN)) // disable ADC (before power-off)


struct EepromSettings
{
    uint16_t    radioId;
    uint8_t     sleepInterval;
    uint8_t     measureMode;
    uint8_t     version;
    char        firmware_version[6];
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
    uint8_t         newSleepInterval;
    uint8_t         newMeasureMode;
};


// 
// DEBUG softSerial
// 
#ifdef DEBUG
    #include <SoftwareSerial.h>
    SoftwareSerial softSerial(99, PIN_B4); // RX, TX
#endif // DEBUG


uint16_t readVcc();
void processSettingsChange(NewSettingsPacket newSettings); // Need to pre-declare this function since it uses a custom struct as a parameter (or use a .h file instead).
void setupRadio();

#ifndef DEBUG_RF24   
void setupBME280();
void powerDownBME280();
#endif

void saveSettings();
void system_sleep();
void setup_watchdog(int ii);
void resetWatchDog();

uint8_t saveADCSRA;                 // variable to save the content of the ADC for later. if needed.
volatile uint8_t counterWD = 0;     // Count how many times WDog has fired. Used in the timing of the 
                                    // loop to increase the delay before the LED is illuminated. For example,
                                    // if WDog is set to 1 second TimeOut, and the counterWD loop to 10, the delay
                                    // between LED illuminations is increased to 1 x 10 = 10 seconds


RF24            _radio(RF24_CE_PIN, RF24_CSN_PIN);
uint8_t         address[RF24_ADDRESS_SIZE] = RF24_ADDRESS;
EepromSettings  _settings;


uint16_t readVcc()
{
    adc_enable();  // enable ADC
    // Details on http://provideyourown.com/2012/secret-arduino-voltmeter-measure-battery-voltage/

    ADMUX = _BV(MUX3) | _BV(MUX2); // Select internal 1.1V reference for measurement.
    delay(2);                      // Let voltage stabilize.
    ADCSRA |= _BV(ADSC);           // Start measuring.
    while (ADCSRA & _BV(ADSC));    // Wait for measurement to complete.
    uint16_t adcReading = ADC;
    uint16_t vcc = (1.1 * 1024.0 / adcReading) * 100; // Note the 1.1V reference can be off +/- 10%, so calibration is needed.

    adc_disable();  // disable ADC
    return vcc;
}

    
void setup() {

#ifdef DEBUG    
    // put your setup code here, to run once:
    softSerial.begin(9600);
    delay(5000);
    softSerial.println(F("Starting ATTiny RF24 Test"));
#endif // DEBUF

    // Disable Analog Digital converter
    adc_disable();  // disable ADC

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
        _settings.sleepInterval = WATCHDOG_WAKEUPS_TARGET;        
        _settings.measureMode = MeasureModeWeatherStation;        
        _settings.version = EEPROM_SETTINGS_VERSION;
        strncpy(_settings.firmware_version, FIRMWARE_VERSION, 5);
        _settings.firmware_version[5] = '\0';

        saveSettings();
    }

#ifndef DEBUG_RF24   

    // setupBME280();
    setupBME280();

#endif // DEBUG_RF24
    setup_watchdog(9); // approximately 8 seconds sleep
}



void loop() {    
    
    resetWatchDog();  // do this first in case WDT fires

    if ( counterWD == _settings.sleepInterval ) {
        counterWD = 0;

        RadioPacket radioData;
        
        radioData.type = RemoteDeviceTypeWeather;
        radioData.radioId = _settings.radioId;
        

    #ifdef DEBUG                            
        softSerial.print(F("Reading BME280 ..."));
    #endif // DEBUG


    #ifndef DEBUG_RF24    

        // setupBME280();

        radioData.temperature   = BME280temperature(sWire);;
        radioData.pressure      = BME280pressure(sWire);
        radioData.humidity      = BME280humidity(sWire);

        // powerDownBME280();

    #else
        radioData.temperature   = 1000;
        radioData.humidity      = 1100;
        radioData.pressure      = 12000;
    #endif // DEBUG_RF24


    #ifdef DEBUG                            
        softSerial.println(F("OK"));
    #endif // DEBUG


    #ifdef USE_BATTERY_CHECK_INTERVAL
        batteryCheckCounter++;

        if (batteryCheckCounter >= 5) {
            batteryVoltage = readVcc();
            batteryCheckCounter = 0;
        }
        
        radioData.voltage = batteryVoltage;
    #else
        radioData.voltage = readVcc();
    #endif


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

        SPI.begin();    
        setupRadio();                        // Re-initialize the radio.    
        
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
    
        _radio.powerDown();                  // Put the radio into a low power state.        
        
        // Put the SPI pins to low for energy saving
        SPI.end();
        digitalWrite(SCK, LOW);
        digitalWrite(MOSI, LOW);

        
    #ifdef DEBUG        
        softSerial.print(F("Sleep for: "));    
        softSerial.println(_settings.sleepInterval);    
    #endif // DEBUG  
    }

  

    // deep sleep    
    system_sleep();
        
}

// 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
// 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec
void setup_watchdog(int ii) {

    uint8_t bb;
    // int ww;
    if (ii > 9 ) ii=9;
    bb=ii & 7;
    if (ii > 7) bb|= (1<<5);
    bb|= (1<<WDCE);
    // ww=bb;

    MCUSR &= ~(1<<WDRF);
    // start timed sequence
    WDTCR |= (1<<WDCE) | (1<<WDE);
    // set new watchdog timeout value
    WDTCR = bb;
    WDTCR |= _BV(WDIE);
}

// void setup_watchdog() {

//     cli();
  
//     // clear the reset flag
//     MCUSR &= ~(1<<WDRF);
    
//     // set WDCE to be able to change/set WDE
//     WDTCR |= (1<<WDCE) | (1<<WDE);
    
//     // set new watchdog timeout prescaler value
// #if WATCHDOG_TIME == 1
//     WDTCR = 1<<WDP1 | 1<<WDP2;
// #elif WATCHDOG_TIME == 2
//     WDTCR = 1<<WDP0 | 1<<WDP1 | 1<<WDP2;
// #elif WATCHDOG_TIME == 4
//     WDTCR = 1<<WDP3;
// #elif WATCHDOG_TIME == 8
//     WDTCR = 1<<WDP0 | 1<<WDP3;
// #else
//     #error WATCHDOG_TIME must be 1, 2, 4 or 8!
// #endif
    
//     // enable the WD interrupt to get an interrupt instead of a reset
//     WDTCR |= (1<<WDIE);
    
//     sei();
// }



ISR( WDT_vect ){    
    counterWD ++;                           // increase the WDog firing counter. Used in the loop to time the flash
                                            // interval of the LED. If you only want the WDog to fire within the normal 
                                            // presets, say 2 seconds, then comment out this command and also the associated
                                            // commands in the if ( counterWD..... ) loop, except the 2 digitalWrites and the
                                            // delay () commands.
} // end of ISR

// set system into the sleep state 
// system wakes up when wtchdog is timed out
void system_sleep() {    
    
    set_sleep_mode ( SLEEP_MODE_PWR_DOWN ); // set sleep mode Power Down
    saveADCSRA = ADCSRA;                    // save the state of the ADC. We can either restore it or leave it turned off.
    ADCSRA = 0;                             // turn off the ADC
    power_all_disable ();                   // turn power off to ADC, TIMER 1 and 2, Serial Interface
    
    noInterrupts ();                        // turn off interrupts as a precaution
    resetWatchDog ();                       // reset the WatchDog before beddy bies
    sleep_enable ();                        // allows the system to be commanded to sleep
    interrupts ();                          // turn on interrupts
    
    sleep_cpu ();                           // send the system to sleep, night night!

    sleep_disable ();                       // after ISR fires, return to here and disable sleep
    power_all_enable ();                    // turn on power to ADC, TIMER1 and 2, Serial Interface
    
    ADCSRA = saveADCSRA;                 // turn on and restore the ADC if needed. Commented out, not needed.

}

void resetWatchDog(){
    MCUSR = 0;     // clear various "reset" flags
    WDTCR = bit (WDCE) | bit (WDE) | bit (WDIF);  // allow changes, disable reset, clear existing interrupt
    // set interrupt mode and an interval (WDE must be changed from 1 to 0 here)
    WDTCR = bit (WDIE) | bit (WDP3) | bit (WDP0);    // set WDIE, and 8 seconds delay
    // pat the dog
    wdt_reset();  
}  // end of resetWatchdog



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

#ifdef DEBUG                                
    } else {
        softSerial.println(F("ERROR: RF24 not found! Please check wiring!")); 
#endif // DEBUG          
    }  
}


#ifndef DEBUG_RF24    
void setupBME280(){
    sWire.begin();
    delay(10);

    if (BME280setup(sWire, BME280_ADDRESS) == false){
#ifdef DEBUG         
        softSerial.println(F("ERROR -> BME280 not found :("));
#endif // DEBUG        
    } 
}

void powerDownBME280(){
    // sWire.end(); // does nothing
    BME280powerDown(sWire);
    // digitalWrite(SDA, LOW);
    // digitalWrite(SCL, LOW);
}
#endif

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

#ifndef DEBUG_RF24        
        if (_settings.measureMode == MeasureModeWeatherStation){
            BME280setSampling(sWire, MODE_FORCED,
						SAMPLING_X1,    // temperature
						SAMPLING_X1,    // pressure
						SAMPLING_X1,    // humidity
						FILTER_OFF);
        } else if (_settings.measureMode == MeasureModeIndoor){
            BME280setSampling(sWire, MODE_NORMAL,
                        SAMPLING_X2,    // temperature
                        SAMPLING_X16,   // pressure
                        SAMPLING_X1,    // humidity
                        FILTER_X16,
                        STANDBY_MS_0_5);
        }
#endif        
        
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
