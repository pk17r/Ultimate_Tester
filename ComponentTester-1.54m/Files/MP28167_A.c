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
#define MP28167_A_INTERRUPT_UVP_FALLING           0x08

#define MP28167_A_VOUT_MIN_mV         1030
#define MP28167_A_VOUT_MAX_mV         18000
#define MP28167_A_VREF_MIN_mV         80
#define MP28167_A_VREF_MAX_mV         1637
#define MP28167_A_VREF_REG_MIN        0
#define MP28167_A_VREF_REG_MAX        2047

#define MP28167_A_ILIM_MIN_mA         200
#define MP28167_A_ILIM_MAX_mA         3000
#define MP28167_A_I_IN_MAX_mA         4000
#define MP28167_A_EFFICIENCY_100      90

uint16_t Current_Ilim_mA = 0;
uint16_t Iout_Max_mA = 0;

////////////////////////////////////////////////////////
//
//  PRIVATE
//


/*
 *  write to MP28167_A register
 *
 *  requires:
 *  - Register: 8 bit register address
 *  - Value: 8 bit register value
 *
 *  returns:
 *  - 1 on success
 *  - 0 on any problem
 */

uint8_t MP28167_A_writeRegister(uint8_t Register, uint8_t Value)
{
  uint8_t           Flag = I2C_ERROR;   /* return value */
  uint8_t           OldTimeout;         /* old ACK timeout */

  OldTimeout = I2C.Timeout;             /* save old timeout */
  I2C.Timeout = 1;                      /* ACK timeout 10�s */

  /*
   *  procedure to write register:
   *  - address MP28167_A in write mode
   *  - select register
   *  - send value byte-wise
   */

  if (I2C_Start(I2C_START) == I2C_OK)             /* start */
  {
    /* send address & write bit, expect ACK from MP28167_A */
    I2C.Byte = MP28167_A_I2C_ADDRESS << 1;        /* address (7 bits) & write (0) */
    if (I2C_WriteByte(I2C_ADDRESS) == I2C_ACK)    /* address MP28167_A */
    {
      /* send register address, expect ACK from MP28167_A */
      I2C.Byte = Register;                        /* set register address */
      if (I2C_WriteByte(I2C_DATA) == I2C_ACK)     /* send data */
      {      
        /* send register value, expect ACK from MP28167_A */
        I2C.Byte = Value;                     /* set Value */
        if (I2C_WriteByte(I2C_DATA) == I2C_ACK)   /* send data */
        {
          Flag = I2C_OK;                                  /* signal success */
        }
      }
    }
  }

  I2C_Stop();                                     /* stop */

  I2C.Timeout = OldTimeout;             /* restore old timeout */

  return Flag;
}



/*
 *  read from MP28167_A register
 *
 *  requires:
 *  - Register: 8 bit register address
 *  - Value: pointer to 8 bit value
 *
 *  returns:
 *  - 1 on success
 *  - 0 on any problem
 */

uint8_t MP28167_A_readRegister(uint8_t Register, uint8_t *Value)
{
  uint8_t           Flag = I2C_ERROR;   /* return value */
  uint8_t           OldTimeout;         /* old ACK timeout */

  OldTimeout = I2C.Timeout;             /* save old timeout */
  I2C.Timeout = 1;                      /* ACK timeout 10�s */

  /*
   *  procedure to read register:
   *  - address MP28167_A in write mode
   *  - select register
   *  - restart I2C transfer
   *  - address MP28167_A in read mode
   *  - read register value byte-wise
   */

  if (I2C_Start(I2C_START) == I2C_OK)             /* start */
  {
    /* send address & write bit, expect ACK from MP28167_A */
    I2C.Byte = MP28167_A_I2C_ADDRESS << 1;              /* address (7 bits) & write (0) */
    if (I2C_WriteByte(I2C_ADDRESS) == I2C_ACK)    /* address MP28167_A */
    {
      /* send register address, expect ACK from MP28167_A */
      I2C.Byte = Register;                        /* set register address */
      if (I2C_WriteByte(I2C_DATA) == I2C_ACK)     /* send data */
      {
        /* end transfer for selecting register */
        // I2C_Stop();                               /* stop */

        /* new transfer for reading register */
        if (I2C_Start(I2C_REPEATED_START) == I2C_OK)       /* start */
        {
          /* send address & read bit, expect ACK from MP28167_A */
          I2C.Byte = (MP28167_A_I2C_ADDRESS << 1) | 0b00000001;   /* address (7 bits) & read (1) */
          if (I2C_WriteByte(I2C_ADDRESS) == I2C_ACK)        /* address MP28167_A */
          {
            /* read high byte and ACK */
            if (I2C_ReadByte(I2C_NACK) == I2C_OK)  /* read byte */
            {
              *Value = I2C.Byte;                  /* save result */
              Flag = I2C_OK;                      /* signal success */
            }
          }
        }
      }
    }
  }

  I2C_Stop();                                     /* stop */

  I2C.Timeout = OldTimeout;             /* restore old timeout */

  return Flag;
}



