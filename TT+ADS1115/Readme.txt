A stripped down version of the ORIGINAL Markus Frejek's code.
ATTENTION : this is NOT a fully working code.

Uses ADS1115 as ADC instead of internal one.
tested in PROTEUS OKK

* some measurement are omitted:
	- resistance. 
	- capacitance.
	- base-emitter voltage for bipolar transistors. 
	- gate capacitance for enhancement MOSFETS. 

* No automatic shut OFF
* No custom chars.
* No battery test.
* The watchdog is not used to reset the chip when the program hangs.

The file tt.hex is the Flash. 
The file eep.bin is the EEPROM contents. Please do not forget this in simulation and real device !

ATMEGA328 @ 8MHz (internal RC oscillator).


