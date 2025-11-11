//    FILE: MP28167_A.c
//  AUTHOR: Prashant Kumar
// VERSION: 0.0.1
//    DATE: 2025-07-14
// PURPOSE: Library for MP28167_A Buck-Boost Converter
//     URL: https://github.com/pk17r/MP28167_A
//
//  Read the datasheet for the details
//  
//  MIT License     (Date 2025-11-09)
//  
//  Copyright (c) 2025 Prashant Kumar
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

#ifdef MP28167_A_BUCK_BOOST_CONVERTER


/*
 *  include header files
 */

/* local includes */
#include "common.h"           /* common header file */
#include "variables.h"        /* global variables */
#include "functions.h"        /* external functions */


#define MP28167_A_LIB_VERSION              "0.0.1"

#define MP28167_A_I2C_ADDRESS         0x60

#define MP28167_A_VREF_L              0x00
#define MP28167_A_VREF_H              0x01
#define MP28167_A_VREF_GO             0x02
#define MP28167_A_IOUT_LIM            0x03
#define MP28167_A_CTL1                0x04
#define MP28167_A_CTL2                0x05
#define MP28167_A_STATUS              0x09
#define MP28167_A_INTERRUPT           0x0A
#define MP28167_A_MASK                0x0B
#define MP28167_A_ID1                 0x0C
#define MP28167_A_MFR_ID              0x27
#define MP28167_A_DEV_ID              0x28
#define MP28167_A_IC_REV              0x29

#define MP28167_A_CTL1_ENABLE                     0x80
#define MP28167_A_CTL1_DISABLE                    0x7F
#define MP28167_A_CTL1_FREQ_500kHz                0x00
#define MP28167_A_CTL1_FREQ_750kHz                0x04
#define MP28167_A_CTL1_HICCUP_MODE                0x40
#define MP28167_A_CTL1_LATCH_OFF_MODE             0x00

#define MP28167_A_STATUS_POWER_GOOD               0x80
#define MP28167_A_STATUS_CONSTANT_CURRENT         0x10

#define MP28167_A_INTERRUPT_OVER_CURRENT_ENTER    0x20

#define VOUT_MIN_mV         1000
#define VOUT_MAX_mV         20470
#define VREF_MIN_mV         80
#define VREF_MAX_mV         1637
#define VREF_REG_MIN        0
#define VREF_REG_MAX        2047


////////////////////////////////////////////////////////
//
//  PRIVATE
//
uint8_t MP28167_A_readRegister(uint8_t reg)
{
  if(I2C_Start(I2C_START) == I2C_ERROR) {
    return I2C_ERROR;
  }
  /* address (7 bit & write) */
  I2C.Byte = (MP28167_A_I2C_ADDRESS << 1);
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
  I2C.Byte = (MP28167_A_I2C_ADDRESS << 1);
  I2C.Byte |= 1;    // set read bit
  if(I2C_WriteByte(I2C_ADDRESS) != I2C_ACK) {
    return I2C_ERROR;
  }
  if(I2C_ReadByte(I2C_ACK) == I2C_ERROR) {
    return I2C_ERROR;
  }
  I2C_Stop();
  return I2C.Byte;
}


uint8_t MP28167_A_writeRegister(uint8_t reg, uint8_t value)
{
  if(I2C_Start(I2C_START) == I2C_ERROR) {
    return I2C_ERROR;
  }
  /* address (7 bit & write) */
  I2C.Byte = (MP28167_A_I2C_ADDRESS << 1);
  if(I2C_WriteByte(I2C_ADDRESS) != I2C_ACK) {
    return I2C_ERROR;
  }
  /* register address to write */
  I2C.Byte = reg;
  if(I2C_WriteByte(I2C_DATA) != I2C_ACK) {
    return I2C_ERROR;
  }
  /* value to write */
  I2C.Byte = value;
  if(I2C_WriteByte(I2C_DATA) != I2C_ACK) {
    return I2C_ERROR;
  }
  I2C_Stop();
  return I2C_OK;
}


////////////////////////////////////////////////////////
//
//  CONSTRUCTOR
//