////////////////////////////////////////////////////////
//
//  CONSTRUCTOR
//


uint8_t MP28167_A_begin()
{
  // if (MP28167_A_isConnected() != I2C_OK)
  //   return I2C_ERROR;

  /* set disable pin as output */
  MP28167_A_DDR |= (1 << MP28167_A_DISABLE);     /* enable output */
  /* disable by turning grounding mosfet high */
  MP28167_A_PORT |= (1 << MP28167_A_DISABLE);    /* set pin high */

  // set 750kHz Frequency and MP28167_A_disable converter
  // uint8_t ctrl1_register = MP28167_A_readRegister(MP28167_A_CTL1);
  // ctrl1_register = (ctrl1_register | MP28167_A_CTL1_FREQ_750kHz);   // use 750kHz frequency
  // ctrl1_register = (ctrl1_register & MP28167_A_CTL1_DISABLE);       // MP28167_A_disable
  MP28167_A_writeRegister(MP28167_A_CTL1, 0x74);   // 0x74 = b01110100 = CTL1 Register = EN / OCP-OVP-HICCUP_LATCH-OFF / DISCHG_EN / MODE_Forced-PWM_Auto-PFM-PWM / FREQ_00-500kHz_01-750kHz / 00_Reserved
  // set Vout to 1V after MP28167_A_disable - good practice
  MP28167_A_setVout_mV(5010);
  // MP28167_A_setILim_mA(300 /*mA*/);
  Current_Ilim_mA = 300;
  MP28167_A_writeRegister(MP28167_A_IOUT_LIM, 0x06);    // Set Initial ILimit = 300mA
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
  /* set disable pin as output */
  MP28167_A_DDR |= (1 << MP28167_A_DISABLE);     /* enable output */
  /* enable by turning grounding mosfet low */
  MP28167_A_PORT &= ~(1 << MP28167_A_DISABLE);   /* set pin low */

  MP28167_A_writeRegister(MP28167_A_CTL1, 0xF4);
  wait10ms();

  MP28167_A_writeRegister(MP28167_A_INTERRUPT, 0xFF);  // clear previous interrupts

  MP28167_A_writeRegister(MP28167_A_MASK, 0x01);  // Masks off the PG indication function on ALT. This is required to disable ALT pin when VOUT [1V, 2.8V] with R1/R2 = 11.5
}


void MP28167_A_disable() {
  // disable via I2C
  MP28167_A_writeRegister(MP28167_A_CTL1, 0x74);

  // set Vout to 1V after MP28167_A_disable - good practice
  // MP28167_A_setVout_mV(1000);

  // wait at least 50ms to allow MP28167_A to drain output via resistive path
  wait100ms();

  /* set disable pin as output */
  MP28167_A_DDR |= (1 << MP28167_A_DISABLE);     /* enable output */
  /* disable by turning grounding mosfet high */
  MP28167_A_PORT |= (1 << MP28167_A_DISABLE);    /* set pin high */
}


uint8_t MP28167_A_GetEnableStatus() {
  uint8_t ctl1_register = 0;
  if(MP28167_A_readRegister(MP28167_A_CTL1, &ctl1_register) == I2C_OK) {
    return (ctl1_register >> 7);
  }
  return 0;
}


