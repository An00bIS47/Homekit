/***************************************************************************
 
 cactus.io
 
 This is a library for the BME280 humidity, temperature & pressure sensor. It
 only supports the SPI bus. It does not support the I2C bus connection.
 
 No warranty is given
 
***************************************************************************/

#include "BME280_SPI.h"

// #include <math.h>
#include <SPI.h>


/***************************************************************************
 
 PUBLIC FUNCTIONS
 
 ***************************************************************************/

BME280_SPI::BME280_SPI(int8_t cspin)

: _cs(cspin), _mosi(-1), _miso(-1), _sck(-1)

{
	  tempcal = 0;
    temperature = 0;
    humidity = 0;
    pressure = 0;
}


BME280_SPI::BME280_SPI(int8_t cspin, int8_t mosipin, int8_t misopin, int8_t sckpin)

: _cs(cspin), _mosi(mosipin), _miso(misopin), _sck(sckpin)

{ 
	  tempcal = 0;
    temperature = 0;
    humidity = 0;
    pressure = 0;
}


void BME280_SPI::setTempCal(int32_t tcal)
{
	tempcal = tcal;
}

void BME280_SPI::readSensor(void)
{
    readTemperature();
    readHumidity();
    readPressure();
}

int32_t BME280_SPI::getTemperature(void)
{
    return temperature + tempcal;
}

// float BME280_SPI::getTemperature_F(void)
// {
//     float temp = temperature + tempcal;
    
//     return temp * 1.8 + 32;
// }

uint32_t BME280_SPI::getHumidity(void)
{
    return humidity;
}

// Gets the pressure in millibars
// float BME280_SPI::getPressure_MB(void) {
    
//     return pressure / 100.0F;
// }

// Gets the pressure in hectapascals
uint32_t BME280_SPI::getPressure(void) {
    
    return pressure;
}

/***************************************************************************
 
 PRIVATE FUNCTIONS
 
 ***************************************************************************/


bool BME280_SPI::begin() {
    
    // Wire.begin();
    
	digitalWrite(_cs,HIGH);
	pinMode(_cs,OUTPUT);

	if(_sck == -1)
		SPI.begin();					// Hardware SPI
	else
	{
		pinMode(_sck,OUTPUT);
		pinMode(_mosi,OUTPUT);
		pinMode(_miso,INPUT);
	}

    
    _sensorID = read8(BME280_REGISTER_CHIPID);
    if (_sensorID != 0x60){
        return false;
    }        

    // reset the device using soft-reset
    // this makes sure the IIR is off, etc.
    write8(BME280_REGISTER_SOFTRESET, 0xB6);
    
    // wait for chip to wake up.
    delay(10);
    
    readSensorCoefficients();
    
    // Set Humidity oversampling to 1
    // write8(BME280_REGISTER_CONTROLHUMID, 0x01); // Set before CONTROL (DS 5.4.3)    
    // write8(BME280_REGISTER_CONTROL, 0x3F);

    setSampling(); // use defaults

    delay(100);
    
    return true;
    
}


/*!
 *   @brief  setup sensor with given parameters / settings
 *
 *   This is simply a overload to the normal begin()-function, so SPI users
 *   don't get confused about the library requiring an address.
 *   @param mode the power mode to use for the sensor
 *   @param tempSampling the temp samping rate to use
 *   @param pressSampling the pressure sampling rate to use
 *   @param humSampling the humidity sampling rate to use
 *   @param filter the filter mode to use
 *   @param duration the standby duration to use
 */
void BME280_SPI::setSampling(sensor_mode mode,
                                  sensor_sampling tempSampling,
                                  sensor_sampling pressSampling,
                                  sensor_sampling humSampling,
                                  sensor_filter filter,
                                  standby_duration duration) {
  _measReg.mode = mode;
  _measReg.osrs_t = tempSampling;
  _measReg.osrs_p = pressSampling;

  _humReg.osrs_h = humSampling;
  _configReg.filter = filter;
  _configReg.t_sb = duration;

  // making sure sensor is in sleep mode before setting configuration
  // as it otherwise may be ignored
  write8(BME280_REGISTER_CONTROL, MODE_SLEEP);

  // you must make sure to also set REGISTER_CONTROL after setting the
  // CONTROLHUMID register, otherwise the values won't be applied (see
  // DS 5.4.3)
  write8(BME280_REGISTER_CONTROLHUMID, _humReg.get());
  write8(BME280_REGISTER_CONFIG, _configReg.get());
  write8(BME280_REGISTER_CONTROL, _measReg.get());
}


/*!
 *  @brief  Take a new measurement (only possible in forced mode)
 */
