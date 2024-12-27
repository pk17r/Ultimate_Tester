/* ************************************************************************
 *
 *   I2C (bit-bang & hardware TWI)
 *
 *   (c) 2017-2024 by Markus Reschke
 *
 * ************************************************************************ */

/*
 *  hints:
 *  - port and pins for bit-bang I2C
 *    I2C_PORT      port data register
 *    I2C_DDR       port data direction register
 *    I2C_PIN       port input pins register
 *    I2C_SDA       pin for SDA
 *    I2C_SCL       pin for SCL
 *  - For hardware I2C (TWI) the MCU specific pins are used:
 *    ATmega 328: SDA PC4 / SCL PC5 
 *    ATmega 644: SDA PC1 / SCL PC0
 *  - bus speed modes
 *    I2C_STANDARD_MODE  100kHz
 *    I2C_FAST_MODE      400kHz
 *  - Don't forget the pull up resistors for SDA and SCL!
 *    Usually 2-10kOhms for 5V.
 */


/* local includes */
#include "config.h"           /* global configuration */

#ifdef HW_I2C


/*
 *  local constants
 */

/* source management */
#define I2C_C


/*
 *  include header files
 */

/* local includes */
#include "common.h"           /* common header file */
#include "variables.h"        /* global variables */
#include "functions.h"        /* external functions */



/*
 *  bus speed modes:
 *  - standard mode      up to 100kbit/s
 *  - fast mode          up to 400kbit/s
 *  - fast mode plus     up to 1Mbit/s      
 *  - high-speed mode
 *    400pF load         up to 1.7Mbit/s
 *    100pF load         up to 3.4Mbit/s
 *
 *  address modes:
 *  - 7 bit + R/W   ->   address byte
 *    R/W (1/0)          bit 0
 *    bit 0 (A0)         bit 1
 *    bit 1 (A1)         bit 2
 *    bit 2 (A2)         bit 3
 *    bits 3-6           bits 4-7
 *  - 10 bit + R/W  ->   2 address bytes
 *    R/W (1/0)          MSB bit 0
 *    bits 0-7           LSB bits 0-7
 *    bits 8-9           MSB bits 1-2
 *    11110              MSB bits 3-7
 */



/* ************************************************************************
 *   functions for bit-bang I2C
 * ************************************************************************ */


#ifdef I2C_BITBANG



/*
 *  set up SDA and SCL lines
 *
 *  returns:
 *  - I2C_ERROR on bus error
 *  - I2C_OK on success
 */

uint8_t I2C_Setup(void)
{
  uint8_t           Flag = I2C_ERROR;   /* return value */
  uint8_t           Bits;               /* register bits */

  /* default is SDA and SCL pulled up by external resistors */
  /* set both pins to HiZ input mode */
  I2C_DDR &= ~((1 << I2C_SDA) | (1 << I2C_SCL));

  /* preset pins to 0 */
  I2C_PORT &= ~((1 << I2C_SDA) | (1 << I2C_SCL));

  /* check if SDA and SCL are pulled up externally */
  Bits = I2C_PIN;                  /* get state */
  Bits &= (1 << I2C_SDA) | (1 << I2C_SCL);        /* filter lines */

  if (Bits == ((1 << I2C_SDA) | (1 << I2C_SCL)))  /* lines are high */
  {
    Flag = I2C_OK;       /* signal success */
  }

  /*
   *  current state:
   *  - SDA high
   *  - SCL high
   */

  return Flag;
}



/*
 *  create start condition
 *  - Type
 *    I2C_START for start condition
 *    I2C_REPEATED_START for repeated start condition (not enabled yet)
 *
 *  returns:
 *  - I2C_ERROR on bus error
 *  - I2C_OK on success
 */

