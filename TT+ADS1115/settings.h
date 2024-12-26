

/*
	Exact values of the used resistors in Ohm.
	The nominal value for R_L is 680 Ohm, for R_H 470kOhm.
	To calibrate the program for deviations from these values (e.g. due to component tolerances)
	enter the resistance values in ohms into the following defines:
*/
#define R_L_VAL 680 			//R_L: Standard value 680 ohms
#define R_H_VAL 470000UL 		//R_H: Standard value 470,000 ohms (UL= unsigned long)

//Calculate the necessary resistance values in the program
#define RH_RL_RATIO (R_H_VAL/R_L_VAL)
#define R_READ_RH (R_H_VAL/100)


/*	
Factors for capacitor capacitance measurement
These factors depend on the AVR's manufacturing tolerances and may therefore need to be adjusted
H_CAPACITY_FACTOR is for measuring with 470k resistor (low capacitance)
L_CAPACITY_FACTOR is for measuring with 680 ohm resistor (high capacitance)
The entire measuring range is approx. 0.2nF to 1000µF.
*/
#define H_CAPACITY_FACTOR 394
#define L_CAPACITY_FACTOR 283

//MCU_STATUS_REG NAME is AVR Type dependent
#define MCU_STATUS_REG MCUCR   // for ATMEGA328

//Strings iN EEPROM
unsigned char TestRunning[] EEMEM = "Test running...";
unsigned char Bat[] EEMEM = "Battery ";
unsigned char BatWeak[] EEMEM = "weak";
unsigned char BatEmpty[] EEMEM = "Empty!";
unsigned char TestFailed1[] EEMEM = "Unknown, Damaged";
unsigned char TestFailed2[] EEMEM = "or Nothing found";
unsigned char Bauteil[] EEMEM = "Device";
unsigned char Unknown[] EEMEM = " Unknown.";
unsigned char Diode[] EEMEM = "Diode: ";
unsigned char DualDiode[] EEMEM = "Double diode";
unsigned char TwoDiodes[] EEMEM = "2 Diodes";
unsigned char Antiparallel[] EEMEM = "antiparallel";
unsigned char InSeries[] EEMEM = "in Serie A=";
unsigned char mosfet[] EEMEM = "-MOS";
unsigned char emode[] EEMEM = "-E";
unsigned char dmode[] EEMEM = "-D";
unsigned char jfet[] EEMEM = "-JFET";
unsigned char Thyristor[] EEMEM = "Thyristor";
unsigned char Triac[] EEMEM = "Triac";
unsigned char A1[] EEMEM = ";A1=";
unsigned char A2[] EEMEM = ";A2=";
unsigned char K1[] EEMEM = ";K1=";
unsigned char K2[] EEMEM = ";K2=";
unsigned char hfestr[] EEMEM ="hFE=";
unsigned char NPN[] EEMEM = "NPN";
unsigned char PNP[] EEMEM = "PNP";
unsigned char bstr[] EEMEM = " B=";
unsigned char cstr[] EEMEM = ";C=";
unsigned char estr[] EEMEM = ";E=";
unsigned char gds[] EEMEM = "GDS=";
unsigned char Uf[] EEMEM = "Uf=";
unsigned char vt[] EEMEM = "Vt=";
unsigned char mV[] EEMEM = "mV";
unsigned char Anode[] EEMEM = "A=";
unsigned char Gate[] EEMEM = "G=";


struct Diode {
	uint8_t Anode;
	uint8_t Cathode;
	int Voltage;
};

// Functions
void CheckPins(uint8_t HighPin, uint8_t LowPin, uint8_t TristatePin);
void DischargePin(uint8_t PinToDischarge, uint8_t DischargeDirection);
unsigned int ReadADC(uint8_t ch);
void GetGateThresholdVoltage(void);

/* Port for the test resistors
The resistors must be connected to the lower 6 pins of the port, in the following order:
RLx = 680R resistor for test pin x
RHx = 470k resistor for test pin x
	
	RL1 on Pin 0
	RH1 on Pin 1
	RL2 on Pin 2
	RH2 on Pin 3
	RL3 on Pin 4
	RH3 on Pin 5
*/
#define R_DDR DDRB
#define R_PORT PORTB

/* 
Port for the test pins
This port must have an ADC (PORTC for the Mega8).
The lower 3 pins of this port must be used for the test pins.
Please do not change the definitions for TP1, TP2 and TP3!
*/
#define ADC_PORT PORTC
#define ADC_DDR DDRC
#define ADC_PIN PINC
#define TP1 PC0
#define TP2 PC1
#define TP3 PC2

// Button
#define RST_PORT PORTD
#define RST_PIN_REG PIND
#define RST_PIN PD7 //Pin which is pulled low when power button is pressed

// Components
#define PART_NONE 0
#define PART_DIODE 1
#define PART_TRANSISTOR 2
#define PART_FET 3
#define PART_TRIAC 4
#define PART_THYRISTOR 5
#define PART_RESISTOR 6

//Special definitions for components
//FETs
#define PART_MODE_N_E_MOS 1
#define PART_MODE_P_E_MOS 2
#define PART_MODE_N_D_MOS 3
#define PART_MODE_P_D_MOS 4
#define PART_MODE_N_JFET 5
#define PART_MODE_P_JFET 6

//Bipolar
#define PART_MODE_NPN 1
#define PART_MODE_PNP 2

struct Diode diodes[6];
uint8_t NumOfDiodes;

uint8_t b,c,e; //Connections of the transistor
unsigned long lhfe; //Gain factor
uint8_t PartReady; //Component recognized as ready (detected)
unsigned int hfe[2]; //Reinforcement factors
unsigned int uBE[2]; //B-E voltage for transistors
uint8_t PartMode;
uint8_t tmpval, tmpval2;

uint8_t PartFound, tmpPartFound; //the found component
char outval[8];
unsigned int adcv[4];
uint8_t tmpval, tmpval2;