void BME280_SPI::takeForcedMeasurement() {
  // If we are in forced mode, the BME sensor goes back to sleep after each
  // measurement and we need to set it to forced mode once at this point, so
  // it will take the next measurement and then return to sleep again.
  // In normal mode simply does new measurements periodically.
  if (_measReg.mode == MODE_FORCED) {
    // set to forced mode, i.e. "take next measurement"
    write8(BME280_REGISTER_CONTROL, _measReg.get());
    // wait until measurement has been completed, otherwise we would read
    // the values from the last measurement
    while (read8(BME280_REGISTER_STATUS) & 0x08)
      delay(1);
  }
}

/*!
 *   Returns Sensor ID found by init() for diagnostics
 *   @returns Sensor ID 0x60 for BME280, 0x56, 0x57, 0x58 BMP280
 */
uint32_t BME280_SPI::sensorID(void) { return _sensorID; }


void BME280_SPI::readTemperature(void)
{
    
    int32_t var1, var2;
    
    int32_t adc_T = read24(BME280_REGISTER_TEMPDATA);
    adc_T >>= 4;
    
    var1  = ((((adc_T>>3) - ((int32_t)cal_data.dig_T1 <<1))) *
             
             ((int32_t)cal_data.dig_T2)) >> 11;
    
    var2  = (((((adc_T>>4) - ((int32_t)cal_data.dig_T1)) *
               
               ((adc_T>>4) - ((int32_t)cal_data.dig_T1))) >> 12) *
             
             ((int32_t)cal_data.dig_T3)) >> 14;
    
    t_fine = var1 + var2;
    
    temperature  = (t_fine * 5 + 128) >> 8;        
}


void BME280_SPI::readPressure(void) {
    
    int64_t var1, var2, p;
    
    int32_t adc_P = read24(BME280_REGISTER_PRESSUREDATA);
    adc_P >>= 4;
    
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)cal_data.dig_P6;
    var2 = var2 + ((var1*(int64_t)cal_data.dig_P5)<<17);
    var2 = var2 + (((int64_t)cal_data.dig_P4)<<35);
    var1 = ((var1 * var1 * (int64_t)cal_data.dig_P3)>>8) +
    
    ((var1 * (int64_t)cal_data.dig_P2)<<12);
    
    var1 = (((((int64_t)1)<<47)+var1))*((int64_t)cal_data.dig_P1)>>33;
    
    if (var1 == 0) {
        
        pressure = 0;  // avoid exception caused by division by zero
        
    }
    
    p = 1048576 - adc_P;
    p = (((p<<31) - var2)*3125) / var1;
    
    var1 = (((int64_t)cal_data.dig_P9) * (p>>13) * (p>>13)) >> 25;
    var2 = (((int64_t)cal_data.dig_P8) * p) >> 19;
    
    p = ((p + var1 + var2) >> 8) + (((int64_t)cal_data.dig_P7)<<4);
    
    pressure = p/256;
    
}



