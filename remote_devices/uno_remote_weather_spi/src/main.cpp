
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
#include <SPI.h>

#include <avr/eeprom.h>
#include <avr/power.h>      // Power management
#include <avr/sleep.h>      // Sleep Modes
#include <avr/wdt.h>        // Watchdog timer

#include "RF24.h"

const char FIRMWARE_VERSION[6] = "1.0.5";

// #define DEBUG       
#define EEPROM_SETTINGS_VERSION 1
#define WATCHDOG_WAKEUPS_TARGET 7                


#ifndef DEVICE_ID
#define DEVICE_ID               0x12
#endif


#ifdef USE_BATTERY_CHECK_INTERVAL
    #define BAT_CHECK_INTERVAL      5        // after how many data collections we should get the battery status
    uint16_t batteryVoltage = 0;
    uint8_t batteryCheckCounter = BAT_CHECK_INTERVAL;
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
#define RF24_CE_PIN     9  // PB3
#define RF24_CSN_PIN    10  // Since we are using 3 pin configuration we will use same pin for both CE and CSN


// 
// EEPRom Macros
// 
#define eepromBegin() eeprom_busy_wait(); noInterrupts() // Details on https://youtu.be/_yOcKwu7mQA
#define eepromEnd()   interrupts()



// Utility macro
#define adc_enable()  (ADCSRA |= (1 << ADEN))
#define adc_disable() (ADCSRA &= ~(1 << ADEN)) // disable ADC (before power-off)


// *****************************************************************************************************************************
// 
// Structs
// 
// *****************************************************************************************************************************

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




// *****************************************************************************************************************************
// 
// Prototyps
// 
// *****************************************************************************************************************************

uint16_t readVcc();

void system_sleep();
void setup_watchdog(int ii);
void resetWatchDog();


bool setupRadio();

void processSettingsChange(NewSettingsPacket newSettings); 
void saveSettings();


// *****************************************************************************************************************************
// 
// Variables
// 
// *****************************************************************************************************************************

// uint8_t saveADCSRA;                 // variable to save the content of the ADC for later. if needed.
volatile uint8_t counterWD = 0;     // Count how many times WDog has fired. Used in the timing of the 
                                    // loop to increase the delay before the LED is illuminated. For example,
                                    // if WDog is set to 1 second TimeOut, and the counterWD loop to 10, the delay
                                    // between LED illuminations is increased to 1 x 10 = 10 seconds


RF24            _radio(RF24_CE_PIN, RF24_CSN_PIN);
uint8_t         address[RF24_ADDRESS_SIZE] = RF24_ADDRESS;
EepromSettings  _settings;



// *****************************************************************************************************************************
// 
// Read VCC
// 
// *****************************************************************************************************************************

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


// *****************************************************************************************************************************
// 
// Setup
// 
// *****************************************************************************************************************************
    
void setup() {

    Serial.begin(19200);

    Serial.println("Starting NRF24 Test...");
    // Disable Analog Digital converter
    adc_disable();  // disable ADC

    // Load settings from eeprom.
    eepromBegin();

#ifdef RESET_EEPROM    
    eeprom_write_block(0xFF, 0, sizeof(_settings));
#endif // RESET_EEPROM

    
    eeprom_read_block(&_settings, 0, sizeof(_settings));
    eepromEnd();

    if (_settings.version != EEPROM_SETTINGS_VERSION) {
        // The settings version in eeprom is not what we expect, so assign default values.
        _settings.radioId = RF24_ID;        
        _settings.sleepInterval = WATCHDOG_WAKEUPS_TARGET;        
        _settings.measureMode = MeasureModeWeatherStation;        
        _settings.version = EEPROM_SETTINGS_VERSION;
        strncpy(_settings.firmware_version, FIRMWARE_VERSION, 5);
        _settings.firmware_version[5] = '\0';

        saveSettings();
    }

    // set slave select pins BME280 as outputs
    // pinMode(BME280_CS_PIN, OUTPUT);
  
    // set BME280_CS_PIN 
    // digitalWrite(BME280_CS_PIN, HIGH);


    // init BME280
    // _bme280.beginSPI(BME280_CS_PIN); // Start using SPI and CS Pin 10

    // _bme280.setTempOverSample(1);
    // _bme280.setPressureOverSample(1);
    // _bme280.setHumidityOverSample(1);

    // _bme280.setMode(tiny::Mode::SLEEP);
    
    // _bme280.setFilter(0);
             
    // setup_watchdog(9); // approximately 8 seconds sleep
}



// *****************************************************************************************************************************
// 
// Loop
// 
// *****************************************************************************************************************************

void loop() {    
    

    if ( counterWD == _settings.sleepInterval ) {

        RadioPacket radioData;
        
        radioData.type = RemoteDeviceTypeWeather;
        radioData.radioId = _settings.radioId;
                
        // _bme280.setMode(tiny::Mode::FORCED); //Wake up sensor and take reading

        // radioData.temperature   = _bme280.readFixedTempC();
        // radioData.pressure      = _bme280.readFixedPressure();
        // radioData.humidity      = _bme280.readFixedHumidity();
        
        radioData.temperature   = 1100;
        radioData.pressure      = 12000;
        radioData.humidity      = 1300;

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
        

        Serial.print("Setup Radio ...");
        // Re-initialize the radio.               
        if (setupRadio()){
            Serial.println("OK");
            if (!_radio.write( &radioData, sizeof(RadioPacket) ) ) { //Send data to 'Receiver' every x seconds                        
                
            } else {
                
                if ( _radio.isAckPayloadAvailable() ) {
                    NewSettingsPacket newSettingsData;
                    _radio.read(&newSettingsData, sizeof(NewSettingsPacket));

                    if (newSettingsData.forRadioId == _settings.radioId) {
                        processSettingsChange(newSettingsData);        

                        // Send new updated settings
                        if (!_radio.write( &_settings, sizeof(EepromSettings) ) ) { //Send data to 'Receiver' every x seconds                                

                        }
                    }           
                }
            }
        } else {
            Serial.println("FAILED");
        }                         
    
        _radio.powerDown();                  // Put the radio into a low power state.        
        
        // Put the SPI pins to low for energy saving
        // SPI.end();
        

#ifdef DEBUG
        delay(1000);
#endif        

        counterWD = 0;
    }


    // deep sleep    
    // system_sleep();
    delay(1000);
    counterWD++;
        
}