uint8_t MP28167_A_begin()
{
  if (MP28167_A_isConnected() != I2C_OK)
    return I2C_ERROR;
  // set ALT pin Masks
  // MP28167_A_writeRegister(MP28167_A_MASK, 0x1F);

  // set 750kHz Frequency and MP28167_A_disable converter
  // uint8_t ctrl1_register = MP28167_A_readRegister(MP28167_A_CTL1);
  // ctrl1_register = (ctrl1_register | MP28167_A_CTL1_FREQ_750kHz);   // use 750kHz frequency
  // ctrl1_register = (ctrl1_register & MP28167_A_CTL1_DISABLE);       // MP28167_A_disable
  MP28167_A_writeRegister(MP28167_A_CTL1, 0x74);   // 0x74 = b01110100 = CTL1 Register = EN / OCP-OVP-HICCUP_LATCH-OFF / DISCHG_EN / MODE_Forced-PWM_Auto-PFM-PWM / FREQ_00-500kHz_01-750kHz / 00_Reserved
  // set Vout to 1V after MP28167_A_disable - good practice
  MP28167_A_setVout_mV(5050);
  MP28167_A_writeRegister(MP28167_A_IOUT_LIM, 0x06);    // 300mA
  return I2C_OK;
}


uint8_t MP28167_A_isConnected()
{
  if(I2C_Setup() != I2C_OK){
    return I2C_ERROR;
  }

  if(I2C_Start(I2C_START) != I2C_OK){
    return I2C_ERROR;
  }

  /* address (7 bit & write) */
  I2C.Byte = (MP28167_A_I2C_ADDRESS << 1);
  if(I2C_WriteByte(I2C_ADDRESS) != I2C_ACK) {
    I2C_Stop();
    return I2C_ERROR;
  }

  I2C_Stop();

  return I2C_OK;
}


uint16_t MP28167_A_VoutToVref_mV(uint16_t Vout_mV)
{
  return (uint16_t)(((uint32_t)Vout_mV * (uint32_t)MP28167_A_R2) / (uint32_t)(MP28167_A_R1 + MP28167_A_R2));
}


uint16_t MP28167_A_VrefToVout_mV(uint16_t Vref_mV)
{
  return (uint16_t)(((uint32_t)Vref_mV * (uint32_t)(MP28167_A_R1 + MP28167_A_R2)) / (uint32_t)MP28167_A_R2);
}


////////////////////////////////////////////////////////
//
//  CORE FUNCTIONS
//

void MP28167_A_enable() {
  // Serial.print("BEFORE interrupt_register=");Serial.println(MP28167_A_readRegister(MP28167_A_INTERRUPT), BIN);
  MP28167_A_writeRegister(MP28167_A_INTERRUPT, 0xFF);  // clear previous interrupts

  // uint8_t ctrl1_register = MP28167_A_readRegister(MP28167_A_CTL1);
  // // Serial.print("BEFORE ctrl1_register=");Serial.println(ctrl1_register, BIN);
  // ctrl1_register = (ctrl1_register | MP28167_A_CTL1_ENABLE);
  // MP28167_A_writeRegister(MP28167_A_CTL1, ctrl1_register);

  MP28167_A_writeRegister(MP28167_A_CTL1, 0xF4);
}


void MP28167_A_disable() {
  // uint8_t ctrl1_register = MP28167_A_readRegister(MP28167_A_CTL1);
  // // Serial.print("BEFORE ctrl1_register=");Serial.println(ctrl1_register, BIN);
  // ctrl1_register = (ctrl1_register & MP28167_A_CTL1_DISABLE);
  // MP28167_A_writeRegister(MP28167_A_CTL1, ctrl1_register);
  MP28167_A_writeRegister(MP28167_A_CTL1, 0x74);
  // set Vout to 1V after MP28167_A_disable - good practice
  // MP28167_A_setVout_mV(1000);
}


uint8_t MP28167_A_CCMode() {
  return ((MP28167_A_readRegister(MP28167_A_STATUS) & MP28167_A_STATUS_CONSTANT_CURRENT) >> 4);
}


uint8_t MP28167_A_PG() {
  return ((MP28167_A_readRegister(MP28167_A_STATUS) & MP28167_A_STATUS_POWER_GOOD) >> 7);
}