void BME280_SPI::readHumidity(void) {
    
    int32_t adc_H = read16(BME280_REGISTER_HUMIDDATA);
    
    int32_t v_x1_u32r;
    
    v_x1_u32r = (t_fine - ((int32_t)76800));
    
    v_x1_u32r = (((((adc_H << 14) - (((int32_t)cal_data.dig_H4) << 20) -
                    
                    (((int32_t)cal_data.dig_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) *
                 
                 (((((((v_x1_u32r * ((int32_t)cal_data.dig_H6)) >> 10) *
                      
                      (((v_x1_u32r * ((int32_t)cal_data.dig_H3)) >> 11) + ((int32_t)32768))) >> 10) +
                    
                    ((int32_t)2097152)) * ((int32_t)cal_data.dig_H2) + 8192) >> 14));
    
    
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
                               
                               ((int32_t)cal_data.dig_H1)) >> 4));
    
    
    v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
    
    v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
    
    uint32_t h = (v_x1_u32r>>12);
    
    humidity = h / 1024.0;    
}


/**************************************************************************

Read the values that are programmed into the sensor during amanufacture

**************************************************************************/

void BME280_SPI::readSensorCoefficients(void)

{
    
    cal_data.dig_T1 = read16_LE(BME280_DIG_T1_REG); 
    cal_data.dig_T2 = readS16_LE(BME280_DIG_T2_REG);
    cal_data.dig_T3 = readS16_LE(BME280_DIG_T3_REG);
    cal_data.dig_P1 = read16_LE(BME280_DIG_P1_REG);
    cal_data.dig_P2 = readS16_LE(BME280_DIG_P2_REG);
    cal_data.dig_P3 = readS16_LE(BME280_DIG_P3_REG);
    cal_data.dig_P4 = readS16_LE(BME280_DIG_P4_REG);
    cal_data.dig_P5 = readS16_LE(BME280_DIG_P5_REG);
    cal_data.dig_P6 = readS16_LE(BME280_DIG_P6_REG);
    cal_data.dig_P7 = readS16_LE(BME280_DIG_P7_REG);
    cal_data.dig_P8 = readS16_LE(BME280_DIG_P8_REG);
    cal_data.dig_P9 = readS16_LE(BME280_DIG_P9_REG);
    cal_data.dig_H1 = read8(BME280_DIG_H1_REG);
    cal_data.dig_H2 = readS16_LE(BME280_DIG_H2_REG);
    cal_data.dig_H3 = read8(BME280_DIG_H3_REG);
    cal_data.dig_H4 = (read8(BME280_DIG_H4_REG) << 4) | (read8(BME280_DIG_H4_REG+1) & 0xF);
    cal_data.dig_H5 = (read8(BME280_DIG_H5_REG+1) << 4) | (read8(BME280_DIG_H5_REG) >> 4);
    cal_data.dig_H6 = (int8_t)read8(BME280_DIG_H6_REG);
    
}

/**************************************************************************

Transfers data over the SPI bus

**************************************************************************/

uint8_t BME280_SPI::spixfer(uint8_t x)
{
	if(_sck == -1)
		return SPI.transfer(x);

	uint8_t response = 0;
	for(int i=7; i>=0; i--) {

		response <<= 1;
		digitalWrite(_sck,LOW);
		digitalWrite(_mosi, x & (1<<i));
		digitalWrite(_sck,HIGH);
		if(digitalRead(_miso))
			response |= 1;
	}

	return response;
}


/**************************************************************************

Writes an 8 bit value over SPI

**************************************************************************/

void BME280_SPI::write8(byte reg, byte value)
{
    
    if(_cs == -1)      // Is it hardware SPI
		SPI.beginTransaction(SPISettings(500000,MSBFIRST,SPI_MODE0));

	digitalWrite(_cs,LOW);
	spixfer(reg & ~0x80);
	spixfer(value);
	digitalWrite(_cs,HIGH);

	if(_sck == -1)
		SPI.endTransaction();
    
}

/**************************************************************************
 
 Reads a signed 8 bit value over the SPI bus
 
 **************************************************************************/

uint8_t BME280_SPI::read8(byte reg)
{
    
    uint8_t value;
    
    if(_cs == -1)      // Is it hardware SPI
		SPI.beginTransaction(SPISettings(500000,MSBFIRST,SPI_MODE0));

	digitalWrite(_cs,LOW);
	spixfer(reg | 0x80);
	value = spixfer(0);
	digitalWrite(_cs,HIGH);

	if(_sck == -1)
		SPI.endTransaction();
    
    return value;
    
}


/**************************************************************************

Reads a signed 16 bit value over the SPI bus

**************************************************************************/

uint16_t BME280_SPI::read16(byte reg)
{
    
    uint16_t value;
    
    if(_cs == -1)      // Is it hardware SPI
		SPI.beginTransaction(SPISettings(500000,MSBFIRST,SPI_MODE0));

	digitalWrite(_cs,LOW);
	spixfer(reg | 0x80);
	value = (spixfer(0) << 8) | spixfer(0);
	digitalWrite(_cs,HIGH);

	if(_sck == -1)
		SPI.endTransaction();
    
    return value;
}

uint16_t BME280_SPI::read16_LE(byte reg) {
    
    uint16_t temp = read16(reg);
    
    return (temp >> 8) | (temp << 8);
}

int16_t BME280_SPI::readS16(byte reg)
{
    return (int16_t)read16(reg);
}

int16_t BME280_SPI::readS16_LE(byte reg)
{
    return (int16_t)read16_LE(reg);
}


/**************************************************************************

Reads a signed 24 bit value over the SPI bus

**************************************************************************/

uint32_t BME280_SPI::read24(byte reg)
{
    
    uint32_t value;

	 if(_cs == -1)      // Is it hardware SPI
		SPI.beginTransaction(SPISettings(500000,MSBFIRST,SPI_MODE0));

	digitalWrite(_cs,LOW);
	spixfer(reg | 0x80);

	value = spixfer(0);
	value <<= 8;
	value |= spixfer(0);
	value <<= 8;
	value |= spixfer(0);
	
	digitalWrite(_cs,HIGH);

	if(_sck == -1)
		SPI.endTransaction();

	return value;
}