// *****************************************************************************************************************************
// 
// Watchdog
// 
// *****************************************************************************************************************************

// 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
// 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec
void setup_watchdog(int ii) {

    /* Setup des Watchdog Timers */
    MCUSR &= ~(1<<WDRF);             /* WDT reset flag loeschen */
    WDTCSR |= (1<<WDCE) | (1<<WDE);  /* WDCE setzen, Zugriff auf Presclaler etc. */
    WDTCSR = 1<<WDP0 | 1<<WDP3;      /* Prescaler auf 8.0 s */
    WDTCSR |= 1<<WDIE;               /* WDT Interrupt freigeben */
}

void resetWatchDog(){
    MCUSR = 0;     // clear various "reset" flags
    // WDTCR = bit (WDCE) | bit (WDE) | bit (WDIF);  // allow changes, disable reset, clear existing interrupt
    // // set interrupt mode and an interval (WDE must be changed from 1 to 0 here)
    // WDTCR = bit (WDIE) | bit (WDP3) | bit (WDP0);    // set WDIE, and 8 seconds delay
    // pat the dog
    wdt_reset();  
}  // end of resetWatchdog



// *****************************************************************************************************************************
// 
// Interrupt
// 
// *****************************************************************************************************************************

ISR( WDT_vect ){    
    counterWD ++;                           // increase the WDog firing counter. Used in the loop to time the flash
                                            // interval of the LED. If you only want the WDog to fire within the normal 
                                            // presets, say 2 seconds, then comment out this command and also the associated
                                            // commands in the if ( counterWD..... ) loop, except the 2 digitalWrites and the
                                            // delay () commands.
} // end of ISR




// *****************************************************************************************************************************
// 
// Sleep
// 
// *****************************************************************************************************************************


// set system into the sleep state 
// system wakes up when wtchdog is timed out
void system_sleep() {    
        
    set_sleep_mode ( SLEEP_MODE_PWR_DOWN ); // set sleep mode Power Down

    power_all_disable ();                   // turn power off to ADC, TIMER 1 and 2, Serial Interface
    
    noInterrupts();                        // turn off interrupts as a precaution
    resetWatchDog();                       // reset the WatchDog before beddy bies
    sleep_enable();                        // allows the system to be commanded to sleep
    interrupts();                          // turn on interrupts
    
    sleep_cpu();                           // send the system to sleep, night night!

    sleep_disable();                       // after ISR fires, return to here and disable sleep
    power_all_enable();                    // turn on power to ADC, TIMER1 and 2, Serial Interface    
    
}




// *****************************************************************************************************************************
// 
// BME280
// 
// *****************************************************************************************************************************



// *****************************************************************************************************************************
// 
// Settings
// 
// *****************************************************************************************************************************

void processSettingsChange(NewSettingsPacket newSettings) {

    if (newSettings.changeType == ChangeTypeNone) {
        return;
    } else if (newSettings.changeType == ChangeRadioId) {
        _settings.radioId = newSettings.newRadioId;
        setupRadio();
    } else if (newSettings.changeType == ChangeSleepInterval) {
        _settings.sleepInterval = newSettings.newSleepInterval;
    } else if (newSettings.changeType == ChangeMeasureType) {       
        _settings.measureMode = newSettings.newMeasureMode;
      
        if (_settings.measureMode == MeasureModeWeatherStation){
            // _bme280.setTempOverSample(1);
            // _bme280.setPressureOverSample(1);
            // _bme280.setHumidityOverSample(1);
            // _bme280.setMode(tiny::Mode::SLEEP);
            // _bme280.setFilter(0);
        } else if (_settings.measureMode == MeasureModeIndoor){
            // _bme280.setTempOverSample(2);
            // _bme280.setPressureOverSample(16);
            // _bme280.setHumidityOverSample(1);
            // _bme280.setMode(tiny::Mode::NORMAL);
            // _bme280.setFilter(16);
            // _bme280.setStandbyTime(0);
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
 
    } else {

        eepromBegin();
        eeprom_write_block(&_settings, 0, sizeof(_settings));
        eepromEnd();
    }
}





// *****************************************************************************************************************************
// 
// RF24
// 
// *****************************************************************************************************************************


bool setupRadio(){

    if (_radio.begin() ){  

        _radio.setPALevel(RF24_PA_LEVEL);   // You can set it as minimum or maximum depending on the distance between the transmitter and receiver.
        _radio.setDataRate(RF24_DATA_RATE);
        // _radio.setAutoAck(1);            // Ensure autoACK is enabled
        _radio.enableAckPayload();
        _radio.setRetries(5,5);             // delay, count
                                            // 5 gives a 1500 µsec delay which is needed for a 32 byte ackPayload
        _radio.openWritingPipe(address);    // Write to device address 'HOMEKIT_RF24'

        return true;
    }  
    return false;
}