uint8_t I2C_Start(uint8_t Type)
{
  uint8_t           Flag = I2C_OK;      /* return value */

  if (Type == I2C_START)      /* start */
  {
    /*
     *  expected state:
     *  - SDA high
     *  - SCL high
     */

    // check I2C state
    uint8_t i = 10;
    while((((I2C_PIN & (1 << I2C_SCL)) == 0) || ((I2C_PIN & (1 << I2C_SDA)) == 0)) && (i > 0)) {
      // high I2C lines

      /* release SDA (SDA high) */
      I2C_DDR &= ~(1 << I2C_SDA);     /* set to HiZ (disable pull-down) */

      /* release SCL (SCL high) */
      I2C_DDR &= ~(1 << I2C_SCL);     /* set to HiZ (disable pull-down) */

      /* I2C Clock Half Time */
      #ifdef I2C_FAST_MODE
        /* fast mode: min. 0.6us */
        wait1us();
      #else
        /* standard mode: min. 4us */
        wait4us();
      #endif

      i--;

      if(i == 0)
        return I2C_ERROR;
    }

    I2C_DDR |= (1 << I2C_SDA);    /* pull down SDA (SDA low) */

    /* SCL will follow after t_HD;STA */

    /* I2C Clock Half Time */
    #ifdef I2C_FAST_MODE
      /* fast mode: min. 0.6us */
      wait1us();
    #else
      /* standard mode: min. 4us */
      wait4us();
    #endif

    I2C_DDR |= (1 << I2C_SCL);    /* pull down SCL (SCL low) */

    /*
     *  current state:
     *  - SDA low
     *  - SCL low
     */
  }
#if 0
  else                        /* repeated start */
  {
    /*
     *  expected state:
     *  - SDA undefined
     *  - SCL low
     */

    /* release SDA (SDA high) */
    I2C_DDR &= ~(1 << I2C_SDA);    /* set to HiZ (disable pull-down) */

    /* SCL has to stay low for t_LOW */
    #ifdef I2C_FAST_MODE
      /* fast mode: min. 1.3µs */
      wait2us();
    #else
      /* standard mode: min. 4.7µs */
      wait5us();
    #endif

    /* release SCL (SCL high) */
    I2C_DDR &= ~(1 << I2C_SCL);    /* set to HiZ (disable pull-down) */

    /* SDA has to stay high for t_SU;STA after SCL rises */
    #ifdef I2C_FAST_MODE
      /* fast mode: min. 0.6µs */
      wait1us();
    #else
      /* standard mode: min. 4.7µs */
      wait5us();
    #endif

    /* follow common start condition */
    /* ... code from above (I2C_START) ... */

    /*
     *  current state:
     *  - SDA low
     *  - SCL low
     */
  }
#endif

  /* current state:
   * - SDA low 
   * - SCL low
   */

  return Flag;
}



/*
 *  write byte (master to slave)
 *  - send byte (address or data) to slave
 *    bit-bang 8 bits, MSB first, LSB last
 *  - get ACK/NACK from slave
 *  - byte to be sent is taken from global I2C.Byte
 *
 *  requires:
 *  - Type
 *    I2C_DATA for data byte
 *    I2C_ADDRESS for address byte
 *
 *  returns:
 *  - I2C_ERROR for bus error
 *  - I2C_ACK for ACK
 *  - I2C_NACK NACK
 */

