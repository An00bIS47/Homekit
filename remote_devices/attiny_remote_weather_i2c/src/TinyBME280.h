/* TinyBME280 Library v2
   David Johnson-Davies - www.technoblogy.com - 22nd June 2019
   
   CC BY 4.0
   Licensed under a Creative Commons Attribution 4.0 International license: 
   http://creativecommons.org/licenses/by/4.0/

   + Modified for Softwire and setSamplingModes
*/

#ifndef TINYBME280
#define TINYBME280

// #define BME280_REGISTER_DIG_T1 0x88
// #define BME280_REGISTER_DIG_T2 0x8A
// #define BME280_REGISTER_DIG_T3 0x8C

// #define BME280_REGISTER_DIG_P1 0x8E
// #define BME280_REGISTER_DIG_P2 0x90
// #define BME280_REGISTER_DIG_P3 0x92
// #define BME280_REGISTER_DIG_P4 0x94
// #define BME280_REGISTER_DIG_P5 0x96
// #define BME280_REGISTER_DIG_P6 0x98
// #define BME280_REGISTER_DIG_P7 0x9A
// #define BME280_REGISTER_DIG_P8 0x9C
// #define BME280_REGISTER_DIG_P9 0x9E

// #define BME280_REGISTER_DIG_H1 0xA1
// #define BME280_REGISTER_DIG_H2 0xE1
// #define BME280_REGISTER_DIG_H3 0xE3
// #define BME280_REGISTER_DIG_H4 0xE4
// #define BME280_REGISTER_DIG_H5 0xE5
// #define BME280_REGISTER_DIG_H6 0xE7

#define BME280_REGISTER_CHIPID        0xD0
#define BME280_REGISTER_VERSION       0xD1
#define BME280_REGISTER_SOFTRESET     0xE0

#define BME280_REGISTER_CAL26         0xE1 // R calibration stored in 0xE1-0xF0

#define BME280_REGISTER_CONTROLHUMID  0xF2
#define BME280_REGISTER_STATUS        0XF3
#define BME280_REGISTER_CONTROL       0xF4
#define BME280_REGISTER_CONFIG        0xF5
#define BME280_REGISTER_PRESSUREDATA  0xF7
#define BME280_REGISTER_TEMPDATA      0xFA
#define BME280_REGISTER_HUMIDDATA     0xFD


  /**************************************************************************/
  /*!
      @brief  sampling rates
  */
  /**************************************************************************/
  enum sensor_sampling {
    SAMPLING_NONE = 0b000,
    SAMPLING_X1 = 0b001,
    SAMPLING_X2 = 0b010,
    SAMPLING_X4 = 0b011,
    SAMPLING_X8 = 0b100,
    SAMPLING_X16 = 0b101
  };

  /**************************************************************************/
  /*!
      @brief  power modes
  */
  /**************************************************************************/
  enum sensor_mode {
    MODE_SLEEP = 0b00,
    MODE_FORCED = 0b01,
    MODE_NORMAL = 0b11
  };

  /**************************************************************************/
  /*!
      @brief  filter values
  */
  /**************************************************************************/
  enum sensor_filter {
    FILTER_OFF = 0b000,
    FILTER_X2 = 0b001,
    FILTER_X4 = 0b010,
    FILTER_X8 = 0b011,
    FILTER_X16 = 0b100
  };

  /**************************************************************************/
  /*!
      @brief  standby duration in ms
  */
  /**************************************************************************/
  enum standby_duration {
    STANDBY_MS_0_5 = 0b000,
    STANDBY_MS_10 = 0b110,
    STANDBY_MS_20 = 0b111,
    STANDBY_MS_62_5 = 0b001,
    STANDBY_MS_125 = 0b010,
    STANDBY_MS_250 = 0b011,
    STANDBY_MS_500 = 0b100,
    STANDBY_MS_1000 = 0b101
  };


  /**************************************************************************/
  /*!
      @brief  config register
  */
  /**************************************************************************/
  struct config {
    // inactive duration (standby time) in normal mode
    // 000 = 0.5 ms
    // 001 = 62.5 ms
    // 010 = 125 ms
    // 011 = 250 ms
    // 100 = 500 ms
    // 101 = 1000 ms
    // 110 = 10 ms
    // 111 = 20 ms
    unsigned int t_sb : 3; ///< inactive duration (standby time) in normal mode

    // filter settings
    // 000 = filter off
    // 001 = 2x filter
    // 010 = 4x filter
    // 011 = 8x filter
    // 100 and above = 16x filter
    unsigned int filter : 3; ///< filter settings

    // unused - don't set
    unsigned int none : 1;     ///< unused - don't set
    unsigned int spi3w_en : 1; ///< unused - don't set

    /// @return combined config register
    unsigned int get() { return (t_sb << 5) | (filter << 2) | spi3w_en; }
  };
  config _configReg; //!< config register object

  /**************************************************************************/
  /*!
      @brief  ctrl_meas register
  */
  /**************************************************************************/
  struct ctrl_meas {
    // temperature oversampling
    // 000 = skipped
    // 001 = x1
    // 010 = x2
    // 011 = x4
    // 100 = x8
    // 101 and above = x16
    unsigned int osrs_t : 3; ///< temperature oversampling

    // pressure oversampling
    // 000 = skipped
    // 001 = x1
    // 010 = x2
    // 011 = x4
    // 100 = x8
    // 101 and above = x16
    unsigned int osrs_p : 3; ///< pressure oversampling

    // device mode
    // 00       = sleep
    // 01 or 10 = forced
    // 11       = normal
    unsigned int mode : 2; ///< device mode

    /// @return combined ctrl register
    unsigned int get() { return (osrs_t << 5) | (osrs_p << 2) | mode; }
  };
  ctrl_meas _measReg; //!< measurement register object

  /**************************************************************************/
  /*!
      @brief  ctrl_hum register
  */
  /**************************************************************************/
  struct ctrl_hum {
    /// unused - don't set
    unsigned int none : 5;

    // pressure oversampling
    // 000 = skipped
    // 001 = x1
    // 010 = x2
    // 011 = x4
    // 100 = x8
    // 101 and above = x16
    unsigned int osrs_h : 3; ///< pressure oversampling

    /// @return combined ctrl hum register
    unsigned int get() { return (osrs_h); }
  };
  ctrl_hum _humReg; //!< hum register object


