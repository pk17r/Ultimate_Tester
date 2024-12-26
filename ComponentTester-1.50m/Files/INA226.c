//  ADAPTED to Component Tester Project by: Prashant Kumar
//    FILE: INA226.c
//  AUTHOR: Rob Tillaart
// VERSION: 0.6.0
//    DATE: 2021-05-18
// PURPOSE: Arduino library for INA226 power sensor
//     URL: https://github.com/RobTillaart/INA226
//
//  ADAPTED TO COMPONENT TESTED PROJECT
//      By: Prashant Kumar
//    DATE: 2024-12-23
//
//  Read the datasheet for the details


#define INA226_LIB_VERSION              "0.6.0"

#include "config.h"           /* global configuration */

#ifdef INA226_CURRENT_SENSOR

/*
 *  include header files
 */

/* local includes */
#include "common.h"           /* common header file */
#include "variables.h"        /* global variables */
#include "functions.h"        /* external functions */
#include "INA226.h"           /* INA226 specifics */


// debug statements
#define   Serial_print(x)   


// Local variables

uint16_t micro_current_LSB = INA_226_MICRO_CURRENT_LSB;

////////////////////////////////////////////////////////
//
//  PRIVATE
//


uint16_t INA226__readRegister(uint8_t reg)
{
  if(I2C_Start(I2C_START) == I2C_ERROR) {
    Serial_print("I2C_Start1 Error\n");
    return I2C_ERROR;
  }
  /* address (7 bit & write) */
  I2C.Byte = (INA226_I2C_ADDR << 1);
  if(I2C_WriteByte(I2C_ADDRESS) != I2C_ACK) {
    Serial_print("I2C_WriteByte1 Error\n");
    return I2C_ERROR;
  }
  /* register address to write */
  I2C.Byte = reg;
  if(I2C_WriteByte(I2C_DATA) != I2C_ACK) {
    Serial_print("I2C_WriteByte2 Error\n");
    return I2C_ERROR;
  }
  I2C_Stop();
  if(I2C_Start(I2C_START) == I2C_ERROR) {
    Serial_print("I2C_Start2 Error\n");
    return I2C_ERROR;
  }
  /* address (7 bit & read) */
  I2C.Byte = (INA226_I2C_ADDR << 1);
  I2C.Byte |= 1;    // set read bit
  if(I2C_WriteByte(I2C_ADDRESS) != I2C_ACK) {
    Serial_print("I2C_WriteByte3 Error\n");
    return I2C_ERROR;
  }
  if(I2C_ReadByte(I2C_ACK) == I2C_ERROR) {
    Serial_print("I2C_Read1 Error\n");
    return I2C_ERROR;
  }
  uint16_t ReadValue = I2C.Byte;
  // ReadValue |= read_byte;
  ReadValue <<= 8;
  // read_byte = 0;
  if(I2C_ReadByte(I2C_ACK) == I2C_ERROR) {
    Serial_print("I2C_Read2 Error\n");
    return I2C_ERROR;
  }
  ReadValue |= I2C.Byte;
  I2C_Stop();
  return ReadValue;
}


uint8_t INA226__writeRegister(uint8_t reg, uint16_t value)
{
  if(I2C_Start(I2C_START) == I2C_ERROR) {
    Serial_print("I2C_Start1 Error\n");
    return I2C_ERROR;
  }
  /* address (7 bit & write) */
  I2C.Byte = (INA226_I2C_ADDR << 1);
  if(I2C_WriteByte(I2C_ADDRESS) != I2C_ACK) {
    Serial_print("I2C_WriteByte1 Error\n");
    return I2C_ERROR;
  }
  /* register address to write */
  I2C.Byte = reg;
  if(I2C_WriteByte(I2C_DATA) != I2C_ACK) {
    Serial_print("I2C_WriteByte2 Error\n");
    return I2C_ERROR;
  }
  /* value MSB to write */
  I2C.Byte = (value >> 8);
  if(I2C_WriteByte(I2C_DATA) != I2C_ACK) {
    Serial_print("I2C_WriteByte2 Error\n");
    return I2C_ERROR;
  }
  /* value LSB to write */
  I2C.Byte = (value & 0xFF);
  if(I2C_WriteByte(I2C_DATA) != I2C_ACK) {
    Serial_print("I2C_WriteByte2 Error\n");
    return I2C_ERROR;
  }
  return I2C_OK;
}



////////////////////////////////////////////////////////
//
//  CORE FUNCTIONS
//

int32_t INA226_getBusVoltage_mV(void)
{
  uint16_t val = INA226__readRegister(INA226_BUS_VOLTAGE);
  Serial_print("INA226_getBusVoltage_V val = "); Serial_print(val); Serial_print("\n");
  return ((int32_t)val) * 1.25;  //  LSB fixed 1.25 mV
}

