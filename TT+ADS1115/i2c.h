#ifndef _I2CMASTER_H
#define _I2CMASTER_H   1

#include <avr/io.h>
#include <compat/twi.h>  // needed for I2C
#include <inttypes.h>

#define I2C_READ    1
#define I2C_WRITE   0
#define SCL_CLOCK  100000L  // I2C clock in Hz

void i2c_init(void);
void i2c_stop(void);
unsigned char i2c_start(unsigned char addr);
unsigned char i2c_rep_start(unsigned char addr);
void i2c_start_wait(unsigned char addr);
unsigned char i2c_write(unsigned char data);
unsigned char i2c_readAck(void);
unsigned char i2c_readNak(void);

#endif
