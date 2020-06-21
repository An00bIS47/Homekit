/***************************************************************************
 
 cactus.io
  
 This is a library for the BME280 humidity, temperature & pressure sensor. It
 only supports the I2C bus. It does not support the SPI bus connection.
 
 
 ***************************************************************************/

#ifndef __BME280_SPI_H__

#define __BME280_SPI_H__

#include "Arduino.h"

#define BME280_ADDRESS      0x77          // define the default I2C address

// Name of Registers used in the BME280

#define    BME280_DIG_T1_REG   0x88
#define    BME280_DIG_T2_REG   0x8A
#define    BME280_DIG_T3_REG   0x8C
#define    BME280_DIG_P1_REG   0x8E
#define    BME280_DIG_P2_REG   0x90
#define    BME280_DIG_P3_REG   0x92
#define    BME280_DIG_P4_REG   0x94
#define    BME280_DIG_P5_REG   0x96
#define    BME280_DIG_P6_REG   0x98
#define    BME280_DIG_P7_REG   0x9A
#define    BME280_DIG_P8_REG   0x9C
#define    BME280_DIG_P9_REG   0x9E 
    
#define    BME280_DIG_H1_REG   0xA1
#define    BME280_DIG_H2_REG   0xE1
#define    BME280_DIG_H3_REG   0xE3
#define    BME280_DIG_H4_REG   0xE4
#define    BME280_DIG_H5_REG   0xE5
#define    BME280_DIG_H6_REG   0xE7
    
#define    BME280_REGISTER_CHIPID       0xD0
#define    BME280_REGISTER_VERSION      0xD1
#define    BME280_REGISTER_SOFTRESET    0xE0
#define    BME280_REGISTER_CAL26        0xE1
#define    BME280_REGISTER_CONTROLHUMID     0xF2
#define    BME280_REGISTER_CONTROL          0xF4
#define    BME280_REGISTER_CONFIG           0xF5
#define    BME280_REGISTER_PRESSUREDATA     0xF7
#define    BME280_REGISTER_TEMPDATA         0xFA
#define    BME280_REGISTER_HUMIDDATA        0xFD

#define    BME280_REGISTER_STATUS 0XF3


// structure to hold the calibration data that is programmed into the sensor in the factory
// during manufacture

struct BME280_Calibration_Data
{
    public:
    
        uint16_t dig_T1;
        int16_t  dig_T2;
        int16_t  dig_T3;
    
        uint16_t dig_P1;
        int16_t  dig_P2;
        int16_t  dig_P3;
        int16_t  dig_P4;
        int16_t  dig_P5;
        int16_t  dig_P6;
        int16_t  dig_P7;
        int16_t  dig_P8;
        int16_t  dig_P9;
    
        uint8_t  dig_H1;
        int16_t  dig_H2;
        uint8_t  dig_H3;
        int16_t  dig_H4;
        int16_t  dig_H5;
        int8_t   dig_H6;
    
};

/*=========================================================================

Main Class for the BME280 SPI library

=========================================================================*/


class BME280_SPI

{
    
public:

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

    
    BME280_SPI(int8_t cspin);							// use this for hardware SPI

    BME280_SPI(int8_t cspin, int8_t mosipin, int8_t misopin, int8_t sckpin);    // use this for software SPI

	bool begin(void);
    
	void setTempCal(int32_t);						// we can set a calibration ofsset for the temperature. 
												// this offset is in degrees celsius
    
    void readSensor(void);

    int32_t getTemperature(void);    
    uint32_t getHumidity(void);
    uint32_t getPressure(void);                 // pressure in hectapascals
    // float getPressure_MB(void);                 // pressure in millibars
    // float getTemperature_F(void);

    void setSampling(sensor_mode mode = MODE_FORCED,
                   sensor_sampling tempSampling = SAMPLING_X1,
                   sensor_sampling pressSampling = SAMPLING_X1,
                   sensor_sampling humSampling = SAMPLING_X1,
                   sensor_filter filter = FILTER_OFF,
                   standby_duration duration = STANDBY_MS_0_5);

    void takeForcedMeasurement();

    uint32_t sensorID(void);
    
private:

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

    
    BME280_Calibration_Data cal_data;
    
    void readTemperature(void);
    void readHumidity(void);
	void readPressure(void);
    void readSensorCoefficients(void);
    
	int32_t  tempcal;							// stores the temp offset calibration
    int32_t  temperature;                       // stores temperature
    uint32_t humidity;                          // stores humidity
    uint32_t pressure;
    
    
    // functions used for sensor communications
    
    uint8_t spixfer(uint8_t x);
    void      write8(byte reg, byte value);
    uint8_t   read8(byte reg);
    uint16_t  read16(byte reg);
	uint32_t  read24(byte reg);
    int16_t   readS16(byte reg);
    uint16_t  read16_LE(byte reg); // little endian
    int16_t   readS16_LE(byte reg); // little endian
    // uint8_t   _i2caddr;
    int32_t   _sensorID;
    int32_t t_fine;
    int8_t _cs, _mosi, _miso, _sck;

};

#endif
