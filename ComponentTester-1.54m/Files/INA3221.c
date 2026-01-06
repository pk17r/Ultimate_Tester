//    FILE: INA3221.c
//  AUTHOR: Prashant Kumar
//    DATE: 2025-03-07
//     URL: https://github.com/pk17r
//
//  MADE USING ARDUINO LIBRARY BY:
//  AUTHOR: Beast Devices, Andrejs Bondarevs
// PURPOSE: Arduino library for INA3221 current and voltage sensor.
//     URL: https://github.com/Tinyu-Zhao/INA3221
//
//  MIT License
//  
//  Copyright (c) 2020 Beast Devices, Andrejs Bondarevs
//  
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.
//  

#include "config.h"           /* global configuration */

#ifdef INA3221_POWER_MONITOR

/*
 *  include header files
 */

/* local includes */
#include "common.h"           /* common header file */
#include "variables.h"        /* global variables */
#include "functions.h"        /* external functions */


//  REGISTERS
#define INA3221_REG_CONF                    0x00
#define INA3221_REG_CH1_SHUNTV              0x01
#define INA3221_REG_CH1_BUSV                0x02
#define INA3221_REG_CH2_SHUNTV              0x03
#define INA3221_REG_CH2_BUSV                0x04
#define INA3221_REG_CH3_SHUNTV              0x05
#define INA3221_REG_CH3_BUSV                0x06
#define INA3221_REG_CH1_CRIT_ALERT_LIM      0x07
#define INA3221_REG_CH1_WARNING_ALERT_LIM   0x08
#define INA3221_REG_CH2_CRIT_ALERT_LIM      0x09
#define INA3221_REG_CH2_WARNING_ALERT_LIM   0x0A
#define INA3221_REG_CH3_CRIT_ALERT_LIM      0x0B
#define INA3221_REG_CH3_WARNING_ALERT_LIM   0x0C
#define INA3221_REG_SHUNTV_SUM              0x0D
#define INA3221_REG_SHUNTV_SUM_LIM          0x0E
#define INA3221_REG_MASK_ENABLE             0x0F
#define INA3221_REG_PWR_VALID_HI_LIM        0x10
#define INA3221_REG_PWR_VALID_LO_LIM        0x11
#define INA3221_REG_MANUF_ID                0xFE
#define INA3221_REG_DIE_ID                  0xFF
#define INA3221_CONF_AVERAGE_MASK          0x0E00


////////////////////////////////////////////////////////
//
//  PRIVATE
//


uint16_t INA3221__readRegister(uint8_t reg)
{
  if(I2C_Start(I2C_START) == I2C_ERROR) {
    return I2C_ERROR;
  }
  /* address (7 bit & write) */
  I2C.Byte = (INA3221_I2C_ADDR << 1);
  if(I2C_WriteByte(I2C_ADDRESS) != I2C_ACK) {
    return I2C_ERROR;
  }
  /* register address to write */
  I2C.Byte = reg;
  if(I2C_WriteByte(I2C_DATA) != I2C_ACK) {
    return I2C_ERROR;
  }
  I2C_Stop();
  /* repeated start condition */
  if(I2C_Start(I2C_START) == I2C_ERROR) {
    return I2C_ERROR;
  }
  /* address (7 bit & read) */
  I2C.Byte = (INA3221_I2C_ADDR << 1);
  I2C.Byte |= 1;    // set read bit
  if(I2C_WriteByte(I2C_ADDRESS) != I2C_ACK) {
    return I2C_ERROR;
  }
  if(I2C_ReadByte(I2C_ACK) == I2C_ERROR) {
    return I2C_ERROR;
  }
  uint16_t ReadValue = I2C.Byte;
  // ReadValue |= read_byte;
  ReadValue <<= 8;
  // read_byte = 0;
  if(I2C_ReadByte(I2C_ACK) == I2C_ERROR) {
    return I2C_ERROR;
  }
  ReadValue |= I2C.Byte;
  I2C_Stop();
  return ReadValue;
}


uint8_t INA3221__writeRegister(uint8_t reg, uint16_t value)
{
  if(I2C_Start(I2C_START) == I2C_ERROR) {
    return I2C_ERROR;
  }
  /* address (7 bit & write) */
  I2C.Byte = (INA3221_I2C_ADDR << 1);
  if(I2C_WriteByte(I2C_ADDRESS) != I2C_ACK) {
    return I2C_ERROR;
  }
  /* register address to write */
  I2C.Byte = reg;
  if(I2C_WriteByte(I2C_DATA) != I2C_ACK) {
    return I2C_ERROR;
  }
  /* value MSB to write */
  I2C.Byte = (value >> 8);
  if(I2C_WriteByte(I2C_DATA) != I2C_ACK) {
    return I2C_ERROR;
  }
  /* value LSB to write */
  I2C.Byte = (value & 0xFF);
  if(I2C_WriteByte(I2C_DATA) != I2C_ACK) {
    return I2C_ERROR;
  }
  I2C_Stop();
  return I2C_OK;
}



