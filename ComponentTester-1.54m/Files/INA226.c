//    FILE: INA226.c
//  AUTHOR: Rob Tillaart
// VERSION: 0.6.0
//    DATE: 2021-05-18
// PURPOSE: Arduino library for INA226 power sensor
//     URL: https://github.com/RobTillaart/INA226
//
//  MODIFIED AND ADAPTED TO COMPONENT TESTER PROJECT
//      By: Prashant Kumar    https://github.com/pk17r
//    DATE: 2024-12-23
//
//  Read the datasheet for the details
//  
//  
//  MIT License     (Date 2025-01-02)
//  
//  Copyright (c) 2021-2024 Rob Tillaart
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

#ifdef INA226_POWER_MONITOR

/*
 *  include header files
 */

/* local includes */
#include "common.h"           /* common header file */
#include "variables.h"        /* global variables */
#include "functions.h"        /* external functions */


//  REGISTERS
#define INA226_CONFIGURATION              0x00
#define INA226_SHUNT_VOLTAGE              0x01
#define INA226_BUS_VOLTAGE                0x02
#define INA226_POWER                      0x03
#define INA226_CURRENT                    0x04
#define INA226_CALIBRATION                0x05
#define INA226_MASK_ENABLE                0x06
#define INA226_ALERT_LIMIT                0x07
#define INA226_MANUFACTURER               0xFE
#define INA226_DIE_ID                     0xFF


//  CONFIGURATION MASKS
#define INA226_CONF_RESET_MASK            0x8000
#define INA226_CONF_AVERAGE_MASK          0x0E00
#define INA226_CONF_BUSVC_MASK            0x01C0
#define INA226_CONF_SHUNTVC_MASK          0x0038
#define INA226_CONF_MODE_MASK             0x0007


//  CALIBRATION VAL
#define INA226_CALIBRATION_VAL                   (5120000 / (INA226_R_SHUNT_MILLI_OHM * INA226_CURRENT_LEAST_COUNT_MICRO_AMP))


////////////////////////////////////////////////////////
//
//  PRIVATE
//


uint16_t INA226__readRegister(uint8_t reg)
{
  if(I2C_Start(I2C_START) == I2C_ERROR) {
    return I2C_ERROR;
  }
  /* address (7 bit & write) */
  I2C.Byte = (INA226_I2C_ADDR << 1);
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
  I2C.Byte = (INA226_I2C_ADDR << 1);
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


uint8_t INA226__writeRegister(uint8_t reg, uint16_t value)
{
  if(I2C_Start(I2C_START) == I2C_ERROR) {
    return I2C_ERROR;
  }
  /* address (7 bit & write) */
  I2C.Byte = (INA226_I2C_ADDR << 1);
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


int16_t INA226_getBusVoltage_mV(void)
{
  uint16_t val = INA226__readRegister(INA226_BUS_VOLTAGE);
  return (int16_t)(((int32_t)val) * 125 / 100);  //  LSB fixed 1.25 mV
}

int16_t INA226_getShuntVoltage_mV(void)
{
  int16_t val = INA226__readRegister(INA226_SHUNT_VOLTAGE);
  return (int16_t)(((int32_t)val) * 25 / 10000);   //  LSB fixed 2.50 uV
}

uint16_t INA226_getLoadVoltage_mV(void)
{
  return (uint16_t)(INA226_getBusVoltage_mV() - INA226_getShuntVoltage_mV());
}

int32_t INA226_getCurrent_uA(void)
{
  int16_t value = INA226__readRegister(INA226_CURRENT);
  return (((int32_t)value) * INA226_CURRENT_LEAST_COUNT_MICRO_AMP);
}

uint8_t INA226_isConnected(void)
{
  if(I2C_Setup() != I2C_OK){
    return I2C_ERROR;
  }

  if(I2C_Start(I2C_START) != I2C_OK){
    return I2C_ERROR;
  }

  /* address (7 bit & write) */
  I2C.Byte = (INA226_I2C_ADDR << 1);
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
uint8_t INA226_setup() {
  if(INA226_isConnected() != I2C_OK){
    return 5;
  }

  // set calibration value
  if(INA226__writeRegister(INA226_CALIBRATION, INA226_CALIBRATION_VAL) != I2C_OK){
    return 6;
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
  mask |= (2 << 9);     //    INA226_16_SAMPLES   = 2,

  if(INA226__writeRegister(INA226_CONFIGURATION, mask) == I2C_ERROR)
    return 7;

  return I2C_OK;
}


#endif

/* ************************************************************************
 *   EOF
 * ************************************************************************ */