uint8_t I2C_WriteByte(uint8_t Type) {

  // low I2C lines
  I2C_DDR |= (1 << I2C_SDA);    /* pull down SDA (SDA low) */
  I2C_DDR |= (1 << I2C_SCL);    /* pull down SCL (SCL low) */

  /* I2C Clock Half Time */
  #ifdef I2C_FAST_MODE
    /* fast mode: min. 0.6us */
    wait1us();
  #else
    /* standard mode: min. 4us */
    wait4us();
  #endif

  // check I2C state
  uint8_t i = 10;
  while((((I2C_PIN & (1 << I2C_SCL)) == 1) || ((I2C_PIN & (1 << I2C_SDA)) == 1)) && (i > 0)) {

    // low I2C lines
    I2C_DDR |= (1 << I2C_SDA);    /* pull down SDA (SDA low) */
    I2C_DDR |= (1 << I2C_SCL);    /* pull down SCL (SCL low) */

    /* I2C Clock Half Time */
    #ifdef I2C_FAST_MODE
      /* fast mode: min. 0.6us */
      wait1us();
    #else
      /* standard mode: min. 4us */
      wait4us();
    #endif

    i--;

    if(i == 0)
      return I2C_ERROR;
  }

  // write byte
  for (i=0; i<8; i++) {

    // clock high, share bit info
    if(I2C.Byte & 0x80) {
      /* release SDA (SDA high) */
      I2C_DDR &= ~(1 << I2C_SDA);     /* set to HiZ (disable pull-down) */
    }
    else {
      I2C_DDR |= (1 << I2C_SDA);    /* pull down SDA (SDA low) */
    }

    /* release SCL (SCL high) */
    I2C_DDR &= ~(1 << I2C_SCL);     /* set to HiZ (disable pull-down) */

    /* I2C Clock Half Time */
    #ifdef I2C_FAST_MODE
      /* fast mode: min. 0.6us */
      wait1us();
    #else
      /* standard mode: min. 4us */
      wait4us();
    #endif

    // clock low
    I2C_DDR |= (1 << I2C_SCL);    /* pull down SCL (SCL low) */

    I2C.Byte <<= 1;

    /* I2C Clock Half Time */
    #ifdef I2C_FAST_MODE
      /* fast mode: min. 0.6us */
      wait1us();
    #else
      /* standard mode: min. 4us */
      wait4us();
    #endif
  }

  // read ACK bit

  /* release SDA (SDA high) */
  I2C_DDR &= ~(1 << I2C_SDA);     /* set to HiZ (disable pull-down) */

  /* release SCL (SCL high) */
  I2C_DDR &= ~(1 << I2C_SCL);     /* set to HiZ (disable pull-down) */

  /* I2C Clock Half Time */
  #ifdef I2C_FAST_MODE
    /* fast mode: min. 0.6us */
    wait1us();
  #else
    /* standard mode: min. 4us */
    wait4us();
  #endif

  uint8_t sda_value = (I2C_PIN & (1 << I2C_SDA));    /* filter SDA pin */

  I2C_DDR |= (1 << I2C_SCL);    /* pull down SCL (SCL low) */

  if(sda_value == 0)
    return I2C_ACK;
  else
    return I2C_NACK;
}



#ifdef I2C_RW

/*
 *  read byte (master from slave)
 *  - read byte (data) from slave
 *    bit-bang 8 bits, MSB first, LSB last
 *  - ACK/NACK to slave
 *  - byte to be read is stored in global I2C.Byte
 *
 *  requires:
 *  - Type
 *    I2C_ACK for sending acknowledge
 *    I2C_NACK for sending not-acknowledge
 *
 *  returns:
 *  - I2C_ERROR on bus error
 *  - I2C_OK on success
 */

