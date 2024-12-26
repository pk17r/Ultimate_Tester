#include <avr/io.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <stdlib.h>
#include <string.h>
#include <avr/eeprom.h>
#include <math.h>

#include "settings.h"
#include "lcd.h"
#include "ads1115.h"

//Program start
int main(void) {
		
	RST_PORT = (1<<RST_PIN);	// enable Pullup for Reset button
	uint8_t tmp;
	//ADC-Init
	ADCSRA = (1<<ADEN) | (1<<ADPS1) | (1<<ADPS0);	//Enable ADC, set Prescaler to 8

	lcd_init();
		
	// Entry point when start button is pressed during Operation 
	start:
	PartFound = PART_NONE;
	tmpPartFound = PART_NONE;
	NumOfDiodes = 0;
	PartReady = 0;
	PartMode = 0;

	lcd_clear();
			
	//  Begin testing sequence
	lcd_eep_string(TestRunning);	// print the String: "Test running"
	//  check All the 6 possible combinations  for the 3 test probes
	CheckPins(TP1, TP2, TP3);
	CheckPins(TP1, TP3, TP2);
	CheckPins(TP2, TP1, TP3);
	CheckPins(TP2, TP3, TP1);
	CheckPins(TP3, TP2, TP1);
	CheckPins(TP3, TP1, TP2);

	// Done , now comes the evaluation
	GetGateThresholdVoltage();
	lcd_clear();
	//----------------------------------------------
	if(PartFound == PART_DIODE) {
		if(NumOfDiodes == 1) {
			//Standard-Diode
			lcd_eep_string(Diode);	//"Diode: "
			lcd_eep_string(Anode);
			lcd_data(diodes[0].Anode + 49);
			lcd_string(";K=");
			lcd_data(diodes[0].Cathode + 49);
			Line2();	//
			lcd_eep_string(Uf);	//"Uf = "
			lcd_string(itoa(diodes[0].Voltage, outval, 10));
			lcd_eep_string(mV);
			goto end;
		} else if(NumOfDiodes == 2) {
		//dual diode
			if(diodes[0].Anode == diodes[1].Anode) {
				//Common Anode
				lcd_eep_string(DualDiode);	//Dual diode
				lcd_string("CA");
				Line2(); //2. Zeile
				lcd_eep_string(Anode);
				lcd_data(diodes[0].Anode + 49);
				lcd_eep_string(K1);	//";K1="
				lcd_data(diodes[0].Cathode + 49);
				lcd_eep_string(K2);	//";K2="
				lcd_data(diodes[1].Cathode + 49);
				goto end;
			} else if(diodes[0].Cathode == diodes[1].Cathode) {
				//Common Cathode
				lcd_eep_string(DualDiode);	//Dual diode
				lcd_string("CC");
				Line2(); //
				lcd_string("K=");
				lcd_data(diodes[0].Cathode + 49);
				lcd_eep_string(A1);		//";A1="
				lcd_data(diodes[0].Anode + 49);
				lcd_eep_string(A2);		//";A2="
				lcd_data(diodes[1].Anode + 49);
				goto end;
			} else if ((diodes[0].Cathode == diodes[1].Anode) && (diodes[1].Cathode == diodes[0].Anode)) {
				//Antiparallel
				lcd_eep_string(TwoDiodes);	//2 Diodes
				Line2(); //
				lcd_eep_string(Antiparallel);	//Antiparallel
				goto end;
			}
		} else if(NumOfDiodes == 3) {
			// series connection from 2 diodes is recognized as 3 diodes 
			b = 3;
			c = 3;
			/* Check for a possible connection of 2 diodes
			For this 2 of the cathodes and 2 of the anodes must match .
			This is because the diodes are recognized as 2 single ones and ADITIONALLY as one big diode 
			*/
			if((diodes[0].Anode == diodes[1].Anode) || (diodes[0].Anode == diodes[2].Anode)) b = diodes[0].Anode;
			if(diodes[1].Anode == diodes[2].Anode) b = diodes[1].Anode;

			if((diodes[0].Cathode == diodes[1].Cathode) || (diodes[0].Cathode == diodes[2].Cathode)) c = diodes[0].Cathode;
			if(diodes[1].Cathode == diodes[2].Cathode) c = diodes[1].Cathode;
			if((b<3) && (c<3)) {
				lcd_eep_string(TwoDiodes);//2 Diodes
				Line2(); //
				lcd_eep_string(InSeries); //"in Series A="
				lcd_data(b + 49);
				lcd_string(";K=");
				lcd_data(c + 49);
				goto end;
			}
		}
	} 
	//---------------------------------------------
	else if (PartFound == PART_TRANSISTOR) {
		if(PartReady == 0) {	// 2nd test never made, e.g. a transistor with protection diode.
			hfe[1] = hfe[0];
			uBE[1] = uBE[0];
		}
		if((hfe[0]>hfe[1])) {	// If the amplification factor was higher in the first test: swap C and E!
			hfe[1] = hfe[0];
			uBE[1] = uBE[0];
			tmp = c;
			c = e;
			e = tmp;
		}

		if(PartMode == PART_MODE_NPN) {
			lcd_eep_string(NPN);
		} else {
			lcd_eep_string(PNP);
		}
		lcd_eep_string(bstr);	//B=
		lcd_data(b + 49);
		lcd_eep_string(cstr);	//;C=
		lcd_data(c + 49);
		lcd_eep_string(estr);	//;E=
		lcd_data(e + 49);
		Line2(); //2. Zeile
		// calculate amplification factor 
		//hFE =  emitter current / base current
		lhfe = hfe[1];
		lhfe *= RH_RL_RATIO;	// Ratio of high to low resistance
		if(uBE[1]<11) uBE[1] = 11;
		lhfe /= uBE[1];
		hfe[1] = (unsigned int) lhfe;
		lcd_eep_string(hfestr);	//"hFE="
		lcd_string(utoa(hfe[1], outval, 10));
		goto end;
	} 
	//-----------------------------------------
	else if (PartFound == PART_FET) {	//JFET or MOSFET
		if(PartMode&1) {	//N-Kanal
			lcd_data('N');
		} else {
			lcd_data('P');	//P-Kanal
		}
		if((PartMode==PART_MODE_N_D_MOS) || (PartMode==PART_MODE_P_D_MOS)) {
			lcd_eep_string(dmode);	//"-D"
			lcd_eep_string(mosfet);	//"-MOS"
		} else {
			if((PartMode==PART_MODE_N_JFET) || (PartMode==PART_MODE_P_JFET)) {
				lcd_eep_string(jfet);	//"-JFET"
			} else {
				lcd_eep_string(emode);	//"-E"
				lcd_eep_string(mosfet);	//"-MOS"
			}
		}

		Line2(); //
		lcd_eep_string(gds);	//"GDS="
		lcd_data(b + 49);
		lcd_data(c + 49);
		lcd_data(e + 49);
		lcd_data(' ');	// space
		if(PartMode < 3) {	// enrichment MOSFET
			lcd_eep_string(vt);
			lcd_string(outval);	// : gate threshold voltage , was previously determined
			lcd_data('m');
		}
		goto end;
	} 
	//----------------------------------------------
	else if (PartFound == PART_THYRISTOR) {
		lcd_eep_string(Thyristor);	//"Thyristor"
		Line2(); //
		lcd_string("GAK=");
		lcd_data(b + 49);
		lcd_data(c + 49);
		lcd_data(e + 49);
		goto end;
	} 
	//----------------------------------------------
	else if (PartFound == PART_TRIAC) {
		lcd_eep_string(Triac);	//"Triac"
		Line2(); //  goto line2
		lcd_eep_string(Gate);
		lcd_data(b + 49);
		lcd_eep_string(A1);		//";A1="
		lcd_data(e + 49);
		lcd_eep_string(A2);		//";A2="
		lcd_data(c + 49);
		goto end;

	}
	// Test failed
	lcd_eep_string(TestFailed1); //None ,unknown . Or
	Line2(); //
	lcd_eep_string(TestFailed2); // defective
	lcd_eep_string(Bauteil);
	
	//----------------------------------------------
	end:
	while(!(RST_PIN_REG & (1<<RST_PIN)));		// wait until button is released
	_delay_ms(200);
		
	// endless loop
	while(1) {
		// only reached, if the automatic Shutdown has not been installed 
		if(!(RST_PIN_REG & (1<<RST_PIN))) goto start; 
	}
	return 0;
} // END of MAIN()

