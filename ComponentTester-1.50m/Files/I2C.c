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


/* pull down SDA (SDA low) */
#define BIT_BANG_SDA_LOW      (I2C_DDR |= (1 << I2C_SDA))

/* pull down SCL (SCL low) */
#define BIT_BANG_SCL_LOW      (I2C_DDR |= (1 << I2C_SCL))

/* release SDA (SDA high) */
/* set to HiZ (disable pull-down) */
#define BIT_BANG_SDA_HIGH_Z   (I2C_DDR &= ~(1 << I2C_SDA))

/* release SCL (SCL high) */
/* set to HiZ (disable pull-down) */
#define BIT_BANG_SCL_HIGH_Z   (I2C_DDR &= ~(1 << I2C_SCL))

/* filter SDA pin */
/* value is 0 for Low and >0 for High */
#define BIT_BANG_READ_SDA     (I2C_PIN & (1 << I2C_SDA))

/* filter SCL pin */
/* value is 0 for Low and >0 for High */
#define BIT_BANG_READ_SCL     (I2C_PIN & (1 << I2C_SCL))

/* I2C Clock Half Time */
/* Data on the I2C-bus can be transferred at rates of;
 *   - up to 100 kbit/s in the Standard-mode = 1 bit in 10µs = 5µs half cycle delay
 *   - up to 400 kbit/s in the Fast-mode = 1 bit in 2.5µs = 1.25µs half cycle delay  */
#ifdef I2C_FAST_MODE
  /* fast mode: 1µs */
  #define BIT_BANG_HALF_DELAY      wait1us()
  #define BIT_BANG_QUARTER_DELAY      wait500ns()
#else
  /* standard mode: 4µs */
  #define BIT_BANG_HALF_DELAY      wait4us()
  #define BIT_BANG_QUARTER_DELAY      wait1us()
#endif


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
 *  create stop condition
 */

void I2C_Stop(void)
{
  // A LOW to HIGH transition on the SDA line while SCL is HIGH defines a STOP condition.

  /*
   *  expected state:
   *  - SDA undefined
   *  - SCL low
   */

  BIT_BANG_QUARTER_DELAY;

  // check and set I2C state
  uint8_t n = I2C.Timeout;         /* get timeout */
  if (n == 0)              /* prevent zero timeout */
  {
    n = 1;
  }
  n *= 5;
  while(((BIT_BANG_READ_SDA == 1) || (BIT_BANG_READ_SCL == 1)) && (n > 0)) {

    // set I2C lines
    BIT_BANG_SDA_LOW;
    BIT_BANG_SCL_LOW;

    BIT_BANG_HALF_DELAY;

    n--;
  }

  // create stop condition

  BIT_BANG_SCL_HIGH_Z;

  BIT_BANG_HALF_DELAY;

  BIT_BANG_SDA_HIGH_Z;

  BIT_BANG_HALF_DELAY;

  /*
   *  current state:
   *  - SDA high
   *  - SCL high
   */
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
    if((BIT_BANG_READ_SDA == 0) || (BIT_BANG_READ_SCL == 0))
      I2C_Stop();

    /* follow common start condition */
  }
  else                        /* repeated start */
  {
    /*
     *  expected state:
     *  - SDA undefined
     *  - SCL low
     */

    /*  desired state:
     * - SDA high 
     * - SCL high
     */

    // set I2C desired state
    uint8_t n = I2C.Timeout;         /* get timeout */
    if (n == 0)              /* prevent zero timeout */
    {
      n = 1;
    }
    n *= 5;
    while(((BIT_BANG_READ_SDA == 0) || (BIT_BANG_READ_SCL == 0)) && (n > 0)) {

      // release (high) I2C lines
      BIT_BANG_SDA_HIGH_Z;
      BIT_BANG_SCL_HIGH_Z;

      BIT_BANG_HALF_DELAY;

      n--;
      if(n == 0)
        return I2C_ERROR;
    }

    /* follow common start condition */
  }

  /* current state:
   * - SDA high 
   * - SCL high
   */

  // issue start condition

  // A HIGH to LOW transition on the SDA line while SCL is HIGH defines a START condition

  /* SCL has to stay low and SDA high for t_HD;STA */
  BIT_BANG_HALF_DELAY;

  BIT_BANG_SDA_LOW;

  /* SCL will follow after t_HD;STA */
  BIT_BANG_HALF_DELAY;

  BIT_BANG_SCL_LOW;

  /* SCL will stay low for t_HD;STA */
  BIT_BANG_HALF_DELAY;

  /* current state:
   * - SDA low 
   * - SCL low
   */

  return Flag;
}



/*
 *  check SCL is not controlled by slave
 *  3.1.9 Clock stretching    https://www.nxp.com/docs/en/user-guide/UM10204.pdf
 *  On the byte level, a device may be able to receive bytes of data at a fast rate, but needs
 *  more time to store a received byte or prepare another byte to be transmitted. Targets can
 *  then hold the SCL line LOW after reception and acknowledgment of a byte to force the
 *  controller into a wait state until the target is ready for the next byte transfer in a type of
 *  handshake procedure.
 *
 *  when to check:
 *  - should be checked after Master releases SCL and quarter cycle has passed
 *
 *  returns:
 *  - I2C_ERROR for bus error
 *  - I2C_OK for bus okay
 */