uint8_t I2C_ReadByte(uint8_t Type) {

  // release data line

  /* release SDA (SDA high) */
  I2C_DDR &= ~(1 << I2C_SDA);     /* set to HiZ (disable pull-down) */

  // make clock low
  I2C_DDR |= (1 << I2C_SCL);    /* pull down SCL (SCL low) */

  /* I2C Clock Half Time */
  #ifdef I2C_FAST_MODE
    /* fast mode: min. 0.6us */
    wait1us();
  #else
    /* standard mode: min. 4us */
    wait4us();
  #endif

  // check whether SCL is controlled by slave
  uint8_t i = 10;
  while(((I2C_PIN & (1 << I2C_SCL)) == 1) && (i > 0)) {

    /* I2C Clock Half Time */
    #ifdef I2C_FAST_MODE
      /* fast mode: min. 0.6us */
      wait1us();
    #else
      /* standard mode: min. 4us */
      wait4us();
    #endif

    i--;

    if(i == 0)
      return I2C_ERROR;
  }

  // read byte
  I2C.Byte = 0;

  for (i=0; i<8; i++) {

    // clock high cycle half, read bit info
    /* release SCL (SCL high) */
    I2C_DDR &= ~(1 << I2C_SCL);     /* set to HiZ (disable pull-down) */

    /* I2C Clock Half Time */
    #ifdef I2C_FAST_MODE
      /* fast mode: min. 0.6us */
      wait1us();
    #else
      /* standard mode: min. 4us */
      wait4us();
    #endif

    if((I2C_PIN & (1 << I2C_SDA)))    /* filter SDA pin */
      I2C.Byte |= 1;

    if(i < 7)
      I2C.Byte <<= 1;

    // clock low cycle half
    I2C_DDR |= (1 << I2C_SCL);    /* pull down SCL (SCL low) */

    /* I2C Clock Half Time */
    #ifdef I2C_FAST_MODE
      /* fast mode: min. 0.6us */
      wait1us();
    #else
      /* standard mode: min. 4us */
      wait4us();
    #endif
  }

  if(Type == I2C_ACK) {

    // send ACK bit

    // check if SDA is released by slave
    i = 10;
    /* filter SDA pin */
    while(((I2C_PIN & (1 << I2C_SDA)) == 0) && (i > 0)) {
      /* I2C Clock Half Time */
      #ifdef I2C_FAST_MODE
        /* fast mode: min. 0.6us */
        wait1us();
      #else
        /* standard mode: min. 4us */
        wait4us();
      #endif

      i--;
      if(i == 0)
        return I2C_ERROR;
    }

    // send acknowledgement bit
    I2C_DDR |= (1 << I2C_SDA);    /* pull down SDA (SDA low) */

    /* release SCL (SCL high) */
    I2C_DDR &= ~(1 << I2C_SCL);     /* set to HiZ (disable pull-down) */

    /* I2C Clock Half Time */
    #ifdef I2C_FAST_MODE
      /* fast mode: min. 0.6us */
      wait1us();
    #else
      /* standard mode: min. 4us */
      wait4us();
    #endif
  }

  // prepare I2C Lines for next cycle
  I2C_DDR |= (1 << I2C_SCL);    /* pull down SCL (SCL low) */

  /* release SDA (SDA high) */
  I2C_DDR &= ~(1 << I2C_SDA);     /* set to HiZ (disable pull-down) */

  /* I2C Clock Half Time */
  #ifdef I2C_FAST_MODE
    /* fast mode: min. 0.6us */
    wait1us();
  #else
    /* standard mode: min. 4us */
    wait4us();
  #endif

  return I2C_OK;
}


#endif



/*
 *  create stop condition
 */

void I2C_Stop(void)
{
  /*
   *  expected state:
   *  - SDA undefined
   *  - SCL low
   */


  // high Z I2C lines

  /* release SDA (SDA high) */
  I2C_DDR &= ~(1 << I2C_SDA);     /* set to HiZ (disable pull-down) */

  /* release SCL (SCL high) */
  I2C_DDR &= ~(1 << I2C_SCL);     /* set to HiZ (disable pull-down) */

  // check I2C state
  uint8_t i = 10;
  while((((I2C_PIN & (1 << I2C_SCL)) == 0) || ((I2C_PIN & (1 << I2C_SDA)) == 0)) && (i > 0)) {

    // high Z I2C lines

    /* release SDA (SDA high) */
    I2C_DDR &= ~(1 << I2C_SDA);     /* set to HiZ (disable pull-down) */

    /* release SCL (SCL high) */
    I2C_DDR &= ~(1 << I2C_SCL);     /* set to HiZ (disable pull-down) */

    /* I2C Clock Half Time */
    #ifdef I2C_FAST_MODE
      /* fast mode: min. 0.6us */
      wait1us();
    #else
      /* standard mode: min. 4us */
      wait4us();
    #endif

    i--;
  }


  // create stop condition

  I2C_DDR |= (1 << I2C_SDA);    /* pull down SDA (SDA low) */

  /* I2C Clock Half Time */
  #ifdef I2C_FAST_MODE
    /* fast mode: min. 0.6us */
    wait1us();
  #else
    /* standard mode: min. 4us */
    wait4us();
  #endif

  /* release SCL (SCL high) */
  I2C_DDR &= ~(1 << I2C_SCL);     /* set to HiZ (disable pull-down) */

  /* I2C Clock Half Time */
  #ifdef I2C_FAST_MODE
    /* fast mode: min. 0.6us */
    wait1us();
  #else
    /* standard mode: min. 4us */
    wait4us();
  #endif

  /* release SDA (SDA high) */
  I2C_DDR &= ~(1 << I2C_SDA);     /* set to HiZ (disable pull-down) */

  /* I2C Clock Half Time */
  #ifdef I2C_FAST_MODE
    /* fast mode: min. 0.6us */
    wait1us();
  #else
    /* standard mode: min. 4us */
    wait4us();
  #endif

  /*
   *  current state:
   *  - SDA high
   *  - SCL high
   */
}