uint8_t MP28167_A_toggle() {
  uint8_t enable_bit = MP28167_A_GetEnableStatus();
  if(enable_bit == 1) {
    MP28167_A_disable();
    return 0;
  }
  else {
    MP28167_A_enable();
    return 1;
  }
}


uint8_t MP28167_A_CCMode() {
  uint8_t status_register = 0;
  if(MP28167_A_readRegister(MP28167_A_STATUS, &status_register) == I2C_OK) {
    return ((status_register & MP28167_A_STATUS_CONSTANT_CURRENT) >> 4);
  }
  return 0;
}


uint8_t MP28167_A_PG() {
  uint8_t status_register = 0;
  if(MP28167_A_readRegister(MP28167_A_STATUS, &status_register) == I2C_OK) {
    return ((status_register & MP28167_A_STATUS_POWER_GOOD) >> 7);
  }
  return 0;
}


uint8_t MP28167_A_OCP() {
  uint8_t interrupt_register = 0;
  if(MP28167_A_readRegister(MP28167_A_INTERRUPT, &interrupt_register) == I2C_OK) {
    return ((interrupt_register & MP28167_A_INTERRUPT_OVER_CURRENT_ENTER) >> 5);
  }
  return 0;
}


uint8_t MP28167_A_UVP() {
  uint8_t interrupt_register = 0;
  if(MP28167_A_readRegister(MP28167_A_INTERRUPT, &interrupt_register) == I2C_OK) {
    return ((interrupt_register & MP28167_A_INTERRUPT_UVP_FALLING) >> 3);
  }
  return 0;
}


void MP28167_A_GetVout_And_ILim_InRange(uint16_t Vin_mV) {
  // get Vout in range
  uint16_t Vout_mV = MP28167_A_getVout_mV();
  if(Vout_mV < MP28167_A_VOUT_MIN_mV)
    MP28167_A_setVout_mV(MP28167_A_VOUT_MIN_mV + 10);
  else if(Vout_mV > MP28167_A_VOUT_MAX_mV)
    MP28167_A_setVout_mV(MP28167_A_VOUT_MAX_mV - 100);

  // get ILim in range
  uint16_t Iout_Max_mA = MP28167_A_getILimMax_mA(Vin_mV);
  if(Iout_Max_mA < Current_Ilim_mA) {
    if(Iout_Max_mA < MP28167_A_ILIM_MIN_mA)
      Iout_Max_mA = MP28167_A_ILIM_MIN_mA;
    MP28167_A_setILim_mA(Iout_Max_mA);
    MP28167_A_getILim_mA(/*fetch = */ 1);
  }
}


void MP28167_A_Clear_Interrupts() {
  MP28167_A_writeRegister(MP28167_A_INTERRUPT, 0xFF);    // clear current interrupts
}


uint16_t MP28167_A_getVref_mV()
{
  uint8_t vref_l = 0, vref_h = 0;
  if(MP28167_A_readRegister(MP28167_A_VREF_L, &vref_l) == I2C_OK) {
    if(MP28167_A_readRegister(MP28167_A_VREF_H, &vref_h) == I2C_OK) {
      uint16_t vref_register_val = ((vref_h << 3) | (vref_l & 0x07));
      uint16_t vref_mV = vref_register_val * 4 / 5;   // * 0.8
      return vref_mV;
    }
  }
  return 0;
}