////////////////////////////////////////////////////////
//
//  CORE FUNCTIONS
//


uint16_t INA3221_getBusVoltage_mV(uint8_t channel)
{
  uint16_t val = 0;
  switch (channel) {
    case 1:
      val = INA3221__readRegister(INA3221_REG_CH1_BUSV);
      break;
    case 2:
      val = INA3221__readRegister(INA3221_REG_CH2_BUSV);
      break;
    case 3:
      val = INA3221__readRegister(INA3221_REG_CH3_BUSV);
      break;
  }
  return val;
}

int32_t INA3221_getShuntVoltage_uV(uint8_t channel)
{
  uint16_t val = 0;
  switch (channel) {
    case 1:
      val = INA3221__readRegister(INA3221_REG_CH1_SHUNTV);
      break;
    case 2:
      val = INA3221__readRegister(INA3221_REG_CH2_SHUNTV);
      break;
    case 3:
      val = INA3221__readRegister(INA3221_REG_CH3_SHUNTV);
      break;
  }
  // 1 LSB = 5uV
  return ((int32_t)val * 5);
}

uint16_t INA3221_getLoadVoltage_mV(uint8_t channel)
{
  return (uint16_t)((int32_t)INA3221_getBusVoltage_mV(channel) - INA3221_getShuntVoltage_uV(channel) / 1000);
}

int32_t INA3221_getCurrent_uA(uint8_t channel)
{
  int32_t current_uA  = 0;
  int32_t shunt_uV  = INA3221_getShuntVoltage_uV(channel);
  switch (channel) {
    case 1:
      current_uA = (shunt_uV * 1000000) / INA3221_CH1_R_SHUNT_MICRO_OHM;
      break;
    case 2:
      current_uA = (shunt_uV * 1000000) / INA3221_CH2_R_SHUNT_MICRO_OHM;
      break;
    case 3:
      current_uA = (shunt_uV * 1000000) / INA3221_CH3_R_SHUNT_MICRO_OHM;
      break;
  }
  return current_uA;
}

uint8_t INA3221_isConnected(void)
{
  if(I2C_Setup() != I2C_OK){
    return I2C_ERROR;
  }

  if(I2C_Start(I2C_START) != I2C_OK){
    return I2C_ERROR;
  }

  /* address (7 bit & write) */
  I2C.Byte = (INA3221_I2C_ADDR << 1);
  if(I2C_WriteByte(I2C_ADDRESS) != I2C_ACK) {
    I2C_Stop();
    return I2C_ERROR;
  }

  I2C_Stop();

  return I2C_OK;
}

/* 
 * INA226 Setup
 */
uint8_t INA3221_setup()
{
  if(INA3221_isConnected() != I2C_OK){
    return I2C_ERROR;
  }

  // set averaging
  //  for setAverage() and getAverage()
  //enum ina3221_average_enum {
  //    INA3221_1_SAMPLE     = 0,
  //    INA3221_4_SAMPLES    = 1,
  //    INA3221_16_SAMPLES   = 2,
  //    INA3221_64_SAMPLES   = 3,
  //    INA3221_128_SAMPLES  = 4,
  //    INA3221_256_SAMPLES  = 5,
  //    INA3221_512_SAMPLES  = 6,
  //    INA3221_1024_SAMPLES = 7
  //};
  uint16_t mask = INA3221__readRegister(INA3221_REG_CONF);
  mask &= ~INA3221_CONF_AVERAGE_MASK;
  #if INA3221_AVERAGING_SAMPLES < 0
  mask |= (0 << 9);     //    INA3221_1_SAMPLE   = 0,
  #elif INA3221_AVERAGING_SAMPLES > 7
  mask |= (7 << 9);     //    INA3221_1024_SAMPLES   = 7,
  #else
  mask |= (INA3221_AVERAGING_SAMPLES << 9);
  #endif

  if(INA3221__writeRegister(INA3221_REG_CONF, mask) == I2C_ERROR)
    return I2C_ERROR;

  return I2C_OK;
}


#endif

/* ************************************************************************
 *   EOF
 * ************************************************************************ */
