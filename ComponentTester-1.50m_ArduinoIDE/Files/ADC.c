/* ************************************************************************
 *
 *   ADC functions
 *
 *   (c) 2012-2023 by Markus Reschke
 *   based on code from Markus Frejek and Karl-Heinz Kübbeler
 *
 * ************************************************************************ */


/*
 *  local constants
 */

/* source management */
#define ADC_C


/*
 *  include header files
 */

/* local includes */
#include "config.h"           /* global configuration */
#include "common.h"           /* common header file */
#include "variables.h"        /* global variables */
#include "functions.h"        /* external functions */
#include <Arduino.h>


/* ************************************************************************
 *   ADC
 * ************************************************************************ */


/*
 *  read ADC channel and return voltage in mV
 *  - use Vcc as reference by default
 *  - switch to bandgap reference for low voltages (< 1.0V) to improve
 *    ADC resolution
 *  - with a 125kHz ADC clock a single conversion needs about 0.1ms
 *    with 25 samples we end up with about 2.6ms
 *
 *  requires:
 *  - Channel: ADC MUX input channel
 *    - ATmega328: register bits corresponding with MUX0-3
 *    - ATmega324/644/1284: register bits corresponding with MUX0-4
 *    - ATmega640/1280/2560: register bits corresponding with MUX0-4
 *      (todo: add MUX5 to support also ADC8-15)
 */

uint16_t ReadU(uint8_t Channel)
{
  uint16_t          U;             /* return value (mV) */
  uint8_t           Counter;       /* loop counter */
  uint32_t          Value;         /* ADC value */
  uint32_t          adc_value;         /* ADC value */


  /*
   *  sample ADC readings
   */

  Value = 0UL;                     /* reset sampling variable */
  Counter = 0;                     /* reset counter */

  while (Counter < Cfg.Samples)    /* take samples */
  {
    adc_value = analogRead(Channel);

    Value += adc_value;                 /* add ADC reading */

    Counter++;                     /* another sample done */
  }


  /*
   *  convert ADC reading to voltage
   *  - single sample: U = ADC reading * U_ref / 1024
   */

  /* get voltage of reference used */
  U = Cfg.Bandgap;                 /* voltage of bandgap reference */
  
  /* convert to voltage; */
  Value *= U;                      /* ADC readings * U_ref */
//  Value += 511 * Cfg.Samples;      /* automagic rounding */
  Value /= 1024;                   /* / 1024 for 10bit ADC */

  /* de-sample to get average voltage */
  Value /= Cfg.Samples;
  U = (uint16_t)Value;

// todo: do we need a sanity check for U <= Vcc?

  return U; 
}



/* ************************************************************************
 *   convenience functions
 * ************************************************************************ */


/*
 *  wait 5ms and then read ADC
 *  - same as ReadU()
 */

uint16_t ReadU_5ms(uint8_t Channel)
{
   wait5ms();       /* wait 5ms */

   return (ReadU(Channel));
}



/*
 *  wait 20ms and then read ADC
 *  - same as ReadU()
 */

uint16_t ReadU_20ms(uint8_t Channel)
{
  wait20ms();       /* wait 20ms */

  return (ReadU(Channel));
}



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */


/* source management */
#undef ADC_C



/* ************************************************************************
 *   EOF
 * ************************************************************************ */
