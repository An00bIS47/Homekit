
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


#ifndef WDTCSR
#define WDTCSR WDTCR
#endif


// *****************************************************************************************************************************
// 
// Structs
// 
// *****************************************************************************************************************************



#define TEST_LED 1
#if TEST_LED
#define LED_PIN PIN_B3
#define SLEEP 1000
#endif



// *****************************************************************************************************************************
// 
// Prototyps
// 
// *****************************************************************************************************************************

uint16_t readVcc();

void enterSleep();
void setupWatchDogTimer();



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
    softSerial.println(F("Starting ATTiny Sleep Test"));

#if TEST_LED    
  	// initialize the digital pin as an output.
  	pinMode(LED_PIN, OUTPUT);
#endif


    setupWatchDogTimer();
}



// *****************************************************************************************************************************
// 
// Loop
// 
// *****************************************************************************************************************************

void loop() {    
    

    if ( counterWD >= 2 ) {

        softSerial.print("wake up - readVcc=");
        softSerial.println(readVcc());
        // Put the SPI pins to low for energy saving
        // SPI.end();            


		// Reset WD counter to 0
        counterWD = 0;

#if TEST_LED
		softSerial.print("Blink ");
        softSerial.println("ON");
		digitalWrite(LED_PIN, HIGH); 	// turn the LED on (HIGH is the voltage level)
  		delay(SLEEP);            		// wait for a second
		softSerial.print("Blink ");
        softSerial.println("OFF");
  		digitalWrite(LED_PIN, LOW);  	// turn the LED off by making the voltage LOW
  		delay(SLEEP);            		// wait for a second
#endif		  
    }


    // deep sleep    
    enterSleep();
        
}





// *****************************************************************************************************************************
// 
// Watchdog
// 
// *****************************************************************************************************************************

// Setup the Watch Dog Timer (WDT)
void setupWatchDogTimer() {
	// The MCU Status Register (MCUSR) is used to tell the cause of the last
	// reset, such as brown-out reset, watchdog reset, etc.
	// NOTE: for security reasons, there is a timed sequence for clearing the
	// WDE and changing the time-out configuration. If you don't use this
	// sequence properly, you'll get unexpected results.

	// Clear the reset flag on the MCUSR, the WDRF bit (bit 3).
	MCUSR &= ~(1<<WDRF);

	// Configure the Watchdog timer Control Register (WDTCSR)
	// The WDTCSR is used for configuring the time-out, mode of operation, etc

	// In order to change WDE or the pre-scaler, we need to set WDCE (This will
	// allow updates for 4 clock cycles).

	// Set the WDCE bit (bit 4) and the WDE bit (bit 3) of the WDTCSR. The WDCE
	// bit must be set in order to change WDE or the watchdog pre-scalers.
	// Setting the WDCE bit will allow updates to the pre-scalers and WDE for 4
	// clock cycles then it will be reset by hardware.
	WDTCSR |= (1<<WDCE) | (1<<WDE);

	/**
	 *	Setting the watchdog pre-scaler value with VCC = 5.0V and 16mHZ
	 *	WDP3 WDP2 WDP1 WDP0 | Number of WDT | Typical Time-out at Oscillator Cycles
	 *	0    0    0    0    |   2K cycles   | 16 ms
	 *	0    0    0    1    |   4K cycles   | 32 ms
	 *	0    0    1    0    |   8K cycles   | 64 ms
	 *	0    0    1    1    |  16K cycles   | 0.125 s
	 *	0    1    0    0    |  32K cycles   | 0.25 s
	 *	0    1    0    1    |  64K cycles   | 0.5 s
	 *	0    1    1    0    |  128K cycles  | 1.0 s
	 *	0    1    1    1    |  256K cycles  | 2.0 s
	 *	1    0    0    0    |  512K cycles  | 4.0 s
	 *	1    0    0    1    | 1024K cycles  | 8.0 s
	*/
	WDTCSR  = (1<<WDP3) | (0<<WDP2) | (0<<WDP1) | (1<<WDP0);
	// Enable the WD interrupt (note: no reset).
	WDTCSR |= _BV(WDIE);
}


// *****************************************************************************************************************************
// 
// Interrupt
// 
// *****************************************************************************************************************************
// Watchdog Interrupt Service. This is executed when watchdog timed out.
ISR( WDT_vect ){    
    counterWD++;                            // increase the WDog firing counter. Used in the loop to time the flash
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


// Puts the arduino into sleep mode.
void enterSleep(void)
{

    softSerial.println(F("Enter sleep mode"));

	// There are five different sleep modes in order of power saving:
	// SLEEP_MODE_IDLE - the lowest power saving mode
	// SLEEP_MODE_ADC
	// SLEEP_MODE_PWR_SAVE
	// SLEEP_MODE_STANDBY
	// SLEEP_MODE_PWR_DOWN - the highest power saving mode
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    power_all_disable ();                   // turn power off to ADC, TIMER 1 and 2, Serial Interface

	sleep_enable();

	// Now enter sleep mode.
	sleep_mode();

	// The program will continue from here after the WDT timeout

	// First thing to do is disable sleep.
	sleep_disable();

	// Re-enable the peripherals.
	power_all_enable();
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