//=========================================================================
	/*
	function for Testing the Properties of the component with the specified pin assignment
	Parameter:
	HighPin: Pin that Starts at H, logic 1 (connected to Vcc).  
	LowPin: Pin that Starts at L, logic 0 (connected to GND via R_L).
	TristatePin: Pin that Starts input Hi-Z, is put both H and L during the tests.
	*/
void CheckPins(uint8_t HighPin, uint8_t LowPin, uint8_t TristatePin) {
	unsigned int adcv[6];
	uint8_t tmpval, tmpval2;

	// Set pins
	tmpval = (LowPin * 2);			// necessary because of the arrangement of the resistances
									// R_L of TPn is connected to PB(n*2) where n = 0, 1 or 2.
									// TP0 => R_L connected to PB0, TP1 => R_L connected to PB2 ... 
	R_DDR = (1<<tmpval);			// Low pin as output and connected via R_L to ground
	R_PORT = 0;
	ADC_DDR = (1<<HighPin);			// High-Pin as output
	ADC_PORT = (1<<HighPin);		// High-Pin connected to Vcc
	_delay_ms(5);
	// For Some MOSFETs the gate (tristate pin) must be discharged first
	//N-Kanal:
	DischargePin(TristatePin,0);
	// measure voltage at low pin
	adcv[0] = ReadADC(LowPin);
	if(adcv[0] < 521) goto next;	// Does the component close now?
	// otherwise : discharge for P channel (gate tied to Vcc)
	DischargePin(TristatePin,1);
	// measure voltage at the low pin
	adcv[0] = ReadADC(LowPin);

	next:
	if(adcv[0] < 521) {	// If the component has no continuity between HighPin and LowPin
		tmpval2 = (TristatePin * 2);	// necessary because of the arrangement of the resistances
		R_DDR |= (1<<tmpval2);			// Tristate pin via R_L to ground, to test for pnp
		_delay_ms(2);
		adcv[0] = ReadADC(LowPin);		// measure the Voltage at the Low pin
		if(adcv[0] > 18246) {
			// Component conducts => pnp transistor or the like
			// measure Amplification factor in both directions 
			R_DDR = (1<<tmpval);		// Tristate-Pin (base) hi-Z
			tmpval2++;
			R_DDR |= (1<<tmpval2);		// Tristate pin (base) through R_H to ground

			_delay_ms(10);
			adcv[0] = ReadADC(LowPin);		// measure Voltage at the low pin (presumed collector) 
			adcv[2] = ReadADC(TristatePin);	// measure base Voltage
			R_DDR = (1<<tmpval);		// Tristate-Pin (base) hi-Z
			// Check if test has been done before
			if((PartFound == PART_TRANSISTOR) || (PartFound == PART_FET)) PartReady = 1;
			hfe[PartReady] = adcv[0];
			uBE[PartReady] = adcv[2];

			if(adcv[2] > 5213) {
				if(PartFound != PART_THYRISTOR) {
					PartFound = PART_TRANSISTOR;	// PNP transistor found (base is "pulled up")
					PartMode = PART_MODE_PNP;
				}
			} else {
				if(PartFound != PART_THYRISTOR) {
				 	PartFound = PART_FET;			// P- channel MOSFET found (gate is NOT "pulled up")
					PartMode = PART_MODE_P_E_MOS;
				}
			}
			if(PartFound != PART_THYRISTOR) {
				b = TristatePin;
				c = LowPin;
				e = HighPin;
			}
		}

		// Tristate (assumed base) to Vcc, to test for npn
		ADC_PORT = 0;					// Low-Pin connected to ground
		tmpval = (TristatePin * 2);		// necessary because of the arrangement of the resistances
		tmpval2 = (HighPin * 2);		// necessary because of the arrangement of the resistances
		R_DDR = (1<<tmpval) | (1<<tmpval2);			//High-Pin and Tristate-Pin outputs
		R_PORT = (1<<tmpval) | (1<<tmpval2);		//High-Pin and Tristate-Pin connected via R_L to Vcc
		ADC_DDR = (1<<LowPin);			//Low-Pin output
		_delay_ms(10);
		adcv[0] = ReadADC(HighPin);		// measure the voltage at the High-Pin
		if(adcv[0] < 13033) {
			if(PartReady==1) goto testend;
			//  Component conducts => npn transistor or the like

			// Gate discharged
			R_PORT = (1<<tmpval2);			//Tristate-Pin (Gate) via R_L to GND
			_delay_ms(10);
			R_DDR = (1<<tmpval2);			//Tristate-Pin (Gate) Hi-Z
			//Test of Thyristor
			_delay_ms(5);
			adcv[1] = ReadADC(HighPin);		// measure Voltage at the high pin (presumed anode) again
			
			R_PORT = 0;						//High-Pin (presumed Anode) to GND
			_delay_ms(5);
			R_PORT = (1<<tmpval2);			//High-Pin (presumed Anode) back to Vcc
			_delay_ms(5);
			adcv[2] = ReadADC(HighPin);		// measure Voltage at the high pin (presumed anode) again
			if((adcv[1] < 13033) && (adcv[2] > 23459)) {// After switching off the holding current, tha thyristor must close 
				// was switched before Switching off the trigger current and still switched although gate off => thyristor
				PartFound = PART_THYRISTOR;
				//Test of Triac
				R_DDR = 0;
				R_PORT = 0;
				ADC_PORT = (1<<LowPin);	//Low-Pin connected to Vcc
				_delay_ms(5);
				R_DDR = (1<<tmpval2);	//HighPin connected via R_L to GND
				_delay_ms(5);
				if(ReadADC(HighPin) > 1303) goto savenresult;	// measure voltage at high pin (presumed A2); if too high : component is now conducting => not a triac
				R_DDR |= (1<<tmpval);	// Gate connected also via R_L to GND => triac should fire
				_delay_ms(5);
				if(ReadADC(TristatePin) < 5213) goto savenresult; // measure voltage at tristate pin (presumed gate) ; abort if voltage too low 
				if(ReadADC(HighPin) < 3910) goto savenresult; // Component does not conduct now => not a triac => abort
				R_DDR = (1<<tmpval2);	//TristatePin (Gate) Hi-Z again
				_delay_ms(5);
				if(ReadADC(HighPin) < 3910) goto savenresult; // Component not conducting after switching off the gate current => not a triac => abort
				R_PORT = (1<<tmpval2);	//HighPin via R_L to Vcc => holding current off
				_delay_ms(5);
				R_PORT = 0;				//HighPin again via R_L to GND; Triac should now inhibit
				_delay_ms(5);
				if(ReadADC(HighPin) > 1303) goto savenresult;	//measure voltage at High-Pin (presumed A2) agin, if too high: component is now conducting => not a triac
				PartFound = PART_TRIAC;
				PartReady = 1;
				goto savenresult;
			}
			//Test of Transistor or MOSFET
			tmpval++;
			R_DDR |= (1<<tmpval);		//Tristate-Pin (Base) as output
			R_PORT |= (1<<tmpval);		//Tristate-Pin (Base) via R_H to Vcc
			_delay_ms(50);
			adcv[0] = ReadADC(HighPin);		// measure voltage at High-Pin (presumed collector)
			adcv[2] = ReadADC(TristatePin);	// measure base voltage
			R_PORT = (1<<tmpval2);			//Tristate-Pin (Base) Hi-Z
			R_DDR = (1<<tmpval2);			//Tristate-Pin (Base) as input

			if((PartFound == PART_TRANSISTOR) || (PartFound == PART_FET)) PartReady = 1;	// check if test has been balready done
			hfe[PartReady] = 26666 - adcv[0];
			uBE[PartReady] = 26666 - adcv[2];
			if(adcv[2] < 13033) {
				PartFound = PART_TRANSISTOR;	//NPN-Transistor found (base is pulled "down")
				PartMode = PART_MODE_NPN;
			} else {
				PartFound = PART_FET;			// N- channel MOSFET found (gate is NOT pulled "down")
				PartMode = PART_MODE_N_E_MOS;
			}
			savenresult:
			b = TristatePin;
			c = HighPin;
			e = LowPin;
		}
		ADC_DDR = 0;
		ADC_PORT = 0;
		// Done (ready)
	} else {	// passage
		// Test for N-JFET or self-conducting N-MOSFET
		R_DDR |= (2<<(TristatePin*2));	//Tristate-Pin (presumed Gate) via R_H to GND
		_delay_ms(20);
		adcv[0] = ReadADC(LowPin);		// Measure voltage at presumed "source"
		R_PORT |= (2<<(TristatePin*2));	//Tristate-Pin (presumed Gate) via R_H to Vcc
		_delay_ms(20);
		adcv[1] = ReadADC(LowPin);		// measure Voltage at presumed "source" again 
		// If it is	a Self-conducting MOSFET or JFET  adcv[1] should be > adcv[0] 
		if(adcv[1]>(adcv[0]+2606)) {
			// measure voltage at the gate, to Distinguish between MOSFET and JFET
			ADC_PORT = 0;
			ADC_DDR = (1<<LowPin);	//Low-Pin fixed to GND
			tmpval = (HighPin * 2);		// necessary because of the arrangement of the resistances
			R_DDR |= (1<<tmpval);			//High-Pin as output
			R_PORT |= (1<<tmpval);			//High-Pin via R_L to Vcc
			_delay_ms(20);
			adcv[2] = ReadADC(TristatePin);		// Measure voltage at presumed gate
			if(adcv[2]>20853) {	//MOSFET
				PartFound = PART_FET;			//N-Kanal-MOSFET
				PartMode = PART_MODE_N_D_MOS;	// Depletion MOSFET
			} else {	// JFET (pn - junction between G and S leads)
				PartFound = PART_FET;			//N-Kanal-JFET
				PartMode = PART_MODE_N_JFET;
			}
			PartReady = 1;
			b = TristatePin;
			c = HighPin;
			e = LowPin;
		}
		ADC_PORT = 0;

		// Test for P-JFET or self-conducting P-MOSFET
		ADC_DDR = (1<<LowPin);	//Low-Pin (presumed Drain) fixed to GND, Tristate-Pin (presumed Gate) is still connected to Vcc via R_H 
		tmpval = (HighPin * 2);			// necessary because of the arrangement of the resistances
		R_DDR |= (1<<tmpval);			//High-Pin as output
		R_PORT |= (1<<tmpval);			//High-Pin via R_L to Vcc
		_delay_ms(20);
		adcv[0] = ReadADC(HighPin);		// Measure voltage at presumed "source"
		R_PORT = (1<<tmpval);			//Tristate-Pin (presumed Gate) via R_H to GND
		_delay_ms(20);
		adcv[1] = ReadADC(HighPin);		// measure Voltage at presumed "source" again 
		// If it is a Self-conducting P-MOSFET or P-JFET adcv[0] should be > adcv[1].
		if(adcv[0]>(adcv[1]+2606)) {
			// measure voltage at the gate, to Distiguish between MOSFET and JFET
			ADC_PORT = (1<<HighPin);	//High-Pin fixed to Vcc
			ADC_DDR = (1<<HighPin);		//High-Pin as output
			_delay_ms(20);
			adcv[2] = ReadADC(TristatePin);		// Measure voltage at presumed gate
			if(adcv[2]<5213) {	//MOSFET
				PartFound = PART_FET;			//P-Kanal-MOSFET
				PartMode = PART_MODE_P_D_MOS;	// Depletion MOSFET
			} else {	// JFET (pn - junction between G and S leads)
				PartFound = PART_FET;			//P-Kanal-JFET
				PartMode = PART_MODE_P_JFET;
			}
			PartReady = 1;
			b = TristatePin;
			c = LowPin;
			e = HighPin;
		}

		tmpval2 = (2<<(2*HighPin));	//R_H
		tmpval = (1<<(2*HighPin));	//R_L
		ADC_PORT = 0;
		//Test of Diode
		ADC_DDR = (1<<LowPin);	//Low-Pin fidx to GND, High-Pin is still connecte to Vcc via R_L 
		DischargePin(TristatePin,1);	// discharge for P-Kanal-MOSFET
		_delay_ms(5);
		adcv[0] = ReadADC(HighPin) - ReadADC(LowPin);
		R_DDR = tmpval2;	//High-Pin via R_H to Vcc
		R_PORT = tmpval2;
		_delay_ms(5);
		adcv[2] = ReadADC(HighPin) - ReadADC(LowPin);
		R_DDR = tmpval;	//High-Pin via R_L to Vcc
		R_PORT = tmpval;
		DischargePin(TristatePin,0);	// discharge for N-Kanal-MOSFET
		_delay_ms(5);
		adcv[1] = ReadADC(HighPin) - ReadADC(LowPin);
		R_DDR = tmpval2;	//High-Pin via R_H  to Vcc
		R_PORT = tmpval2;
		_delay_ms(5);
		adcv[3] = ReadADC(HighPin) - ReadADC(LowPin);
		/*  Without discharging, false detections may occur since the gate of a mosfet may still be charged.
			The additional Measurement with the "big" resistance R_H is performed to detect antiparallel diodes.
			A diode has a forward pass voltage that is relativily independent of forward current
			For a resistor the voltage drop changes strongly (lineraly) with the current
		*/
		if(adcv[0] > adcv[1]) {
			adcv[1] = adcv[0];	// the higher value wins
			adcv[3] = adcv[2];
		}

		if((adcv[1] > 782) && (adcv[1] < 24763)) { // voltage is above 0.15V and below 4.64V => Ok
			// specify diode only if no other Component has been found yet. 
			// Otherwise there would be problems with transistors with protection diode
			if((PartFound == PART_NONE) || (PartFound == PART_RESISTOR)) PartFound = PART_DIODE;	
			diodes[NumOfDiodes].Anode = HighPin;
			diodes[NumOfDiodes].Cathode = LowPin;
			diodes[NumOfDiodes].Voltage = (adcv[1]*54/11);	// multiply approx by 4.9 to obtain the voltage in millivolts from the ADC value
			NumOfDiodes++;
			for(uint8_t i=0;i<NumOfDiodes;i++) {
				if((diodes[i].Anode == LowPin) && (diodes[i].Cathode == HighPin)) {	//two antiparallel Diodes: defective or Dual LED
					if((adcv[3]*64) < (adcv[1] / 5)) {	// Forward voltage drops sharply(greatl) at lower Test current => defect component
						if(i<NumOfDiodes) {
							for(uint8_t j=i;j<(NumOfDiodes-1);j++) {
								diodes[j].Anode = diodes[j+1].Anode;
								diodes[j].Cathode = diodes[j+1].Cathode;
								diodes[j].Voltage = diodes[j+1].Voltage;
							}
						}
						NumOfDiodes -= 2;
					}
				}
			}
		}
	}

	testend:
	ADC_DDR = 0;
	ADC_PORT = 0;
	R_DDR = 0;
	R_PORT = 0;
}