int32_t INA226_getShuntVoltage_mV(void)
{
  int16_t val = INA226__readRegister(INA226_SHUNT_VOLTAGE);
  return ((int32_t)val) * 2.5e-3;   //  LSB fixed 2.50 uV
}

int32_t INA226_getLoadVoltage_mV(void)
{
  return (INA226_getBusVoltage_mV() - INA226_getShuntVoltage_mV());
}

int32_t INA226_getCurrent_uA(void)
{
  int16_t val = INA226__readRegister(INA226_CURRENT);
  return (((int32_t)val) * (int32_t)micro_current_LSB);
}


/*  CALIBRATION
 *  
 *  Pre-calculated values:
 *  
 *  Imax_input = 3A, Resistance_Shunt_Ohms = 0.022
 *  
 *  normalize:	 true
 *  initial current_LSB:	91.6 uA / bit
 *  Pre-scale current_LSB:	91.6 uA / bit
 *  After scale current_LSB:	100.0 uA / bit
 *  Final current_LSB:	100.0 uA / bit
 *  Calibration:	2327
 *  Max current:	3.277 A
 *  Shunt:	0.0220 Ohm
 *  ShuntV:	0.0660 Volt
 *  
 */
// #define printdebug
/*uint16_t INA226_setMaxCurrentShunt(float maxCurrent, float shunt, uint8_t normalize)
{
  //  https://github.com/RobTillaart/INA226/pull/29

  //  #define printdebug true

  //  fix #16 - datasheet 6.5 Electrical Characteristics
  //            rounded value to 80 mV
  float shuntVoltage = maxCurrent * shunt;
  if (shuntVoltage > 0.080)         return INA226_ERR_SHUNTVOLTAGE_HIGH;
  if (maxCurrent < 0.001)           return INA226_ERR_MAXCURRENT_LOW;
  if (shunt < INA226_MINIMAL_SHUNT) return INA226_ERR_SHUNT_LOW;

  float _current_LSB = maxCurrent * 3.0517578125e-5;      //  maxCurrent / 32768;

#ifdef printdebug
  Serial.println();
  Serial.print("normalize:\t");
  Serial.println(normalize ? " true" : " false");
  Serial.print("initial current_LSB:\t");
  Serial.print(_current_LSB * 1e+6, 1);
  Serial.println(" uA / bit");
#endif

  //  normalize the LSB to a round number
  //  LSB will increase
  if (normalize)
  {
      // check if maxCurrent (normal) or shunt resistor
      // (due to unusual low resistor values in relation to maxCurrent) determines currentLSB
      // we have to take the upper value for currentLSB
      // (adjusted in 0.6.0)
      // calculation of currentLSB based on shunt resistor and calibration register limits (2 bytes)
      // cal = 0.00512 / ( shunt * currentLSB )
      // cal(max) = 2^15-1
      // currentLSB(min) = 0.00512 / ( shunt * cal(max) )
      // currentLSB(min) ~= 0.00512 / ( shunt * 2^15 )
      // currentLSB(min) ~= 2^9 * 1e-5 / ( shunt * 2^15 )
      // currentLSB(min) ~= 1e-5 / 2^6 / shunt
      // currentLSB(min) ~= 1.5625e-7 / shunt
    if ( 1.5625e-7 / shunt > _current_LSB ) {
      //  shunt resistor determines current_LSB
      //  => take this a starting point for current_LSB
      _current_LSB = 1.5625e-7 / shunt;
    }

#ifdef printdebug
    Serial.print("Pre-scale current_LSB:\t");
    Serial.print(_current_LSB * 1e+6, 1);
    Serial.println(" uA / bit");
#endif

    //  normalize _current_LSB to a value of 1, 2 or 5 * 1e-6 to 1e-3
    //  convert float to int
    uint16_t currentLSB_uA = _current_LSB * 1e+6;  // float(_current_LSB * 1e+6);
    currentLSB_uA++;  //  ceil() would be more precise, but uses 176 bytes of flash.

    uint16_t factor = 1;  //  1uA to 1000uA
    uint8_t i = 0;        //  1 byte loop reduces footprint
    uint8_t result = 0;
    do {
      if ( 1 * factor >= currentLSB_uA) {
        _current_LSB = 1 * factor * 1e-6;
        result = 1;
      } else if ( 2 * factor >= currentLSB_uA) {
        _current_LSB = 2 * factor * 1e-6;
        result = 1;
      } else if ( 5 * factor >= currentLSB_uA) {
        _current_LSB = 5 * factor * 1e-6;
        result = 1;
      } else {
        factor *= 10;
        i++;
      }
    } while ( (i < 4) && (result == 0) );  //  factor < 10000

    if (result == 0)  //  not succeeded to normalize.
    {
      _current_LSB = 0;
      return INA226_ERR_NORMALIZE_FAILED;
    }

#ifdef printdebug
    Serial.print("After scale current_LSB:\t");
    Serial.print(_current_LSB * 1e+6, 1);
    Serial.println(" uA / bit");
#endif
    // done
  }
  //  auto scale calibration if needed.
  uint32_t calib = round(0.00512 / (_current_LSB * shunt));
  while (calib > 32767)
  {
    _current_LSB *= 2;
    calib >>= 1;
  }
  micro_current_LSB = (uint16_t)(_current_LSB * 1e6);
  Serial.print("micro_current_LSB = "); Serial.println(micro_current_LSB);
  Serial.print("\ncalib = "); Serial.println(calib); Serial.print("\n");
  INA226__writeRegister(INA226_CALIBRATION, calib);

  float _maxCurrent = _current_LSB * 32768;
  float _shunt = shunt;

#ifdef printdebug
  Serial.print("Final current_LSB:\t");
  Serial.print(_current_LSB * 1e+6, 1);
  Serial.println(" uA / bit");
  Serial.print("Calibration:\t");
  Serial.println(calib);
  Serial.print("Max current:\t");
  Serial.print(_maxCurrent, 3);
  Serial.println(" A");
  Serial.print("Shunt:\t");
  Serial.print(_shunt, 4);
  Serial.println(" Ohm");
  Serial.print("ShuntV:\t");
  Serial.print(shuntVoltage, 4);
  Serial.println(" Volt");
#endif

  return INA226_ERR_NONE;
}*/