uint8_t MP28167_A_OCP() {
  uint8_t ocp = ((MP28167_A_readRegister(MP28167_A_INTERRUPT) & MP28167_A_INTERRUPT_OVER_CURRENT_ENTER) >> 5);
  MP28167_A_writeRegister(MP28167_A_INTERRUPT, 0xFF);    // clear current interrupts
  return ocp;
}


uint16_t MP28167_A_getVref_mV()
{
  uint8_t vref_l = MP28167_A_readRegister(MP28167_A_VREF_L);
  uint8_t vref_h = MP28167_A_readRegister(MP28167_A_VREF_H);
  uint16_t vref_register_val = ((vref_h << 3) | (vref_l & 0x07));
  uint16_t vref_mV = vref_register_val * 4 / 5;   // * 0.8
  return vref_mV;
}


uint8_t MP28167_A_setVref_mV(uint16_t vref_mV)
{
  MP28167_A_writeRegister(MP28167_A_INTERRUPT, 0xFF);  // clear previous interrupts

  if(vref_mV < VREF_MIN_mV)
    vref_mV = VREF_MIN_mV;
  else if(vref_mV > VREF_MAX_mV)
    vref_mV = VREF_MAX_mV;

  uint16_t vref_register_val = vref_mV * 5 / 4;   // / 0.8

  if(vref_register_val < VREF_REG_MIN)
    vref_register_val = VREF_REG_MIN;
  else if(vref_register_val > VREF_REG_MAX)
    vref_register_val = VREF_REG_MAX;

  uint8_t vref_l = (vref_register_val & 0x0007);
  uint8_t vref_h = ((vref_register_val >> 3) & 0x00ff);
  if(MP28167_A_writeRegister(MP28167_A_VREF_L, vref_l) == I2C_OK)
  {
    if(MP28167_A_writeRegister(MP28167_A_VREF_H, vref_h) == I2C_OK)
    {
      // set GO bit
      return MP28167_A_writeRegister(MP28167_A_VREF_GO, 0x01);
    }
  }
  return I2C_ERROR;
}


uint16_t MP28167_A_getVout_mV()
{
  return MP28167_A_VrefToVout_mV(MP28167_A_getVref_mV());
}


uint8_t MP28167_A_setVout_mV(uint16_t vout_mV)
{
  return MP28167_A_setVref_mV(MP28167_A_VoutToVref_mV(vout_mV));
}


uint8_t MP28167_A_setILim_mA(uint16_t IoutLim_mA)
{
  uint8_t ilim_register_val_desired = (uint8_t)(IoutLim_mA / (uint16_t)50);
  ilim_register_val_desired = (0x7F & ilim_register_val_desired);
  // Serial.print("ilim_register_val_desired=");Serial.println(ilim_register_val_desired);

  // read current IoutLimit
  uint8_t ilim_register_val_current = MP28167_A_getILimReg();

  // increase or decrease step by step
  uint8_t continue_writes = 1;
  while ((ilim_register_val_current != ilim_register_val_desired) && (continue_writes == 1))
  {
    uint8_t ilim_register_val_new = (ilim_register_val_current > ilim_register_val_desired ? (ilim_register_val_current - 1) : (ilim_register_val_current + 1));
    if(ilim_register_val_new <= 1) {
      ilim_register_val_new = 1;
      continue_writes = 0;
    }
    else if(ilim_register_val_new >= 127) {
      ilim_register_val_new = 127;
      continue_writes = 0;
    }
    uint8_t result = MP28167_A_writeRegister(MP28167_A_IOUT_LIM, ilim_register_val_new);
    // return if unsuccessful write
    if(result != I2C_OK)
      return I2C_ERROR;
    // read current IoutLimit
    ilim_register_val_current = MP28167_A_getILimReg();
  }

  // Serial.print("ilim_register_val_current=");Serial.println(ilim_register_val_current);
  return I2C_OK;
}


uint16_t MP28167_A_getILim_mA()
{
  return (((uint16_t)MP28167_A_getILimReg()) * (uint16_t)50);
}


uint8_t MP28167_A_getILimReg()
{
  return (0x7F & MP28167_A_readRegister(MP28167_A_IOUT_LIM));
}




#endif

//  -- END OF FILE --