int16_t T[4], P[10], H[7];
int32_t BME280t_fine;

int BME280address = 0x76;

int BME280sensorID = 280;


int32_t BME280temperature (SoftWire &wire);
uint32_t BME280pressure (SoftWire &wire);
uint32_t BME280humidity (SoftWire &wire);


int16_t read16(SoftWire &wire) {
	uint8_t lo, hi;
	lo = wire.read(); hi = wire.read();
	return hi<<8 | lo;
}

int32_t read32(SoftWire &wire) {
	uint8_t msb, lsb, xlsb;
	msb = wire.read(); lsb = wire.read(); xlsb = wire.read();
	return (uint32_t)msb<<12 | (uint32_t)lsb<<4 | (xlsb>>4 & 0x0F);
}

void write8(SoftWire &wire, uint8_t reg, uint8_t value) {  
    wire.beginTransmission((uint8_t)BME280address);
    wire.write((uint8_t)reg);
    wire.write((uint8_t)value);
    wire.endTransmission();
}

uint8_t read8(SoftWire &wire,uint8_t reg) {    
    wire.beginTransmission((uint8_t)BME280address);
    wire.write((uint8_t)reg);
    wire.endTransmission();
    wire.requestFrom((uint8_t)BME280address, (uint8_t)1);
    return wire.read();
}

// 
// Defaults to Weather Station Mode
// 
void BME280setSampling(SoftWire &wire, sensor_mode mode = MODE_FORCED,
                   sensor_sampling tempSampling = SAMPLING_X1,
                   sensor_sampling pressSampling = SAMPLING_X1,
                   sensor_sampling humSampling = SAMPLING_X1,
                   sensor_filter filter = FILTER_OFF,
                   standby_duration duration = STANDBY_MS_0_5) {


    _measReg.mode = mode;
    _measReg.osrs_t = tempSampling;
    _measReg.osrs_p = pressSampling;

    _humReg.osrs_h = humSampling;
    _configReg.filter = filter;
    _configReg.t_sb = duration;

	// making sure sensor is in sleep mode before setting configuration
    // as it otherwise may be ignored
	write8(wire, BME280_REGISTER_CONTROL, MODE_SLEEP);


	  // you must make sure to also set REGISTER_CONTROL after setting the
  	// CONTROLHUMID register, otherwise the values won't be applied (see
  	// DS 5.4.3)
  	write8(wire, BME280_REGISTER_CONTROLHUMID, _humReg.get());
  	write8(wire, BME280_REGISTER_CONFIG, _configReg.get());
  	write8(wire, BME280_REGISTER_CONTROL, _measReg.get());	
}


void BME280powerDown(SoftWire &wire){
	write8(wire, BME280_REGISTER_CONTROL, MODE_SLEEP);
}