#endif



/* ************************************************************************
 *   functions for hardware TWI
 * ************************************************************************ */


#ifdef I2C_HARDWARE

/*
 *  set up TWI
 *
 *  returns:
 *  - I2C_ERROR on bus error
 *  - I2C_OK on success
 */

uint8_t I2C_Setup(void)
{
  uint8_t           Flag = I2C_OK;      /* return value */

  /*
   *  set bus speed:
   *  - SCL clock = MCU clock / (16 + 2*TWBR * prescaler)
   *  - available prescalers: 1, 4, 16 & 64
   *  - TWBR register: 1-255
   */

  /* set prescaler to 1 */
  TWSR = 0;           /* TWPS1=0 / TWPS0=0 */

  #ifdef I2C_FAST_MODE
    /* 400kHz */
    TWBR = CPU_FREQ / 400000;
  #else
    /* 100kHz */
    TWBR = CPU_FREQ / 100000;
  #endif

  return Flag;
}



/*
 *  create start condition
 *
 *  requires:
 *  - Type
 *    I2C_START for start condition
 *    I2C_REPEATED_START for repeated start condition
 *
 *  returns:
 *  - I2C_ERROR on bus error
 *  - I2C_OK on success
 */

uint8_t I2C_Start(uint8_t Type)
{
  uint8_t           Flag = I2C_ERROR;   /* return value */
  uint8_t           Bits;               /* bits */

  /*
   *  MCU's state maschine for TWI:
   *  - Start for new communication
   *  - Repeated Start after Start (no Stop yet)
   */

  /* set expected status flag */
  if (Type == I2C_START)      /* start */
  {
    Type = (1 << TWS3);       /* 0x08 */
  }
  else                        /* repeated start */
  {
    Type = (1 << TWS4);       /* 0x10 */
  }

  /*
   *  (repeated) start condition:
   *  - set TWINT to clear interrupt flag
   *  - set TWEN to enable TWI
   *  - set TWSTA for "start"
   */

  TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTA);

  /* wait for job done */
  while (!(TWCR & (1 << TWINT)));       /* wait for flag */

  /*
   *  check result
   */

  Bits = TWSR;                /* get status */
  /* filter status bits */
  Bits &= (1 << TWS7) | (1 << TWS6) | (1 << TWS5) | (1 << TWS4) | (1 << TWS3);

  if (Bits == Type)           /* got expected flag */
  {
    Flag = I2C_OK;            /* signal success */
  }

  return Flag;
}



/*
 *  write byte (master to slave)
 *  - send byte (address or data) to slave
 *  - get ACK/NACK from slave
 *  - byte to be sent is taken from global I2C.Byte
 *
 *  requires:
 *  - Type
 *    I2C_DATA for data byte
 *    I2C_ADDRESS for address byte
 *
 *  returns:
 *  - I2C_ERROR for bus error
 *  - I2C_ACK for ACK
 *  - I2C_NACK NACK
 */