uint8_t I2C_Check_Slave_SCL_Control(void)
{
  // check SCL is not controlled by slave
  uint8_t n = I2C.Timeout;         /* get timeout */
  if (n == 0)              /* prevent zero timeout */
  {
    n = 1;
  }
  n *= 5;
  while((BIT_BANG_READ_SCL == 0) && (n > 0)) {

    BIT_BANG_QUARTER_DELAY;

    n--;
    if(n == 0)
      return I2C_ERROR;
  }
  return I2C_OK;
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

uint8_t I2C_WriteByte(uint8_t Type)
{
  uint8_t           Flag = I2C_NACK;   /* return value */
  uint8_t           Byte;               /* byte */

  /*
   *  expected state:
   *  - SDA undefined
   *  - SCL low
   */

  /*
   *  send byte
   */

  Byte = I2C.Byte;            /* get byte */

  /* bit-bang 8 bits */
  uint8_t n = 8;
  while (n > 0)               /* 8 bits */
  {
    /*
     *  set SDA
     */

    /* get current MSB and set SDA */
    if(Byte & 0x80)
      BIT_BANG_SDA_HIGH_Z;
    else
      BIT_BANG_SDA_LOW;

    BIT_BANG_QUARTER_DELAY;

    /*
     *  run SCL cycle
     */
    BIT_BANG_SCL_HIGH_Z;

    BIT_BANG_QUARTER_DELAY;

    // check SCL is not controlled by slave
    if(I2C_Check_Slave_SCL_Control() == I2C_ERROR)
    {
      I2C_Stop();
      return I2C_ERROR;
    }

    BIT_BANG_QUARTER_DELAY;

    BIT_BANG_SCL_LOW;

    BIT_BANG_QUARTER_DELAY;

    // move to next bit
    Byte <<= 1;
    n--;                      /* next bit */
  }

  // ACK bit

  // release SDA
  BIT_BANG_SDA_HIGH_Z;

  BIT_BANG_QUARTER_DELAY;

  /*
   *  run SCL cycle
   */
  BIT_BANG_SCL_HIGH_Z;

  BIT_BANG_HALF_DELAY;

  // wait until ACK bit is received from slave
  n = I2C.Timeout;         /* get timeout */
  if (n == 0)              /* prevent zero timeout */
  {
    n = 1;
  }
  n *= 5;
  while(n > 0)
  {
    if(BIT_BANG_READ_SDA == 0)
    {
      // ACK received
      Flag = I2C_ACK;
      break;
    }
    BIT_BANG_QUARTER_DELAY;
    n--;
  }

  BIT_BANG_SCL_LOW;

  BIT_BANG_HALF_DELAY;

  return Flag;
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

uint8_t I2C_ReadByte(uint8_t Type)
{
  uint8_t           Byte = 0;               /* byte */
  uint8_t           i = 8;                  /* counter */

  /*
   *  expected state:
   *  - SDA undefined
   *  - SCL low
   */

  BIT_BANG_QUARTER_DELAY;

  /*
   *  read byte
   */

  while (i > 0)
  {
    /*
     *  run SCL cycle
     */

    BIT_BANG_SCL_HIGH_Z;

    BIT_BANG_QUARTER_DELAY;

    // check SCL is not controlled by slave
    if(I2C_Check_Slave_SCL_Control() == I2C_ERROR)
    {
      I2C_Stop();
      return I2C_ERROR;
    }

    BIT_BANG_QUARTER_DELAY;

    // read bit info
    if(BIT_BANG_READ_SDA)
      Byte |= 1;

    i--;    // decrease counter

    // shift only first 7 read bits
    if(i > 0)
      Byte <<= 1;

    // low clock
    BIT_BANG_SCL_LOW;

    BIT_BANG_HALF_DELAY;
  }

  // save read byte
  I2C.Byte = Byte;

  // send ACK bit if asked by user
  if(Type == I2C_ACK)
  {
    // check if SDA is released by slave
    i = 10;
    while((BIT_BANG_READ_SDA == 0) && (i > 0))
    {
      BIT_BANG_HALF_DELAY;

      i--;
      if(i == 0)
        return I2C_ERROR;
    }

    // send acknowledgement bit
    BIT_BANG_SDA_LOW;
    BIT_BANG_SCL_HIGH_Z;

    BIT_BANG_HALF_DELAY;
  }

  // prepare I2C Lines for next cycle
  BIT_BANG_SCL_LOW;
  BIT_BANG_SDA_HIGH_Z;

  BIT_BANG_HALF_DELAY;

  return I2C_OK;
}


#endif



#undef BIT_BANG_SDA_LOW
#undef BIT_BANG_SCL_LOW
#undef BIT_BANG_SDA_HIGH_Z
#undef BIT_BANG_SCL_HIGH_Z
#undef BIT_BANG_READ_SDA
#undef BIT_BANG_READ_SCL
#undef BIT_BANG_HALF_DELAY
#undef BIT_BANG_QUARTER_DELAY

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
