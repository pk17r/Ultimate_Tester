// Controlling a HD44780 compatible LCD in 4-bit interface mode
// The pinout is adjustable via defines in lcd-routines.h
 
#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include "lcd.h"

// sends a data byte to the LCD
void lcd_data(unsigned char temp1){
   LCD_PORT |= (1<<LCD_RS); // set RS to 1
   lcd_send(temp1);
}
 
// send a command to the LCD
void lcd_command(unsigned char temp1){
   LCD_PORT &= ~(1<<LCD_RS);        // Set RS to 0
   lcd_send(temp1);
}

//actual LCD access function; 4-bit mode
void lcd_send(unsigned char data) {
   // set upper nibble
  LCD_PORT = (LCD_PORT & 0xF0) | ((data >> 4) & 0x0F);
  _delay_us(5);
  lcd_enable();
   // set lower nibble
  LCD_PORT = (LCD_PORT & 0xF0) | (data & 0x0F);
  _delay_us(5);
  lcd_enable();
  _delay_us(60);  
  LCD_PORT &= 0xF0;
}

// generates the enable pulse
void lcd_enable(void){
	LCD_PORT |= (1<<LCD_EN1);
    _delay_us(10); // short pause.
   // If problems occur, extend the pause according to the LCD controller's datasheet
   LCD_PORT &= ~(1<<LCD_EN1);
}
 
// Initialization: 
void lcd_init(void){
	LCD_DDR = LCD_DDR | 0x0F | (1<<LCD_RS) | (1<<LCD_EN1);   // Port auf Ausgang schalten
	// must be sent 3 times in a row for initialization
	_delay_ms(30);
	LCD_PORT = (LCD_PORT & 0xF0 & ~(1<<LCD_RS)) | 0x03;
	lcd_enable();

	_delay_ms(5);
	lcd_enable();

	_delay_ms(1);
	lcd_enable();
	_delay_ms(1);
	LCD_PORT = (LCD_PORT & 0xF0 & ~(1<<LCD_RS)) | 0x02;
	_delay_ms(1);
	lcd_enable();
	_delay_ms(1);

	// 4Bit / 2 lines / 5x7
	lcd_command(CMD_SetIFOptions | 0x08);

	// display on / cursor off / no blinking
	lcd_command(CMD_SetDisplayAndCursor | 0x04);

	// increment / no scrolling    
	lcd_command(CMD_SetEntryMode | 0x02);	
	lcd_clear();
}
 
// Sends the command to clear the display. 
void lcd_clear(void){
   lcd_command(CLEAR_DISPLAY);
   _delay_ms(5);
}
 
 
// Writes a string to the LCD 
void lcd_string(char *data){
    while(*data) lcd_data(*data++);
}

//Load string from EEPROM and send to LCD
void lcd_eep_string(const unsigned char *data){	
	unsigned char c;
    while(1) {
		c = eeprom_read_byte(data++);
		if(c==0) return;
        lcd_data(c);
    }
}
