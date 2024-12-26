// Controlling an HD44780 compatible LCD in 4-bit interface mode

// Functions
void lcd_data(unsigned char temp1);
void lcd_command(unsigned char temp1);
void lcd_send(unsigned char data);
void lcd_string(char *data);
void lcd_enable(void);
void lcd_init(void);
void lcd_clear(void);
void lcd_eep_string(const unsigned char *data);

//LCD commands
#define CMD_SetEntryMode         0x04
#define CMD_SetDisplayAndCursor  0x08
#define CMD_SetIFOptions         0x20
#define CMD_SetDDRAMAddress 	0x80 	// to set cursor
#define CLEAR_DISPLAY 			0x01

//Macros for LCD
#define SetCursor(y, x) lcd_command((uint8_t)(CMD_SetDDRAMAddress + (0x40*(y-1)) + x)) //Jump to a specific position
#define Line1() SetCursor(1,0) //Jump to the beginning of the 1st line
#define Line2() SetCursor(2,0) //Jump to the beginning of the 2nd line
   
// pinout for the LCD, match to used pins
#define LCD_PORT      PORTD
#define LCD_DDR       DDRD
#define LCD_RS        PD4
#define LCD_EN1       PD5