uint8_t I2C_WriteByte(uint8_t Type)
{
  uint8_t           Flag = I2C_ERROR;   /* return value */
  uint8_t           Bits;               /* register bits */
  uint8_t           AckBits;            /* register bits for ACK */
  uint8_t           NackBits;           /* register bits for NACK */

  /*
   *  MCU's state maschine for TWI:
   *  - after Start the slave has to be addressed
   *  - after addressing the slave (write) data can be sent
   */


  Bits = I2C.Byte;            /* get byte */

  /* set status register bits */
  if (Type == I2C_DATA)            /* data byte */
  {
    AckBits = (1 << TWS5) | (1 << TWS3);     /* 0x28 data & ACK */
    NackBits = (1 << TWS5) | (1 << TWS4);    /* 0x30 data & NACK */
  }
  else                             /* address byte */
  {
    if (Bits & 0b00000001)         /* read mode (bit #0 = 1) */
    {
      AckBits = (1 << TWS6);                 /* 0x40 SLA+R & ACK */
      NackBits = (1 << TWS6) | (1 << TWS3);  /* 0x48 SLA+R & NACK */
    }
    else                           /* write mode (bit #0 = 0) */
    {
      AckBits = (1 << TWS4) | (1 << TWS3);   /* 0x18 SLA+W & ACK */
      NackBits = (1 << TWS5);                /* 0x20 SLA+W & NACK */
    }
  }

  /* load byte into data register */
  TWDR = Bits;

  /* send by clearing TWINT */
  TWCR = (1 << TWINT) | (1 << TWEN);

  /* wait for job done */
  while (!(TWCR & (1 << TWINT)));       /* wait for flag */

  /* check result */
  Bits = TWSR;                /* get status */
  /* filter status bits */
  Bits &= (1 << TWS7) | (1 << TWS6) | (1 << TWS5) | (1 << TWS4) | (1 << TWS3);

  if (Bits == AckBits)        /* got expected bits for ACK */
  {
    Flag = I2C_ACK;           /* signal ACK */
  }
  else if (Bits == NackBits)  /* got expected bits for NACK */
  {
    Flag = I2C_NACK;          /* signal NACK */
  }
  /* else 0x38: keep default "bus error" */

  return Flag;
}



#ifdef I2C_RW

/*
 *  read byte (master from slave)
 *  - read byte (data) from slave
 *  - ACK/NACK to slave
 *  - byte to be read is stored in global I2C.Byte
 *
 *  requires:
 *  - Type
 *    I2C_ACK for sending acknowledge
 *    I2C_NACK for sending not-acknowledge
 *
 *  returns:
 *  - I2C_ERROR on bus error
 *  - I2C_OK on success
 */

uint8_t I2C_ReadByte(uint8_t Type)
{
  uint8_t           Flag = I2C_ERROR;   /* return value */
  uint8_t           Bits;               /* register bits */
  uint8_t           StatusBits;         /* status register bits */

  /*
   *  MCU's state maschine for TWI:
   *  - after addressing slave (read) data can be read 
   */

  /* receive by clearing TWINT */
  if (Type == I2C_ACK)             /* ACK */
  {
    /* read and respond with ACK */
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);

    StatusBits = (1 << TWS6) | (1 << TWS4);  /* 0x50 data read & ACK */
  }
  else                             /* NACK */
  {
    /* read and respond with NACK */
    TWCR = (1 << TWINT) | (1 << TWEN);

    StatusBits = (1 << TWS6) | (1 << TWS4) | (1 << TWS3);  /* 0x58 data read & NACK */
  }

  /* wait for job done */
  while (!(TWCR & (1 << TWINT)));       /* wait for flag */

  /* check result */
  Bits = TWSR;                /* get status */
  /* filter status bits */
  Bits &= (1 << TWS7) | (1 << TWS6) | (1 << TWS5) | (1 << TWS4) | (1 << TWS3);

  if (Bits == StatusBits)     /* got expected bits */
  {
    I2C.Byte = TWDR;          /* save byte */
    Flag = I2C_OK;            /* signal success */
  }
  /* else 0x38: keep default "bus error" */

  return Flag;
}

#endif



/*
 *  create stop condition
 */

void I2C_Stop(void)
{
  /*
   *  stop condition:
   *  - set TWINT to clear interrupt flag
   *  - set TWEN to enable TWI
   *  - set TWSTO for "stop"
   */

  TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);

  /* in this case TWINT won't be set after the job is done */


  /*
   *  bus free time between Stop and next Start (T_BUF)
   *  - standard mode: min. 4.7µs
   *  - fast mode: min. 1.3µs
   */

  #ifdef I2C_FAST_MODE
    /* fast mode: min. 1.3µs */
    wait5us();
  #else
    /* standard mode: min. 4.7µs */
    wait20us();
  #endif
}


#endif



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */

/* source management */
#undef I2C_C

#endif

/* ************************************************************************
 *   EOF
 * ************************************************************************ */