//=========================================================================
void GetGateThresholdVoltage(void) {
	unsigned int tmpint = 0;
	unsigned int tmpintb = 0;

	uint8_t extcnt = 0;
	tmpval = (1<<(2*c) | (2<<(2*b)));
	tmpval2=(1<<(2*c));
	R_DDR = tmpval;        // Drain via R_L as output, Gate via R_H as output
	ADC_DDR=(1<<e)|(1<<b);	//Gate and Source as output
	if((PartFound==PART_FET) && (PartMode == PART_MODE_N_E_MOS)) {
		// measure Gate threshold voltage 
		ADC_PORT = 0;			//Gate and Source fixed to GND
		R_PORT = tmpval;  	   // Drain via R_L to Vcc, Gate via R_H to Vcc
		tmpval=(1<<c);
		_delay_ms(10);
		ADC_DDR=(1<<e);          // Charge Gate slowly via R_H
	
		while ((ADC_PIN&tmpval)) {  // wait until the MOSFET switches and drain goes low; loop lasts 7 cycles
			tmpint++;
			if(tmpint==0) {
				extcnt++;
				if(extcnt == 8) break; // Timeout for gate threshold voltage measurement
			}
		}
		
		R_PORT=tmpval2;          // switch Gate high impedance 
		R_DDR=tmpval2;          
		tmpintb=ReadADC(b);

	} else if((PartFound==PART_FET) && (PartMode == PART_MODE_P_E_MOS)) {
		ADC_PORT = (1<<e)|(1<<b);	// Gate and source fixed to Vcc
		R_PORT = 0;					//Drain via R_L to GND, Gate via R_H to GND
		tmpval=(1<<c);
		_delay_ms(10);
		ADC_DDR=(1<<e);          //  Gate as input
		ADC_PORT=(1<<e);          // charging the Gate slowly via R_H (Gate Pullup)
		while (!(ADC_PIN&tmpval)) {  // Wait until the MOSFET turns on and drain goes high
			tmpint++;
			if(tmpint==0) {
				extcnt++;
				if(extcnt == 8) break; // Timeout for gate threshold voltage measurement
			}
		}
		R_DDR=tmpval2;          // Gate Hi-Z
		tmpintb=ReadADC(b);

	}
	R_DDR = 0;
	R_PORT = 0;
	ADC_DDR = 0;
	ADC_PORT = 0;
	if((tmpint > 0) || (extcnt > 0)) {
		if(PartMode == PART_MODE_P_E_MOS) {
			tmpintb = 26666-tmpintb;
		}
		tmpintb=(tmpintb*39/8);
		utoa(tmpintb, outval, 10);
	}
}

