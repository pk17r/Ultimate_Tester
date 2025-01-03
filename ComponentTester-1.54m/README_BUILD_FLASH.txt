Steps to build using WinAVR Programmer's Notepad:
- Open WinAVR Programmer's Notepad
- Open Makefile
- Run Clean All
- Run Make All


Steps to flash using AVRDUDESS:
- Use Arduino ISP Microcontroller to program new MCU
- Attach ICSP and Reset Pins to new MCU
- Set AVRDUDESS:
	- Programmer: Arduino
	- Baud Rate: 19200
	- Bit clock: <empty>
- Under MCU Section, click 'Detect' - 1E9705 should come up (ATmega1284) or 1E9706 (ATmega1284p)
- Set Fuses
- Your new MCU is ready to be flashed!


ATmega1284p Fuses
L 0xF7
H 0xDE
E 0xFD
LB 0xFF


Flash Component Tester:
- Under Flash Section:
	- Select ComponentTester.hex file
	- Select 'Write'
	- Format: Auto
- Under EEPROM Section:
	- Select ComponentTester.eep file
	- Select 'Write'
	- Format: Auto
- Under Options Section:
	- Erase flash and EEPROM
	- Verbosity: 2
    
- Click Program!