uint8_t MP28167_A_setVref_mV(uint16_t vref_mV)
{
  MP28167_A_writeRegister(MP28167_A_INTERRUPT, 0xFF);  // clear previous interrupts

  if(vref_mV < MP28167_A_VREF_MIN_mV)
    vref_mV = MP28167_A_VREF_MIN_mV;
  else if(vref_mV > MP28167_A_VREF_MAX_mV)
    vref_mV = MP28167_A_VREF_MAX_mV;

  uint16_t vref_register_val = vref_mV * 5 / 4;   // / 0.8

  if(vref_register_val < MP28167_A_VREF_REG_MIN)
    vref_register_val = MP28167_A_VREF_REG_MIN;
  else if(vref_register_val > MP28167_A_VREF_REG_MAX)
    vref_register_val = MP28167_A_VREF_REG_MAX;

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


void MP28167_A_increase_Vref(uint16_t steps)
{
  uint16_t vref_mV = MP28167_A_getVref_mV();
  if((steps > MP28167_A_VREF_MAX_mV) || ((vref_mV + steps) > MP28167_A_VREF_MAX_mV))
    vref_mV = MP28167_A_VREF_MAX_mV;
  else
    vref_mV += steps;
  MP28167_A_setVref_mV(vref_mV);
}


void MP28167_A_decrease_Vref(uint16_t steps)
{
  uint16_t vref_mV = MP28167_A_getVref_mV();
  if((vref_mV + MP28167_A_VREF_MIN_mV) < steps)
    vref_mV = MP28167_A_VREF_MIN_mV;
  else
    vref_mV -= steps;
  MP28167_A_setVref_mV(vref_mV);
}


void MP28167_A_inc_dec_Vout(uint16_t steps, uint8_t increase)
{
  uint16_t vout_mV = MP28167_A_getVout_mV();
  if(increase == 1) {
    if((steps > MP28167_A_VOUT_MAX_mV) || ((vout_mV + steps) > MP28167_A_VOUT_MAX_mV))
      vout_mV = MP28167_A_VOUT_MAX_mV;
    else
      vout_mV += steps;
  }
  else {
    if((vout_mV + MP28167_A_VOUT_MIN_mV) < steps)
      vout_mV = MP28167_A_VOUT_MIN_mV;
    else
      vout_mV -= steps;
  }
  if(vout_mV < MP28167_A_VOUT_MIN_mV)
    vout_mV = MP28167_A_VOUT_MIN_mV;
  else if(vout_mV > MP28167_A_VOUT_MAX_mV)
    vout_mV = MP28167_A_VOUT_MAX_mV;
  MP28167_A_setVout_mV(vout_mV);
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


uint16_t MP28167_A_getILim_mA(uint8_t fetch)
{
  if(fetch != 0)
    Current_Ilim_mA = (((uint16_t)MP28167_A_getILimReg()) * (uint16_t)50);
  return Current_Ilim_mA;
}


uint8_t MP28167_A_getILimReg()
{
  uint8_t iout_lim_register = 0;
  if(MP28167_A_readRegister(MP28167_A_IOUT_LIM, &iout_lim_register) == I2C_OK) {
    return (0x7F & iout_lim_register);
  }
  return 0;
}


uint16_t MP28167_A_getILimMax_mA(uint16_t Vin_mV)
{
  // continous Iout = 3A or Iin = 4A
  uint16_t Vout_mV = MP28167_A_getVout_mV();
  uint32_t Power_In_Max_mmW = (uint32_t)Vin_mV * MP28167_A_I_IN_MAX_mA;
  uint32_t Power_Out_Max_mmW = Power_In_Max_mmW * MP28167_A_EFFICIENCY_100 / 100;
  Iout_Max_mA = Power_Out_Max_mmW / Vout_mV;
  if(Iout_Max_mA > MP28167_A_ILIM_MAX_mA)
    Iout_Max_mA = MP28167_A_ILIM_MAX_mA;
  return Iout_Max_mA;
}


uint16_t MP28167_A_getILimMin_mA()
{
  return MP28167_A_ILIM_MIN_mA;
}


void MP28167_A_change_ILim(uint16_t steps, uint8_t increase)
{
  uint16_t IoutLim_new_mA = 0;
  uint16_t IoutLim_change_mA = 50*steps;
  if(increase)
    IoutLim_new_mA = Current_Ilim_mA + IoutLim_change_mA;
  else {
    if(IoutLim_change_mA > Current_Ilim_mA)
      IoutLim_new_mA = MP28167_A_ILIM_MIN_mA;
    else
      IoutLim_new_mA = Current_Ilim_mA - IoutLim_change_mA;
  }
  if(IoutLim_new_mA > Iout_Max_mA)
    IoutLim_new_mA = Iout_Max_mA;
  else if(IoutLim_new_mA < MP28167_A_ILIM_MIN_mA)
    IoutLim_new_mA = MP28167_A_ILIM_MIN_mA;
  MP28167_A_setILim_mA(IoutLim_new_mA);
  MP28167_A_getILim_mA(/*fetch = */ 1);
}


#endif

//  -- END OF FILE --

