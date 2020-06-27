
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

#include <SoftwareSerial.h>
SoftwareSerial softSerial(99, PIN_B4); // RX, TX


// Utility macro
#define adc_enable()  (ADCSRA |= (1 << ADEN))
#define adc_disable() (ADCSRA &= ~(1 << ADEN)) // disable ADC (before power-off)


// *****************************************************************************************************************************
// 
// Structs
// 
// *****************************************************************************************************************************





// *****************************************************************************************************************************
// 
// Prototyps
// 
// *****************************************************************************************************************************

uint16_t readVcc();

void system_sleep();
void setup_watchdog(int ii);
void resetWatchDog();


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

    softSerial.begin(9600);
    delay(5000);
    softSerial.println(F("Starting ATTiny Sleep Test"));


    // Disable Analog Digital converter
    adc_disable();  // disable ADC

             
    setup_watchdog(9); // approximately 8 seconds sleep
}



// *****************************************************************************************************************************
// 
// Loop
// 
// *****************************************************************************************************************************

void loop() {    
    

    if ( counterWD == 2 ) {

        
        softSerial.println(readVcc());
        // Put the SPI pins to low for energy saving
        // SPI.end();
        

#ifdef DEBUG
        delay(1000);
#endif        

        counterWD = 0;
    }


    // deep sleep    
    system_sleep();
        
}



// *****************************************************************************************************************************
// 
// Watchdog
// 
// *****************************************************************************************************************************

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

void resetWatchDog(){
    MCUSR = 0;     // clear various "reset" flags
    WDTCR = bit (WDCE) | bit (WDE) | bit (WDIF);  // allow changes, disable reset, clear existing interrupt
    // set interrupt mode and an interval (WDE must be changed from 1 to 0 here)
    WDTCR = bit (WDIE) | bit (WDP3) | bit (WDP0);    // set WDIE, and 8 seconds delay
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






// *****************************************************************************************************************************
// 
// RF24
// 
// *****************************************************************************************************************************


