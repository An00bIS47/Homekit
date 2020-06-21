
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <SPI.h>

#define TINY_BME280_SPI
#include <TinyBME280.h>
#define SLEEP_TOTAL 1 // 113*8s = 904s ~15min


#define BME280_CS_PIN PIN_B3

// sleep cycles
volatile uint8_t sleep_count = 0;

// Global sensor object
tiny::BME280 sensor;
// Global LoRaWan object

#include <SoftwareSerial.h>
SoftwareSerial softSerial(99, PIN_B4); // RX, TX

void goToSleep();
void watchdogOn();

void setup()
{


    softSerial.begin(9600);
    delay(5000);
    softSerial.println(F("Starting ATTiny BME280 Test"));



    SPI.begin();
   
    // set slave select pins from RFM95 and BME280 as outputs
    pinMode(BME280_CS_PIN, OUTPUT);
  
    // set NSS_BME280 and NSS_RFM high
    digitalWrite(BME280_CS_PIN, HIGH);


    // init BME280
    sensor.beginSPI(BME280_CS_PIN); // Start using SPI and CS Pin 10

    sensor.setTempOverSample(1);
    sensor.setPressureOverSample(1);
    sensor.setHumidityOverSample(1);

    sensor.setMode(tiny::Mode::SLEEP);
    
    sensor.setFilter(0);


  
    // Turn on the watch dog timer
    watchdogOn();
  
  

}

void loop()
{
  //Disable ADC 
  ADCSRA &= ~(1<<ADEN); 
  
  // goToSleep();

  if (sleep_count >= SLEEP_TOTAL) { 

    sensor.setMode(tiny::Mode::FORCED); //Wake up sensor and take reading
    
    int32_t  temp = sensor.readFixedTempC();
    uint32_t hum = sensor.readFixedHumidity();
    uint32_t pressure = sensor.readFixedPressure();

    softSerial.print("temp: ");
    softSerial.println(temp);
    softSerial.print("hum: ");
    softSerial.println(hum);
    softSerial.print("pres: ");
    softSerial.println(pressure);


    // reset sleep count
    sleep_count = 0;
  }

  goToSleep();
}

void watchdogOn() {
  // clear various "reset" flags
  MCUSR = 0;
  // allow changes, disable reset
  WDTCR = (1 << WDCE) | (1 << WDE);
  // set interrupt mode and an interval 
  WDTCR = (1 << WDIE) | (1 << WDP3) | (1 << WDP0); // set WDIE, and 8 seconds delay
  wdt_reset();  // pat the dog
}

void goToSleep() {
  // Set sleep mode.
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  // Enter sleep mode.
  sleep_cpu();
  // After waking from watchdog interrupt the code continues from this point.
  
  // Disable sleep mode after waking.
  sleep_disable();
}

ISR(WDT_vect) {
  sleep_count++; // keep track of how many sleep cycles have been completed.
}