// Must be called once at start
bool BME280setup(SoftWire &wire, int address = 0x76) {

    BME280address = address;

    delay(2);

  	// check if sensor, i.e. the chip ID is correct
  	BME280sensorID = read8(wire, BME280_REGISTER_CHIPID);
  	if (BME280sensorID != 0x60) {
		return false;
	}
    	

	// Set the mode to Normal, no upsampling
	BME280setSampling(wire);

	// Read the chip calibrations.
	wire.beginTransmission((uint8_t)BME280address);
	wire.write(0x88);
	wire.endTransmission();
	wire.requestFrom(BME280address, 26);
	for (int i=1; i<=3; i++) T[i] = read16(wire);     // Temperature
	for (int i=1; i<=9; i++) P[i] = read16(wire);     // Pressure
	wire.read();  // Skip 0xA0
	H[1] = (uint8_t)wire.read();                  // Humidity
	//
	wire.beginTransmission(BME280address);
	wire.write(BME280_REGISTER_CAL26);
	wire.endTransmission();
	wire.requestFrom(BME280address, 7);
	H[2] = read16(wire);
	H[3] = (uint8_t)wire.read();
	uint8_t e4 = wire.read(); uint8_t e5 = wire.read();
	H[4] = ((int16_t)((e4 << 4) + (e5 & 0x0F)));
	H[5] = ((int16_t)((wire.read() << 4) + ((e5 >> 4) & 0x0F)));
	H[6] = ((int8_t)wire.read()); // 0xE7



	// Read the temperature to set BME280t_fine
	BME280temperature(wire);

	return true;
}

// Returns temperature in DegC, resolution is 0.01 DegC
// Output value of “5123” equals 51.23 DegC
int32_t BME280temperature(SoftWire &wire) {

	wire.beginTransmission(BME280address);
	wire.write(0xFA);
	wire.endTransmission();
	wire.requestFrom(BME280address, 3);
	int32_t adc = read32(wire);
	// Compensate
	int32_t var1, var2;
	var1 = ((((adc>>3) - ((int32_t)((uint16_t)T[1])<<1))) * ((int32_t)T[2])) >> 11;
	var2 = ((((adc>>4) - ((int32_t)((uint16_t)T[1]))) * ((adc>>4) - ((int32_t)((uint16_t)T[1])))) >> 12);
	var2 = (var2 * ((int32_t)T[3])) >> 14;
	BME280t_fine = var1 + var2;
	return (BME280t_fine*5+128)>>8;
}

// Returns pressure in Pa as unsigned 32 bit integer
// Output value of “96386” equals 96386 Pa = 963.86 hPa
uint32_t BME280pressure(SoftWire &wire) {

	wire.beginTransmission(BME280address);
	wire.write(0xF7);
	wire.endTransmission();
	wire.requestFrom(BME280address, 3);
	int32_t adc = read32(wire);

	// Compensate
	int32_t var1, var2;
	uint32_t p;
	var1 = (((int32_t)BME280t_fine)>>1) - (int32_t)64000;
	var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((int32_t)P[6]);
	var2 = var2 + ((var1*((int32_t)P[5]))<<1);
	var2 = (var2>>2) + (((int32_t)P[4])<<16);
	var1 = (((P[3] * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((int32_t)P[2]) * var1)>>1))>>18;
	var1 = ((((32768+var1))*((int32_t)((uint16_t)P[1])))>>15);
	if (var1 == 0) return 0;
	p = (((uint32_t)(((int32_t)1048576) - adc) - (var2>>12)))*3125;
	if (p < 0x80000000) p = (p << 1) / ((uint32_t)var1);
	else p = (p / (uint32_t)var1) * 2;
	var1 = (((int32_t)P[9]) * ((int32_t)(((p>>3) * (p>>3))>>13)))>>12;
	var2 = (((int32_t)(p>>2)) * ((int32_t)P[8]))>>13;
	p = (uint32_t)((int32_t)p + ((var1 + var2 + P[7]) >> 4));
	return p;
}

// Humidity in %RH, resolution is 0.01%RH
// Output value of “4653” represents 46.53 %RH
uint32_t BME280humidity(SoftWire &wire) {

	wire.beginTransmission(BME280address);
	wire.write(0xFD);
	wire.endTransmission();
	wire.requestFrom(BME280address, 2);
	uint8_t hi = wire.read(); uint8_t lo = wire.read();
	int32_t adc = (uint16_t)(hi<<8 | lo);
	// Compensate
	int32_t var1; 
	var1 = (BME280t_fine - ((int32_t)76800));
	var1 = (((((adc << 14) - (((int32_t)H[4]) << 20) - (((int32_t)H[5]) * var1)) +
	((int32_t)16384)) >> 15) * (((((((var1 * ((int32_t)H[6])) >> 10) * (((var1 *
	((int32_t)H[3])) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) *
	((int32_t)H[2]) + 8192) >> 14));
	var1 = (var1 - (((((var1 >> 15) * (var1 >> 15)) >> 7) * ((int32_t)H[1])) >> 4));
	var1 = (var1 < 0 ? 0 : var1);
	var1 = (var1 > 419430400 ? 419430400 : var1);

	return (uint32_t)((var1>>12)*25)>>8;
}



#endif