uint8_t INA226_isConnected(void)
{
  if(I2C_Setup() == I2C_ERROR) {
    Serial_print("I2C_Setup Error\n");
    return I2C_ERROR;
  }
  if(I2C_Start(I2C_START) == I2C_ERROR) {
    Serial_print("I2C_Start Error\n");
    return I2C_ERROR;
  }
  /* address (7 bit & write) */
  I2C.Byte = (INA226_I2C_ADDR << 1);
  if(I2C_WriteByte(I2C_ADDRESS) != I2C_ACK) {
    Serial_print("I2C_WriteByte1 Error\n");
    I2C_Stop();
    return I2C_ERROR;
  }
  I2C_Stop();
  return I2C_OK;
}

/*  INA226 Setup
 *  
 *  Pre-calculated calibration values using above commented function INA226_setMaxCurrentShunt()
 *  
 *  Imax_input = 3A, Resistance_Shunt_Ohms = 0.022
 *  
 *  normalize:	 true
 *  initial current_LSB:	61.0 uA / bit
 *  Pre-scale current_LSB:	61.0 uA / bit
 *  After scale current_LSB:	100.0 uA / bit
 *  Final current_LSB:	100.0 uA / bit
 *  Calibration:	2438
 *  Max current:	3.277 A
 *  Shunt:	0.0210 Ohm
 *  ShuntV:	0.0420 Volt
 *  
 */
uint8_t INA226_setup() {
  if(INA226_isConnected() == I2C_ERROR) {
    return I2C_ERROR;
  }
  // set calibration value
  if(INA226__writeRegister(INA226_CALIBRATION, INA_226_CALIBRATION_VAL) == I2C_ERROR) {
    return I2C_ERROR;
  }
  // set averaging
  //  for setAverage() and getAverage()
  //enum ina226_average_enum {
  //    INA226_1_SAMPLE     = 0,
  //    INA226_4_SAMPLES    = 1,
  //    INA226_16_SAMPLES   = 2,
  //    INA226_64_SAMPLES   = 3,
  //    INA226_128_SAMPLES  = 4,
  //    INA226_256_SAMPLES  = 5,
  //    INA226_512_SAMPLES  = 6,
  //    INA226_1024_SAMPLES = 7
  //};
  uint16_t mask = INA226__readRegister(INA226_CONFIGURATION);
  mask &= ~INA226_CONF_AVERAGE_MASK;
  mask |= (2 << 9);
  if(INA226__writeRegister(INA226_CONFIGURATION, mask) == I2C_ERROR) {
    return I2C_ERROR;
  }
  return I2C_OK;
}

//uint16_t INA226_getManufacturerID(void)
//{
//  return INA226__readRegister(INA226_MANUFACTURER);
//}
//uint16_t INA226_getDieID(void)
//{
//  return INA226__readRegister(INA226_DIE_ID);
//}



#endif

/* ************************************************************************
 *   EOF
 * ************************************************************************ */