//=========================================================================
	/* Connect one component pin briefly (10ms) to a specific potential
		This function is intended for Discharging MOSFET gates to be able to detect protection diodes etc
		Parameters:
		PinToDischarge: pin to be discharged
		DischargeDirection: 0 = against  GND (N-Kanal-FET), 1= against  Vcc(P-Kanal-FET)
	*/
void DischargePin(uint8_t PinToDischarge, uint8_t DischargeDirection) {
	uint8_t tmpval;
	tmpval = (PinToDischarge * 2);	// necessary because of the arrangement of the resistances

	if(DischargeDirection) R_PORT |= (1<<tmpval);	//if DischargeDirection = 1,  R_L pin to Vcc
	R_DDR |= (1<<tmpval);			//Pin as output (and via R_L to GND if DischargeDirection = 0)
	_delay_ms(10);
	R_DDR &= ~(1<<tmpval);			// Pin back to input
	if(DischargeDirection) R_PORT &= ~(1<<tmpval);	//R_L off
}

//====================================================================
uint16_t ReadADC(uint8_t ch){
	uint16_t adc;	// digital value

	// read digital value at input An @8SPS and Gain=2/3
	adc = ads1115_readADC_SingleEnded(ADS1115_ADDR_SDA, ch, DATARATE_8SPS, FSR_6_144);
	return adc;
}
