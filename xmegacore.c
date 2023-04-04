#include "xmegacore.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <avr/wdt.h>
#include <avr/delay.h>



void Osc32MHz(void) {
	OSC.CTRL		=	OSC_RC32MEN_bm|OSC_RC32KEN_bm;	// włączenie oscylatora 32MHz
 	while(!(OSC.STATUS & OSC_RC32MRDY_bm));				// czekanie na ustabilizowanie się generatora 32m
	while(!(OSC.STATUS & OSC_RC32KRDY_bm));				// czekanie na ustabilizowanie się generatora 32k
	DFLLRC32M.CTRL =0x01;								// odblokowanie petlikalibracyjnej dfll
	CPU_CCP			=	CCP_IOREG_gc;					// odblokowanie zmiany źródła sygnału zegarowego
	CLK.CTRL		=	CLK_SCLKSEL_RC32M_gc;			// zmiana źródła sygnału zegarowego na RC 32MHz
}
void Osc2MHz(void) {
	OSC.CTRL		=	OSC_RC2MEN_bm;					// włączenie oscylatora 2MHz
	while(!(OSC.STATUS & OSC_RC2MRDY_bm));				// czekanie na ustabilizowanie się generatora
	CPU_CCP			=	CCP_IOREG_gc;					// odblokowanie zmiany źródła sygnału zegarowego
	CLK.CTRL		=	CLK_SCLKSEL_RC2M_gc;			// zmiana źródła sygnału zegarowego na RC 2MHz
}
void OscXtal(void) {	
	OSC.XOSCCTRL	=	OSC_FRQRANGE_12TO16_gc |		// wybór kwarcu od 12 do 16 MHZ
	OSC_XOSCSEL_XTAL_16KCLK_gc;		// czas na uruchomienie generatora
	OSC.CTRL		=	OSC_XOSCEN_bm;					// uruchomienie generatora kwarcowego	
	for(uint8_t i=0; i<255; i++) {
		if(OSC.STATUS & OSC_XOSCRDY_bm) {
			CPU_CCP			=	CCP_IOREG_gc;			// odblokowanie zmiany źródła sygnału zegarowego
			CLK.CTRL		=	CLK_SCLKSEL_XOSC_gc;	// wybór źródła sygnału zegarowego na XTAL 16MHz
			CPU_CCP			=	CCP_IOREG_gc;			// odblokowanie modyfikacji ważnych rejestrów
			OSC.XOSCFAIL	=	OSC_XOSCFDEN_bm;		// włączenie układu detekcji błędu sygnału zegarowego
			return;										// wyjście z funkcji jeśli generator się uruchomił
		}
		_delay_us(10);}
}
void OscPLL(uint8_t pllfactor, uint8_t CSource) {  // Csource to wyboz źródła, 0 to 2 mhz, 1 to 8 mhz.
	if (CSource == 0)
	{OSC.CTRL		=	OSC_RC2MEN_bm;					// włączenie oscylatora 2MHz
	while(!(OSC.STATUS & OSC_RC2MRDY_bm));				// czekanie na ustabilizowanie się generatora
	CPU_CCP			=	CCP_IOREG_gc;					// odblokowanie zmiany źródła sygnału zegarowego
	CLK.CTRL		=	CLK_SCLKSEL_RC2M_gc;			// zmiana źródła sygnału zegarowego na RC 2MHz
	OSC.CTRL		&= ~OSC_PLLEN_bm;
	OSC.PLLCTRL		=	OSC_PLLSRC_RC2M_gc |			// wybór RC 2MHz jako źródło sygnału dla PLL
	pllfactor;						// mnożnik częstotliwości (od 1 do 31)
	}
	if (CSource == 1)
	{	OSC.CTRL		=	OSC_RC32MEN_bm;					// włączenie oscylatora 32MHz
		while(!(OSC.STATUS & OSC_RC32MRDY_bm));				// czekanie na ustabilizowanie się generatora
		CPU_CCP			=	CCP_IOREG_gc;					// odblokowanie zmiany źródła sygnału zegarowego
		CLK.CTRL		=	CLK_SCLKSEL_RC32M_gc;			// zmiana źródła sygnału zegarowego na RC 2MHz
		OSC.CTRL		&= ~OSC_PLLEN_bm;
		OSC.PLLCTRL		=	OSC_PLLSRC_RC32M_gc |			// wybór RC 32MHz jako źródło sygnału dla PLL
		pllfactor;						// mnożnik częstotliwości (od 1 do 31)
	}		
	OSC.CTRL		=	OSC_PLLEN_bm;					// włączenie układu PLL
	while(!(OSC.STATUS & OSC_PLLRDY_bm));
	CPU_CCP			=	CCP_IOREG_gc;					// odblokowanie zmiany źródła sygnału zegarowego
	CLK.CTRL		=	CLK_SCLKSEL_PLL_gc;				// wybór źródła sygnału zegarowego PLL
	CPU_CCP			=	CCP_IOREG_gc;					// odblokowanie modyfikacji ważnych rejestrów
	OSC.XOSCFAIL	=	OSC_PLLFDEN_bm;					// włączenie układu detekcji błędu sygnału zegarowego
}
uint8_t ReadCalibrationByte(uint8_t index) {
	uint8_t result;
	NVM_CMD = NVM_CMD_READ_CALIB_ROW_gc;
	result = pgm_read_byte(index);
	NVM_CMD = NVM_CMD_NO_OPERATION_gc;
	return(result);
}


void ADCA_INIT(uint8_t source, uint8_t conv_mode, uint8_t freemode, uint8_t prescaler, uint8_t analogpin){
	ADCA.CALL = ReadCalibrationByte( offsetof(NVM_PROD_SIGNATURES_t, ADCACAL0) );
	ADCA.CALH = ReadCalibrationByte( offsetof(NVM_PROD_SIGNATURES_t, ADCACAL1) );
	ADCA.CTRLA=0x01; // aktywacja ADC 
	if(conv_mode)ADCA.CTRLB|=0x10; else ADCA.CTRLB&=~0x10; // czy wynik ze znakiem? 
	if(freemode) ADCA.CTRLB|=0x08; else ADCA.CTRLB&=~0x08; // czy maja kanaly ciagle pobierac probki
	ADCA.REFCTRL= 0x03; if(source > 4) source =4; source <<=4; ADCA.REFCTRL |= source; // wybór źródła
	if(prescaler > 7) prescaler =7;	ADCA.PRESCALER = prescaler;						   // wybór preskalera
	if(analogpin & 0x01)PORTA.PIN0CTRL=PORT_ISC_INPUT_DISABLE_gc;
	if(analogpin & 0x02)PORTA.PIN1CTRL=PORT_ISC_INPUT_DISABLE_gc;
	if(analogpin & 0x04)PORTA.PIN2CTRL=PORT_ISC_INPUT_DISABLE_gc;
	if(analogpin & 0x08)PORTA.PIN3CTRL=PORT_ISC_INPUT_DISABLE_gc;
	if(analogpin & 0x10)PORTA.PIN4CTRL=PORT_ISC_INPUT_DISABLE_gc;
	if(analogpin & 0x20)PORTA.PIN5CTRL=PORT_ISC_INPUT_DISABLE_gc;
	if(analogpin & 0x40)PORTA.PIN6CTRL=PORT_ISC_INPUT_DISABLE_gc;
	if(analogpin & 0x80)PORTA.PIN7CTRL=PORT_ISC_INPUT_DISABLE_gc;	
	};
	
void ADCA_DISABLE(){ADCA.CTRLA=0x00;}		

void ADCACH0_start(){ADCA.CH0.CTRL |= 0x80;}
	
uint8_t ADCACH0_ready(){if(ADCA.CH0.INTFLAGS&0x01)return(1); return (0);} 
	
int16_t ADCACH0_read(uint8_t ready ){
if(ready==1){uint16_t devolay=0;
while(ADCACH0_ready()==0){devolay++; if(devolay >5000)return(0);}}

if((ADCA.CTRLB&0x10)&&(ADCA.CH0.RES&0x0800)) return(0xf000 & ADCA.CH0.RES);
return(ADCA.CH0.RES); }	

void ADCACH0_init(uint8_t gain, uint8_t inputmode, uint8_t posend, uint8_t negend){
if(gain>7)gain=7; gain<<=2; if(inputmode>3)inputmode=3;
	ADCA.CH0.CTRL = 0; ADCA.CH0.CTRL |=gain; ADCA.CH0.CTRL |=inputmode;
	if(posend > 15)posend =15; posend <<=3; if(negend>7)negend=7;
	ADCA.CH0.MUXCTRL= posend | negend;		
}	

	
void ADCACH1_start(){ADCA.CH1.CTRL |= 0x80;}

uint8_t ADCACH1_ready(){if(ADCA.CH1.INTFLAGS&0x01)return(1); return (0);}

int16_t ADCACH1_read(uint8_t ready ){
	if(ready==1){uint16_t devolay=0;
		while(ADCACH1_ready()==0){devolay++; if(devolay >5000)return(0);}}

		if((ADCA.CTRLB&0x10)&&(ADCA.CH1.RES&0x0800)) return(0xf000 & ADCA.CH1.RES);
	return(ADCA.CH1.RES); }

	void ADCACH1_init(uint8_t gain, uint8_t inputmode, uint8_t posend, uint8_t negend){
		if(gain>7)gain=7; gain<<=2; if(inputmode>3)inputmode=3;
		ADCA.CH1.CTRL = 0; ADCA.CH1.CTRL |=gain; ADCA.CH1.CTRL |=inputmode;
		if(posend > 15)posend =15; posend <<=3; if(negend>7)negend=7;
		ADCA.CH1.MUXCTRL= posend | negend;
	}

void ADCACH2_start(){ADCA.CH2.CTRL |= 0x80;}

uint8_t ADCACH2_ready(){if(ADCA.CH2.INTFLAGS&0x01)return(1); return (0);}

int16_t ADCACH2_read(uint8_t ready ){
	if(ready==1){uint16_t devolay=0;
		while(ADCACH2_ready()==0){devolay++; if(devolay >5000)return(0);}}

		if((ADCA.CTRLB&0x10)&&(ADCA.CH2.RES&0x0800)) return(0xf000 & ADCA.CH2.RES);
	return(ADCA.CH2.RES); }

	void ADCACH2_init(uint8_t gain, uint8_t inputmode, uint8_t posend, uint8_t negend){
		if(gain>7)gain=7; gain<<=2; if(inputmode>3)inputmode=3;
		ADCA.CH2.CTRL = 0; ADCA.CH2.CTRL |=gain; ADCA.CH2.CTRL |=inputmode;
		if(posend > 15)posend =15; posend <<=3; if(negend>7)negend=7;
		ADCA.CH2.MUXCTRL= posend | negend;
	}

void ADCACH3_start(){ADCA.CH3.CTRL |= 0x80;}

uint8_t ADCACH3_ready(){if(ADCA.CH3.INTFLAGS&0x01)return(1); return (0);}

int16_t ADCACH3_read(uint8_t ready ){
	if(ready==1){uint16_t devolay=0;
		while(ADCACH3_ready()==0){devolay++; if(devolay >5000)return(0);}}

		if((ADCA.CTRLB&0x10)&&(ADCA.CH3.RES&0x0800)) return(0xf000 & ADCA.CH3.RES);
	return(ADCA.CH3.RES); }

	void ADCACH3_init(uint8_t gain, uint8_t inputmode, uint8_t posend, uint8_t negend){
		if(gain>7)gain=7; gain<<=2; if(inputmode>3)inputmode=3;
		ADCA.CH3.CTRL = 0; ADCA.CH3.CTRL |=gain; ADCA.CH3.CTRL |=inputmode;
		if(posend > 15)posend =15; posend <<=3; if(negend>7)negend=7;
		ADCA.CH3.MUXCTRL= posend | negend;
	}




void ADCB_INIT(uint8_t source, uint8_t conv_mode, uint8_t freemode, uint8_t prescaler, uint8_t analogpin){
	ADCB.CALL = ReadCalibrationByte( offsetof(NVM_PROD_SIGNATURES_t, ADCBCAL0) );
	ADCB.CALH = ReadCalibrationByte( offsetof(NVM_PROD_SIGNATURES_t, ADCBCAL1) );
	ADCB.CTRLA=0x01; // aktywacja ADC
	if(conv_mode)ADCB.CTRLB|=0x10; else ADCB.CTRLB&=~0x10; // czy wynik ze znakiem?
	if(freemode) ADCB.CTRLB|=0x08; else ADCB.CTRLB&=~0x08; // czy maja kanaly ciagle pobierac probki
	ADCB.REFCTRL= 0x03; if(source > 4) source =4; source <<=4; ADCB.REFCTRL |= source; // wybór źródła
	if(prescaler > 7) prescaler =7;	ADCB.PRESCALER = prescaler;						   // wybór preskalera
	if(analogpin & 0x01)PORTA.PIN0CTRL=PORT_ISC_INPUT_DISABLE_gc;
	if(analogpin & 0x02)PORTA.PIN1CTRL=PORT_ISC_INPUT_DISABLE_gc;
	if(analogpin & 0x04)PORTA.PIN2CTRL=PORT_ISC_INPUT_DISABLE_gc;
	if(analogpin & 0x08)PORTA.PIN3CTRL=PORT_ISC_INPUT_DISABLE_gc;
	if(analogpin & 0x10)PORTA.PIN4CTRL=PORT_ISC_INPUT_DISABLE_gc;
	if(analogpin & 0x20)PORTA.PIN5CTRL=PORT_ISC_INPUT_DISABLE_gc;
	if(analogpin & 0x40)PORTA.PIN6CTRL=PORT_ISC_INPUT_DISABLE_gc;
	if(analogpin & 0x80)PORTA.PIN7CTRL=PORT_ISC_INPUT_DISABLE_gc;
};

void ADCB_DISABLE(){ADCB.CTRLA=0x00;}

void ADCBCH0_start(){ADCB.CH0.CTRL |= 0x80;}

uint8_t ADCBCH0_ready(){if(ADCB.CH0.INTFLAGS&0x01)return(1); return (0);}

int16_t ADCBCH0_read(uint8_t ready ){
	if(ready==1){uint16_t devolay=0;
		while(ADCBCH0_ready()==0){devolay++; if(devolay >5000)return(0);}}

		if((ADCB.CTRLB&0x10)&&(ADCB.CH0.RES&0x0800)) return(0xf000 & ADCB.CH0.RES);
	return(ADCB.CH0.RES); }

	void ADCBCH0_init(uint8_t gain, uint8_t inputmode, uint8_t posend, uint8_t negend){
		if(gain>7)gain=7; gain<<=2; if(inputmode>3)inputmode=3;
		ADCB.CH0.CTRL = 0; ADCB.CH0.CTRL |=gain; ADCB.CH0.CTRL |=inputmode;
		if(posend > 15)posend =15; posend <<=3; if(negend>7)negend=7;
		ADCB.CH0.MUXCTRL= posend | negend;
	}

	
	void ADCBCH1_start(){ADCB.CH1.CTRL |= 0x80;}

	uint8_t ADCBCH1_ready(){if(ADCB.CH1.INTFLAGS&0x01)return(1); return (0);}

	int16_t ADCBCH1_read(uint8_t ready ){
		if(ready==1){uint16_t devolay=0;
			while(ADCBCH1_ready()==0){devolay++; if(devolay >5000)return(0);}}

			if((ADCB.CTRLB&0x10)&&(ADCB.CH1.RES&0x0800)) return(0xf000 & ADCB.CH1.RES);
		return(ADCB.CH1.RES); }

		void ADCBCH1_init(uint8_t gain, uint8_t inputmode, uint8_t posend, uint8_t negend){
			if(gain>7)gain=7; gain<<=2; if(inputmode>3)inputmode=3;
			ADCB.CH1.CTRL = 0; ADCB.CH1.CTRL |=gain; ADCB.CH1.CTRL |=inputmode;
			if(posend > 15)posend =15; posend <<=3; if(negend>7)negend=7;
			ADCB.CH1.MUXCTRL= posend | negend;
		}

		void ADCBCH2_start(){ADCB.CH2.CTRL |= 0x80;}

		uint8_t ADCBCH2_ready(){if(ADCB.CH2.INTFLAGS&0x01)return(1); return (0);}

		int16_t ADCBCH2_read(uint8_t ready ){
			if(ready==1){uint16_t devolay=0;
				while(ADCBCH2_ready()==0){devolay++; if(devolay >5000)return(0);}}

				if((ADCB.CTRLB&0x10)&&(ADCB.CH2.RES&0x0800)) return(0xf000 & ADCB.CH2.RES);
			return(ADCB.CH2.RES); }

			void ADCBCH2_init(uint8_t gain, uint8_t inputmode, uint8_t posend, uint8_t negend){
				if(gain>7)gain=7; gain<<=2; if(inputmode>3)inputmode=3;
				ADCB.CH2.CTRL = 0; ADCB.CH2.CTRL |=gain; ADCB.CH2.CTRL |=inputmode;
				if(posend > 15)posend =15; posend <<=3; if(negend>7)negend=7;
				ADCB.CH2.MUXCTRL= posend | negend;
			}

			void ADCBCH3_start(){ADCB.CH3.CTRL |= 0x80;}

			uint8_t ADCBCH3_ready(){if(ADCB.CH3.INTFLAGS&0x01)return(1); return (0);}

			int16_t ADCBCH3_read(uint8_t ready ){
				if(ready==1){uint16_t devolay=0;
					while(ADCBCH3_ready()==0){devolay++; if(devolay >5000)return(0);}}

					if((ADCB.CTRLB&0x10)&&(ADCB.CH3.RES&0x0800)) return(0xf000 & ADCB.CH3.RES);
				return(ADCB.CH3.RES); }

				void ADCBCH3_init(uint8_t gain, uint8_t inputmode, uint8_t posend, uint8_t negend){
					if(gain>7)gain=7; gain<<=2; if(inputmode>3)inputmode=3;
					ADCB.CH3.CTRL = 0; ADCB.CH3.CTRL |=gain; ADCB.CH3.CTRL |=inputmode;
					if(posend > 15)posend =15; posend <<=3; if(negend>7)negend=7;
					ADCB.CH3.MUXCTRL= posend | negend;
				}

// DAC

void DAC_init(uint8_t source,  uint8_t mode0, uint8_t mode1, uint8_t low_pwr, uint8_t leftadj){
	DACB.CH0GAINCAL = ReadCalibrationByte( offsetof(NVM_PROD_SIGNATURES_t, DACB0GAINCAL ) );
	DACB.CH1GAINCAL = ReadCalibrationByte( offsetof(NVM_PROD_SIGNATURES_t, DACB1GAINCAL ) );
	DACB.CH0OFFSETCAL = ReadCalibrationByte( offsetof(NVM_PROD_SIGNATURES_t, DACB0OFFCAL ) );
	DACB.CH1OFFSETCAL = ReadCalibrationByte( offsetof(NVM_PROD_SIGNATURES_t, DACB1OFFCAL ) );
	if(low_pwr) DACB.CTRLA=0x03; else DACB.CTRLA=0x01; 
	uint8_t a=0;
	if(mode0 == 1) {DACB.CTRLA |=0x04; a=1;}
	if(mode0 == 2) {DACB.CTRLA |=0x10; a=1;}
	if(mode1 == 1) {DACB.CTRLA |=0x08; a+=2;}
	if(a==1) DACB.CTRLB = 0x00;
	if(a==2) DACB.CTRLB = 0x20;
	if(a==3) DACB.CTRLB = 0x40;
	if(leftadj) DACB.CTRLC =0x01; else DACB.CTRLC =0x00; 
	if(source ==1) DACB.CTRLC |= 0x08;
	if(source ==2) DACB.CTRLC |= 0x10;
	if(source ==3) DACB.CTRLC |= 0x18;
}

uint8_t DAC_write(uint8_t kanal, uint16_t wartosc){
uint16_t a=0;	
if(kanal){
while (!(DACB.STATUS &0x02)){a++; if(a>10000)return(1);}	
DACB.CH1DATA = wartosc;	
}else{
while (!(DACB.STATUS &0x01)){a++; if(a>10000)return(1);}	
DACB.CH0DATA = wartosc;		
} return (0);}


// analog comparator

void AC_init(uint8_t channel, uint8_t int_mode, uint8_t int_lvl, uint8_t speed, uint8_t hysteresis,uint8_t muxpos, uint8_t muxneg, uint8_t outpin, uint8_t scalefactor, uint8_t windowmode,  uint8_t int_window, uint8_t current ){
if(channel ==0){
switch(int_mode){
		case 0:  ACA.AC0CTRL = 0; break;
		case 1:  ACA.AC0CTRL = 0x80; break;
		case 2:  ACA.AC0CTRL = 0xC0; break;
		default:  ACA.AC0CTRL = 0;	break;}
switch(int_lvl){
	case 0:   ACA.AC0CTRL |= 0; break;
	case 1:   ACA.AC0CTRL |= 0x10; break;
	case 2:   ACA.AC0CTRL |= 0x20; break;
	case 3:   ACA.AC0CTRL |= 0x30; break;
	default:  ACA.AC0CTRL |= 0;	break;}
if(speed)	  ACA.AC0CTRL |= 0x08;
switch(hysteresis){
	case 0:   ACA.AC0CTRL |= 0; break;
	case 1:   ACA.AC0CTRL |= 0x02; break;
	case 2:   ACA.AC0CTRL |= 0x04; break;
	default:  ACA.AC0CTRL |= 0;	break;}
if(muxpos <8) ACA.AC0MUXCTRL = muxpos<<3;
if(muxneg <8) ACA.AC0MUXCTRL |= muxneg;
if(outpin) ACA.CTRLA |= 0x01; else ACA.CTRLA &= 0xfe;
if(scalefactor < 0x40)  ACA.CTRLB = scalefactor;
switch(windowmode){
	case 0:   ACA.WINCTRL = 0; break;
	case 1:   ACA.WINCTRL = 0x10; break;
	case 2:   ACA.WINCTRL = 0x14; break;
	case 3:   ACA.WINCTRL = 0x18; break;
	case 4:   ACA.WINCTRL = 0x1c; break;
	default:  ACA.WINCTRL = 0;	break;}
switch(int_lvl){
	case 0:   ACA.WINCTRL |= 0; break;
	case 1:   ACA.WINCTRL |= 0x01; break;
	case 2:   ACA.WINCTRL |= 0x02; break;
	case 3:   ACA.WINCTRL |= 0x03; break;
	default:  ACA.WINCTRL |= 0;	break;}
}

if(channel ==1){
	switch(int_mode){
		case 0:  ACA.AC1CTRL = 0; break;
		case 1:  ACA.AC1CTRL = 0x80; break;
		case 2:  ACA.AC1CTRL = 0xC0; break;
	default:  ACA.AC1CTRL = 0;	break;}
	switch(int_lvl){
		case 0:   ACA.AC1CTRL |= 0; break;
		case 1:   ACA.AC1CTRL |= 0x10; break;
		case 2:   ACA.AC1CTRL |= 0x20; break;
		case 3:   ACA.AC1CTRL |= 0x30; break;
	default:  ACA.AC1CTRL |= 0;	break;}
	if(speed)	  ACA.AC1CTRL |= 0x08;
	switch(hysteresis){
		case 0:   ACA.AC1CTRL |= 0; break;
		case 1:   ACA.AC1CTRL |= 0x02; break;
		case 2:   ACA.AC1CTRL |= 0x04; break;
	default:  ACA.AC1CTRL |= 0;	break;}
	if(muxpos <8) ACA.AC1MUXCTRL = muxpos<<3;
	if(muxneg <8) ACA.AC1MUXCTRL |= muxneg;
	if(outpin) ACA.CTRLA |= 0x01; else ACA.CTRLA &= 0xfe;
	if(scalefactor < 0x40)  ACA.CTRLB = scalefactor;
	switch(windowmode){
		case 0:   ACA.WINCTRL = 0; break;
		case 1:   ACA.WINCTRL = 0x10; break;
		case 2:   ACA.WINCTRL = 0x14; break;
		case 3:   ACA.WINCTRL = 0x18; break;
		case 4:   ACA.WINCTRL = 0x1c; break;
	default:  ACA.WINCTRL = 0;	break;}
	switch(int_lvl){
		case 0:   ACA.WINCTRL |= 0; break;
		case 1:   ACA.WINCTRL |= 0x01; break;
		case 2:   ACA.WINCTRL |= 0x02; break;
		case 3:   ACA.WINCTRL |= 0x03; break;
	default:  ACA.WINCTRL |= 0;	break;}
}

if(channel ==2){
	switch(int_mode){
		case 0:  ACB.AC0CTRL = 0; break;
		case 1:  ACB.AC0CTRL = 0x80; break;
		case 2:  ACB.AC0CTRL = 0xC0; break;
	default:  ACB.AC0CTRL = 0;	break;}
	switch(int_lvl){
		case 0:   ACB.AC0CTRL |= 0; break;
		case 1:   ACB.AC0CTRL |= 0x10; break;
		case 2:   ACB.AC0CTRL |= 0x20; break;
		case 3:   ACB.AC0CTRL |= 0x30; break;
	default:  ACB.AC0CTRL |= 0;	break;}
	if(speed)	  ACB.AC0CTRL |= 0x08;
	switch(hysteresis){
		case 0:   ACB.AC0CTRL |= 0; break;
		case 1:   ACB.AC0CTRL |= 0x02; break;
		case 2:   ACB.AC0CTRL |= 0x04; break;
	default:  ACB.AC0CTRL |= 0;	break;}
	if(muxpos <8) ACB.AC0MUXCTRL = muxpos<<3;
	if(muxneg <8) ACB.AC0MUXCTRL |= muxneg;
	if(outpin) ACB.CTRLA |= 0x01; else ACB.CTRLA &= 0xfe;
	if(scalefactor < 0x40)  ACB.CTRLB = scalefactor;
	switch(windowmode){
		case 0:   ACB.WINCTRL = 0; break;
		case 1:   ACB.WINCTRL = 0x10; break;
		case 2:   ACB.WINCTRL = 0x14; break;
		case 3:   ACB.WINCTRL = 0x18; break;
		case 4:   ACB.WINCTRL = 0x1c; break;
	default:  ACB.WINCTRL = 0;	break;}
	switch(int_lvl){
		case 0:   ACB.WINCTRL |= 0; break;
		case 1:   ACB.WINCTRL |= 0x01; break;
		case 2:   ACB.WINCTRL |= 0x02; break;
		case 3:   ACB.WINCTRL |= 0x03; break;
	default:  ACB.WINCTRL |= 0;	break;}
}

if(channel ==3){
	switch(int_mode){
		case 0:  ACB.AC1CTRL = 0; break;
		case 1:  ACB.AC1CTRL = 0x80; break;
		case 2:  ACB.AC1CTRL = 0xC0; break;
	default:  ACB.AC1CTRL = 0;	break;}
	switch(int_lvl){
		case 0:   ACB.AC1CTRL |= 0; break;
		case 1:   ACB.AC1CTRL |= 0x10; break;
		case 2:   ACB.AC1CTRL |= 0x20; break;
		case 3:   ACB.AC1CTRL |= 0x30; break;
	default:  ACB.AC1CTRL |= 0;	break;}
	if(speed)	  ACB.AC1CTRL |= 0x08;
	switch(hysteresis){
		case 0:   ACB.AC1CTRL |= 0; break;
		case 1:   ACB.AC1CTRL |= 0x02; break;
		case 2:   ACB.AC1CTRL |= 0x04; break;
	default:  ACB.AC1CTRL |= 0;	break;}
	if(muxpos <8) ACB.AC1MUXCTRL = muxpos<<3;
	if(muxneg <8) ACB.AC1MUXCTRL |= muxneg;
	if(outpin) ACB.CTRLA |= 0x01; else ACB.CTRLA &= 0xfe;
	if(scalefactor < 0x40)  ACB.CTRLB = scalefactor;
	switch(windowmode){
		case 0:   ACB.WINCTRL = 0; break;
		case 1:   ACB.WINCTRL = 0x10; break;
		case 2:   ACB.WINCTRL = 0x14; break;
		case 3:   ACB.WINCTRL = 0x18; break;
		case 4:   ACB.WINCTRL = 0x1c; break;
	default:  ACB.WINCTRL = 0;	break;}
	switch(int_lvl){
		case 0:   ACB.WINCTRL |= 0; break;
		case 1:   ACB.WINCTRL |= 0x01; break;
		case 2:   ACB.WINCTRL |= 0x02; break;
		case 3:   ACB.WINCTRL |= 0x03; break;
	default:  ACB.WINCTRL |= 0;	break;}
}}

void AC_disable(uint8_t channel){
	switch(channel){
		case 0: ACA.AC0CTRL &=0xfe; break;
		case 1: ACA.AC1CTRL &=0xfe; break;		
		case 2: ACB.AC0CTRL &=0xfe; break;
		case 3: ACB.AC1CTRL &=0xfe; break;					
	}
	
}	

uint8_t AC_state(uint8_t channel){
	switch(channel){
		case 0: if(ACA.STATUS&0x10)return (1); else return(0); break;
		case 1: if(ACA.STATUS&0x20)return (1); else return(0); break;
		case 2: if(ACB.STATUS&0x10)return (1); else return(0); break;
		case 3: if(ACB.STATUS&0x20)return (1); else return(0); break;
		case 4: return(ACA.STATUS>>6); break;
		case 5: return(ACB.STATUS>>6); break;
	}return(0);
}	



// UART
void UART_init(uint8_t channel,uint8_t char_mode, uint8_t work_mode,uint8_t mode_io, uint8_t parity, uint8_t stopbit,   uint32_t baudrate, uint32_t fcpu, uint8_t int_tx, uint8_t int_rx ,uint8_t int_ready){
if(channel ==0){
	switch(char_mode){
		case 0:  USARTC0.CTRLC =0x00; break;
		case 1:  USARTC0.CTRLC =0x01; break;
		case 2:  USARTC0.CTRLC =0x02; break;
		case 3:  USARTC0.CTRLC =0x03; break;
		case 4:  USARTC0.CTRLC =0x07; break;
		default: USARTC0.CTRLC =0x03; break;	}	
	switch(work_mode){
		case 0:  USARTC0.CTRLC |=0x00; break;
		case 1:  USARTC0.CTRLC |=0x40; break;
		case 2:  USARTC0.CTRLC |=0x80; break;
		default: USARTC0.CTRLC |=0x00; break;	}		
	switch(parity){
		case 0:  USARTC0.CTRLC |=0x00; break;
		case 1:  USARTC0.CTRLC |=0x20; break;
		case 2:  USARTC0.CTRLC |=0x30; break;
		default: USARTC0.CTRLC |=0x00; break;	}
	if(stopbit) USARTC0.CTRLC|=0x08;			
	switch(mode_io){
		case 0:  USARTC0.CTRLB =0x00; break;
		case 1:  USARTC0.CTRLB =0x10; break;
		case 2:  USARTC0.CTRLB =0x08; break;
		case 3:  USARTC0.CTRLB =0x18; break;
		default: USARTC0.CTRLB =0x18; break;	}	
	switch(int_ready){
		case 0:  USARTC0.CTRLA =0x00; break;
		case 1:  USARTC0.CTRLA =0x01; break;
		case 2:  USARTC0.CTRLA =0x02; break;
		case 3:  USARTC0.CTRLA =0x03; break;
		default: USARTC0.CTRLA =0x00; break;	}		
	switch(int_tx){
		case 0:  USARTC0.CTRLA |=0x00; break;
		case 1:  USARTC0.CTRLA |=0x04; break;
		case 2:  USARTC0.CTRLA |=0x08; break;
		case 3:  USARTC0.CTRLA |=0x0C; break;
		default: USARTC0.CTRLA |=0x00; break;	}	
	switch(int_rx){
		case 0:  USARTC0.CTRLA |=0x00; break;
		case 1:  USARTC0.CTRLA |=0x10; break;
		case 2:  USARTC0.CTRLA |=0x20; break;
		case 3:  USARTC0.CTRLA |=0x30; break;
		default: USARTC0.CTRLA |=0x00; break;	}
	float bscale =128;
	float mod=(baudrate*16);
	float mod2 = fcpu;
	mod2/=mod;
	mod2--;
	for(int8_t i=-7; i<8; i++){
	mod=bscale*mod2;
	if(mod < 4090){
	uint16_t wyn = mod;
	USARTC0.BAUDCTRLA =wyn; USARTC0.BAUDCTRLB =wyn>>8; 
	USARTC0.BAUDCTRLB |= i<<4;
	i=10;}
	bscale/=2;}	USARTC0.STATUS=0x40;}

if(channel ==1){
	switch(char_mode){
		case 0:  USARTC1.CTRLC =0x00; break;
		case 1:  USARTC1.CTRLC =0x01; break;
		case 2:  USARTC1.CTRLC =0x02; break;
		case 3:  USARTC1.CTRLC =0x03; break;
		case 4:  USARTC1.CTRLC =0x07; break;
	default: USARTC1.CTRLC =0x03; break;	}
	switch(work_mode){
		case 0:  USARTC1.CTRLC |=0x00; break;
		case 1:  USARTC1.CTRLC |=0x40; break;
		case 2:  USARTC1.CTRLC |=0x80; break;
	default: USARTC1.CTRLC |=0x00; break;	}
	switch(parity){
		case 0:  USARTC1.CTRLC |=0x00; break;
		case 1:  USARTC1.CTRLC |=0x20; break;
		case 2:  USARTC1.CTRLC |=0x30; break;
	default: USARTC1.CTRLC |=0x00; break;	}
	if(stopbit) USARTC1.CTRLC|=0x08;
	switch(mode_io){
		case 0:  USARTC1.CTRLB =0x00; break;
		case 1:  USARTC1.CTRLB =0x10; break;
		case 2:  USARTC1.CTRLB =0x08; break;
		case 3:  USARTC1.CTRLB =0x18; break;
	default: USARTC1.CTRLB =0x18; break;	}
	switch(int_ready){
		case 0:  USARTC1.CTRLA =0x00; break;
		case 1:  USARTC1.CTRLA =0x01; break;
		case 2:  USARTC1.CTRLA =0x02; break;
		case 3:  USARTC1.CTRLA =0x03; break;
	default: USARTC1.CTRLA =0x00; break;	}
	switch(int_tx){
		case 0:  USARTC1.CTRLA |=0x00; break;
		case 1:  USARTC1.CTRLA |=0x04; break;
		case 2:  USARTC1.CTRLA |=0x08; break;
		case 3:  USARTC1.CTRLA |=0x0C; break;
	default: USARTC1.CTRLA |=0x00; break;	}
	switch(int_rx){
		case 0:  USARTC1.CTRLA |=0x00; break;
		case 1:  USARTC1.CTRLA |=0x10; break;
		case 2:  USARTC1.CTRLA |=0x20; break;
		case 3:  USARTC1.CTRLA |=0x30; break;
	default: USARTC1.CTRLA |=0x00; break;	}
	float bscale =128;
	float mod=(baudrate*16);
	float mod2 = fcpu;
	mod2/=mod;
	mod2--;
	for(int8_t i=-7; i<8; i++){
		mod=bscale*mod2;
		if(mod < 4090){
			uint16_t wyn = mod;
			USARTC1.BAUDCTRLA =wyn; USARTC1.BAUDCTRLB =wyn>>8;
			USARTC1.BAUDCTRLB |= i<<4;
		i=10;}
	bscale/=2;}USARTC1.STATUS=0x40;}	
	
if(channel ==2){
	switch(char_mode){
		case 0:  USARTD0.CTRLC =0x00; break;
		case 1:  USARTD0.CTRLC =0x01; break;
		case 2:  USARTD0.CTRLC =0x02; break;
		case 3:  USARTD0.CTRLC =0x03; break;
		case 4:  USARTD0.CTRLC =0x07; break;
	default: USARTD0.CTRLC =0x03; break;	}
	switch(work_mode){
		case 0:  USARTD0.CTRLC |=0x00; break;
		case 1:  USARTD0.CTRLC |=0x40; break;
		case 2:  USARTD0.CTRLC |=0x80; break;
	default: USARTD0.CTRLC |=0x00; break;	}
	switch(parity){
		case 0:  USARTD0.CTRLC |=0x00; break;
		case 1:  USARTD0.CTRLC |=0x20; break;
		case 2:  USARTD0.CTRLC |=0x30; break;
	default: USARTD0.CTRLC |=0x00; break;	}
	if(stopbit) USARTD0.CTRLC|=0x08;
	switch(mode_io){
		case 0:  USARTD0.CTRLB =0x00; break;
		case 1:  USARTD0.CTRLB =0x10; break;
		case 2:  USARTD0.CTRLB =0x08; break;
		case 3:  USARTD0.CTRLB =0x18; break;
	default: USARTD0.CTRLB =0x18; break;	}
	switch(int_ready){
		case 0:  USARTD0.CTRLA =0x00; break;
		case 1:  USARTD0.CTRLA =0x01; break;
		case 2:  USARTD0.CTRLA =0x02; break;
		case 3:  USARTD0.CTRLA =0x03; break;
	default: USARTD0.CTRLA =0x00; break;	}
	switch(int_tx){
		case 0:  USARTD0.CTRLA |=0x00; break;
		case 1:  USARTD0.CTRLA |=0x04; break;
		case 2:  USARTD0.CTRLA |=0x08; break;
		case 3:  USARTD0.CTRLA |=0x0C; break;
	default: USARTD0.CTRLA |=0x00; break;	}
	switch(int_rx){
		case 0:  USARTD0.CTRLA |=0x00; break;
		case 1:  USARTD0.CTRLA |=0x10; break;
		case 2:  USARTD0.CTRLA |=0x20; break;
		case 3:  USARTD0.CTRLA |=0x30; break;
	default: USARTD0.CTRLA |=0x00; break;	}
	float bscale =128;
	float mod=(baudrate*16);
	float mod2 = fcpu;
	mod2/=mod;
	mod2--;
	for(int8_t i=-7; i<8; i++){
		mod=bscale*mod2;
		if(mod < 4090){
			uint16_t wyn = mod;
			USARTD0.BAUDCTRLA =wyn; USARTD0.BAUDCTRLB =wyn>>8;
			USARTD0.BAUDCTRLB |= i<<4;
		i=10;}
	bscale/=2;}USARTD0.STATUS=0x40;}

	if(channel ==3){
		switch(char_mode){
			case 0:  USARTD1.CTRLC =0x00; break;
			case 1:  USARTD1.CTRLC =0x01; break;
			case 2:  USARTD1.CTRLC =0x02; break;
			case 3:  USARTD1.CTRLC =0x03; break;
			case 4:  USARTD1.CTRLC =0x07; break;
		default: USARTD1.CTRLC =0x03; break;	}
		switch(work_mode){
			case 0:  USARTD1.CTRLC |=0x00; break;
			case 1:  USARTD1.CTRLC |=0x40; break;
			case 2:  USARTD1.CTRLC |=0x80; break;
		default: USARTD1.CTRLC |=0x00; break;	}
		switch(parity){
			case 0:  USARTD1.CTRLC |=0x00; break;
			case 1:  USARTD1.CTRLC |=0x20; break;
			case 2:  USARTD1.CTRLC |=0x30; break;
		default: USARTD1.CTRLC |=0x00; break;	}
		if(stopbit) USARTD1.CTRLC|=0x08;
		switch(mode_io){
			case 0:  USARTD1.CTRLB =0x00; break;
			case 1:  USARTD1.CTRLB =0x10; break;
			case 2:  USARTD1.CTRLB =0x08; break;
			case 3:  USARTD1.CTRLB =0x18; break;
		default: USARTD1.CTRLB =0x18; break;	}
		switch(int_ready){
			case 0:  USARTD1.CTRLA =0x00; break;
			case 1:  USARTD1.CTRLA =0x01; break;
			case 2:  USARTD1.CTRLA =0x02; break;
			case 3:  USARTD1.CTRLA =0x03; break;
		default: USARTD1.CTRLA =0x00; break;	}
		switch(int_tx){
			case 0:  USARTD1.CTRLA |=0x00; break;
			case 1:  USARTD1.CTRLA |=0x04; break;
			case 2:  USARTD1.CTRLA |=0x08; break;
			case 3:  USARTD1.CTRLA |=0x0C; break;
		default: USARTD1.CTRLA |=0x00; break;	}
		switch(int_rx){
			case 0:  USARTD1.CTRLA |=0x00; break;
			case 1:  USARTD1.CTRLA |=0x10; break;
			case 2:  USARTD1.CTRLA |=0x20; break;
			case 3:  USARTD1.CTRLA |=0x30; break;
		default: USARTD1.CTRLA |=0x00; break;	}
		float bscale =128;
		float mod=(baudrate*16);
		float mod2 = fcpu;
		mod2/=mod;
		mod2--;
		for(int8_t i=-7; i<8; i++){
			mod=bscale*mod2;
			if(mod < 4090){
				uint16_t wyn = mod;
				USARTD1.BAUDCTRLA =wyn; USARTD1.BAUDCTRLB =wyn>>8;
				USARTD1.BAUDCTRLB |= i<<4;
			i=10;}
		bscale/=2;}USARTD1.STATUS=0x40;}
		
if(channel ==4){
	switch(char_mode){
		case 0:  USARTE0.CTRLC =0x00; break;
		case 1:  USARTE0.CTRLC =0x01; break;
		case 2:  USARTE0.CTRLC =0x02; break;
		case 3:  USARTE0.CTRLC =0x03; break;
		case 4:  USARTE0.CTRLC =0x07; break;
	default: USARTE0.CTRLC =0x03; break;	}
	switch(work_mode){
		case 0:  USARTE0.CTRLC |=0x00; break;
		case 1:  USARTE0.CTRLC |=0x40; break;
		case 2:  USARTE0.CTRLC |=0x80; break;
	default: USARTE0.CTRLC |=0x00; break;	}
	switch(parity){
		case 0:  USARTE0.CTRLC |=0x00; break;
		case 1:  USARTE0.CTRLC |=0x20; break;
		case 2:  USARTE0.CTRLC |=0x30; break;
	default: USARTE0.CTRLC |=0x00; break;	}
	if(stopbit) USARTE0.CTRLC|=0x08;
	switch(mode_io){
		case 0:  USARTE0.CTRLB =0x00; break;
		case 1:  USARTE0.CTRLB =0x10; break;
		case 2:  USARTE0.CTRLB =0x08; break;
		case 3:  USARTE0.CTRLB =0x18; break;
	default: USARTE0.CTRLB =0x18; break;	}
	switch(int_ready){
		case 0:  USARTE0.CTRLA =0x00; break;
		case 1:  USARTE0.CTRLA =0x01; break;
		case 2:  USARTE0.CTRLA =0x02; break;
		case 3:  USARTE0.CTRLA =0x03; break;
	default: USARTE0.CTRLA =0x00; break;	}
	switch(int_tx){
		case 0:  USARTE0.CTRLA |=0x00; break;
		case 1:  USARTE0.CTRLA |=0x04; break;
		case 2:  USARTE0.CTRLA |=0x08; break;
		case 3:  USARTE0.CTRLA |=0x0C; break;
	default: USARTE0.CTRLA |=0x00; break;	}
	switch(int_rx){
		case 0:  USARTE0.CTRLA |=0x00; break;
		case 1:  USARTE0.CTRLA |=0x10; break;
		case 2:  USARTE0.CTRLA |=0x20; break;
		case 3:  USARTE0.CTRLA |=0x30; break;
	default: USARTE0.CTRLA |=0x00; break;	}
	float bscale =128;
	float mod=(baudrate*16);
	float mod2 = fcpu;
	mod2/=mod;
	mod2--;
	for(int8_t i=-7; i<8; i++){
		mod=bscale*mod2;
		if(mod < 4090){
			uint16_t wyn = mod;
			USARTE0.BAUDCTRLA =wyn; USARTE0.BAUDCTRLB =wyn>>8;
			USARTE0.BAUDCTRLB |= i<<4;
		i=10;}
	bscale/=2;}USARTE0.STATUS=0x40;}

	if(channel ==5){
		switch(char_mode){
			case 0:  USARTF0.CTRLC =0x00; break;
			case 1:  USARTF0.CTRLC =0x01; break;
			case 2:  USARTF0.CTRLC =0x02; break;
			case 3:  USARTF0.CTRLC =0x03; break;
			case 4:  USARTF0.CTRLC =0x07; break;
		default: USARTF0.CTRLC =0x03; break;	}
		switch(work_mode){
			case 0:  USARTF0.CTRLC |=0x00; break;
			case 1:  USARTF0.CTRLC |=0x40; break;
			case 2:  USARTF0.CTRLC |=0x80; break;
		default: USARTF0.CTRLC |=0x00; break;	}
		switch(parity){
			case 0:  USARTF0.CTRLC |=0x00; break;
			case 1:  USARTF0.CTRLC |=0x20; break;
			case 2:  USARTF0.CTRLC |=0x30; break;
		default: USARTF0.CTRLC |=0x00; break;	}
		if(stopbit) USARTF0.CTRLC|=0x08;
		switch(mode_io){
			case 0:  USARTF0.CTRLB =0x00; break;
			case 1:  USARTF0.CTRLB =0x10; break;
			case 2:  USARTF0.CTRLB =0x08; break;
			case 3:  USARTF0.CTRLB =0x18; break;
		default: USARTF0.CTRLB =0x18; break;	}
		switch(int_ready){
			case 0:  USARTF0.CTRLA =0x00; break;
			case 1:  USARTF0.CTRLA =0x01; break;
			case 2:  USARTF0.CTRLA =0x02; break;
			case 3:  USARTF0.CTRLA =0x03; break;
		default: USARTF0.CTRLA =0x00; break;	}
		switch(int_tx){
			case 0:  USARTF0.CTRLA |=0x00; break;
			case 1:  USARTF0.CTRLA |=0x04; break;
			case 2:  USARTF0.CTRLA |=0x08; break;
			case 3:  USARTF0.CTRLA |=0x0C; break;
		default: USARTF0.CTRLA |=0x00; break;	}
		switch(int_rx){
			case 0:  USARTF0.CTRLA |=0x00; break;
			case 1:  USARTF0.CTRLA |=0x10; break;
			case 2:  USARTF0.CTRLA |=0x20; break;
			case 3:  USARTF0.CTRLA |=0x30; break;
		default: USARTF0.CTRLA |=0x00; break;	}
		float bscale =128;
		float mod=(baudrate*16);
		float mod2 = fcpu;
		mod2/=mod;
		mod2--;
		for(int8_t i=-7; i<8; i++){
			mod=bscale*mod2;
			if(mod < 4090){
				uint16_t wyn = mod;
				USARTF0.BAUDCTRLA =wyn; USARTF0.BAUDCTRLB =wyn>>8;
				USARTF0.BAUDCTRLB |= i<<4;
			i=10;}
		bscale/=2;}USARTF0.STATUS=0x40;}
					
	}
uint16_t Uart_read(uint8_t channel, uint8_t fast){
uint16_t a=0;
uint16_t wyn=0;
if(channel == 0){
	if(fast==0)while(!(USARTC0.STATUS&0x80)){asm("nop");a++; if(a>32000)return(0x8000);}	
	if(USARTC0.CTRLC&0x04){
		wyn=USARTC0.STATUS&0x01; wyn=wyn<<8;	}
	wyn|=USARTC0.DATA; return(wyn);}
if(channel == 1){
	if(fast==0) while(!(USARTC1.STATUS&0x80)){asm("nop");a++; if(a>32000)return(0x8000);}
	if(USARTC1.CTRLC&0x04){
		wyn=USARTC1.STATUS&0x01; wyn=wyn<<8;	}
	wyn|=USARTC1.DATA; return(wyn);}
if(channel == 2){
	if(fast==0)while(!(USARTD0.STATUS&0x80)){asm("nop");a++; if(a>32000)return(0x8000);}
	if(USARTD0.CTRLC&0x04){
		wyn=USARTD0.STATUS&0x01; wyn=wyn<<8;	}
	wyn|=USARTD0.DATA; return(wyn);}
if(channel == 3){
	if(fast==0)while(!(USARTD1.STATUS&0x80)){asm("nop");a++; if(a>32000)return(0x8000);}
	if(USARTD1.CTRLC&0x04){
		wyn=USARTD1.STATUS&0x01; wyn=wyn<<8;	}
	wyn|=USARTD1.DATA; return(wyn);}
if(channel == 4){
	if(fast==0)while(!(USARTE0.STATUS&0x80)){asm("nop");a++; if(a>32000)return(0x8000);}
	if(USARTE0.CTRLC&0x04){
		wyn=USARTE0.STATUS&0x01; wyn=wyn<<8;	}
	wyn|=USARTE0.DATA; return(wyn);}
if(channel == 5){
	if(fast==0)while(!(USARTF0.STATUS&0x80)){asm("nop");a++; if(a>32000)return(0x8000);}
	if(USARTF0.CTRLC&0x04){
		wyn=USARTF0.STATUS&0x01; wyn=wyn<<8;	}
	wyn|=USARTF0.DATA; return(wyn);}	
	return(0);
	}
	
uint8_t Uart_write(uint8_t channel, uint16_t data, uint8_t fast){
uint16_t a=0;	
if(channel == 0){
	USARTC0.STATUS|= 0x40;
	if(USARTC0.CTRLC&0x04){	if(data&0x0100)USARTC0.CTRLB|=0x01; else USARTC0.CTRLB&=0xfe;}
	USARTC0.DATA=data;	
	if(fast==0)while(!(USARTC0.STATUS &0x40)){if(a>32000) return(0x80);else a++; }
	}
if(channel == 1){
	USARTC1.STATUS|= 0x40;
	if(USARTC1.CTRLC&0x04){	if(data&0x0100)USARTC1.CTRLB|=0x01; else USARTC1.CTRLB&=0xfe;}
	USARTC1.DATA=data;
	if(fast==0)while(!(USARTC1.STATUS &0x40)){if(a>32000) return(0x80);else a++; }
}
if(channel == 2){
	USARTD0.STATUS|= 0x40;
	if(USARTD0.CTRLC&0x04){	if(data&0x0100)USARTD0.CTRLB|=0x01; else USARTD0.CTRLB&=0xfe;}
	USARTD0.DATA=data;
	if(fast==0)while(!(USARTD0.STATUS &0x40)){if(a>32000) return(0x80);else a++; }
}
if(channel == 3){
	USARTD1.STATUS|= 0x40;
	if(USARTD1.CTRLC&0x04){	if(data&0x0100)USARTD1.CTRLB|=0x01; else USARTD1.CTRLB&=0xfe;}
	USARTD1.DATA=data;
	if(fast==0)while(!(USARTD1.STATUS &0x40)){if(a>32000) return(0x80);else a++; }
}
if(channel == 4){
	USARTE0.STATUS|= 0x40;
	if(USARTE0.CTRLC&0x04){	if(data&0x0100)USARTE0.CTRLB|=0x01; else USARTE0.CTRLB&=0xfe;}
	USARTE0.DATA=data;
	if(fast==0)while(!(USARTE0.STATUS &0x40)){if(a>32000) return(0x80);else a++; }
}
if(channel == 5){
	USARTF0.STATUS|= 0x40;
	if(USARTF0.CTRLC&0x04){	if(data&0x0100)USARTF0.CTRLB|=0x01; else USARTF0.CTRLB&=0xfe;}
	USARTF0.DATA=data;
	if(fast==0)while(!(USARTF0.STATUS &0x40)){if(a>32000) return(0x80);else a++; }
}	
return(0);}

void ETWI_MASTER_init(uint8_t channel, uint8_t int_mode,uint8_t int_lvl, uint32_t baudrate, uint32_t fcpu, uint8_t timeout){
if(channel == 0){ TWIC.CTRL =0; 
		switch(int_lvl){
			case 0: TWIC.MASTER.CTRLA =0x00;  break;
			case 1: TWIC.MASTER.CTRLA =0x40;  break;
			case 2: TWIC.MASTER.CTRLA =0x80;  break;
			case 3: TWIC.MASTER.CTRLA =0xc0;  break;
		default: TWIC.MASTER.CTRLA =0; break;}
		switch(int_mode){
			case 0: TWIC.MASTER.CTRLA |=0;  break;
			case 1: TWIC.MASTER.CTRLA |=0x10;  break;
			case 2: TWIC.MASTER.CTRLA |=0x20;  break;
			case 3: TWIC.MASTER.CTRLA |=0x30;  break;
		default: TWIC.MASTER.CTRLA |=0; break;}	
	if(baudrate > 400000) baudrate =400000;
	uint32_t teemp= baudrate*2;
	teemp = fcpu/teemp;
	if(teemp <6) teemp=6;
	teemp -=5;
	if(teemp >255)teemp =255;
	TWIC.MASTER.BAUD=teemp;		
	switch(timeout){
		case 0: TWIC.MASTER.CTRLB =0;  break;
		case 1: TWIC.MASTER.CTRLB =0x04;  break;
		case 2: TWIC.MASTER.CTRLB =0x08;  break;
		case 3: TWIC.MASTER.CTRLB =0x0C;  break;
	default: TWIC.MASTER.CTRLB =0; break;}
	TWIC.MASTER.CTRLA |= 0x08;}

if(channel == 1){ TWIE.CTRL =0;
	switch(int_lvl){
		case 0: TWIE.MASTER.CTRLA =0x00;  break;
		case 1: TWIE.MASTER.CTRLA =0x40;  break;
		case 2: TWIE.MASTER.CTRLA =0x80;  break;
		case 3: TWIE.MASTER.CTRLA =0xc0;  break;
	default: TWIE.MASTER.CTRLA =0; break;}
	switch(int_mode){
		case 0: TWIE.MASTER.CTRLA |=0;  break;
		case 1: TWIE.MASTER.CTRLA |=0x10;  break;
		case 2: TWIE.MASTER.CTRLA |=0x20;  break;
		case 3: TWIE.MASTER.CTRLA |=0x30;  break;
	default: TWIE.MASTER.CTRLA |=0; break;}
	if(baudrate > 1000000) baudrate =1000000;
	uint32_t teemp= baudrate*2;
	teemp = fcpu/teemp;
	if(teemp <6) teemp=6;
	teemp -=5;
	if(teemp >255)teemp =255;
	TWIE.MASTER.BAUD=teemp;
	switch(timeout){
		case 0: TWIE.MASTER.CTRLB =0;  break;
		case 1: TWIE.MASTER.CTRLB =0x04;  break;
		case 2: TWIE.MASTER.CTRLB =0x08;  break;
		case 3: TWIE.MASTER.CTRLB =0x0C;  break;
	default: TWIE.MASTER.CTRLB =0; break;}
TWIE.MASTER.CTRLA |= 0x08;}
	}
	
void ETWI_MASTER_order(uint8_t channel, uint8_t order){
	if(channel == 0){
			switch(order){	
			case 0:  TWIC.MASTER.CTRLC =0x01;  break;
			case 1:  TWIC.MASTER.CTRLC =0x02;  break;
			case 2:  TWIC.MASTER.CTRLC =0x06;  break;
			case 3:  TWIC.MASTER.CTRLC =0x03;  break;
			default: TWIC.MASTER.CTRLC =0; break;}}
	if(channel == 1){
		switch(order){
			case 0:  TWIE.MASTER.CTRLC =0x01;  break;
			case 1:  TWIE.MASTER.CTRLC =0x02;  break;
			case 2:  TWIE.MASTER.CTRLC =0x06;  break;
			case 3:  TWIE.MASTER.CTRLC =0x03;  break;
			default: TWIE.MASTER.CTRLC =0; break;}}
}
void ETWI_SLAVE_init (uint8_t channel, uint8_t int_mode ,uint8_t int_lvl, uint8_t workmode, uint8_t adres, uint8_t adresmask){};
void ETWI_SLAVE_order(uint8_t channel, uint8_t order){}; 	

uint8_t ETWI_read_init(uint8_t channel, uint8_t adress){
	if(channel == 0){
		TWIC.MASTER.STATUS=0x01;
		uint16_t timeout =0;
		TWIC.MASTER.ADDR = adress|0x01;
		while(!(TWIC.MASTER.STATUS & 0x80)){timeout++; if(timeout > 4000)return(1);}
		if(TWIC.MASTER.STATUS&0x04) return(2);
		if(TWIC.MASTER.STATUS&0x08) return(3); 
		return(0);	
	 }
	if(channel == 1){
		TWIE.MASTER.STATUS=0x01;
		uint16_t timeout =0;
		TWIE.MASTER.ADDR = adress|0x01;
		while(!(TWIE.MASTER.STATUS & 0x80)){timeout++; if(timeout > 4000)return(1);}
		if(TWIE.MASTER.STATUS&0x04) return(2);
		if(TWIE.MASTER.STATUS&0x08) return(3);
		return(0);
	}	 
	};
	
uint8_t ETWI_read(uint8_t channel){
		if(channel == 0){ uint16_t timeout =0;
			while(!(TWIC.MASTER.STATUS & 0x80)){timeout++; if(timeout > 4000)return(0);}
			uint8_t data = TWIC.MASTER.DATA;
			return (data);
		}
		if(channel == 1){ uint16_t timeout =0;
			while(!(TWIE.MASTER.STATUS & 0x80)){timeout++; if(timeout > 4000)return(0);}
			uint8_t data = TWIE.MASTER.DATA;
			return (data);
		}		
	};
	
uint8_t ETWI_write_init(uint8_t channel, uint8_t adress){
	if(channel == 0){
		TWIC.MASTER.STATUS=0x01;
		uint16_t timeout =0;
		TWIC.MASTER.ADDR = adress&~0x01;
	while(!(TWIC.MASTER.STATUS & 0x40)){timeout++; if(timeout > 20000)return(1);}
	if(TWIC.MASTER.STATUS&0x04) return(2);
	if(TWIC.MASTER.STATUS&0x08) return(3);	
	timeout =0;
	if(TWIC.MASTER.STATUS & 0x10)return(4);
	return(0);
	}
	if(channel == 1){
		TWIE.MASTER.STATUS=0x01;
		uint16_t timeout =0;
		TWIE.MASTER.ADDR = adress&~0x01;
		while(!(TWIE.MASTER.STATUS & 0x40)){timeout++; if(timeout > 20000)return(1);}
		if(TWIE.MASTER.STATUS&0x04) return(2);
		if(TWIE.MASTER.STATUS&0x08) return(3);
		if(TWIE.MASTER.STATUS & 0x10)return(4);
		return(0);
	}	
	}
uint8_t ETWI_write(uint8_t channel, uint8_t data){
	if(channel == 0){uint16_t timeout =0;
		TWIC.MASTER.DATA = data;
		while(!(TWIC.MASTER.STATUS & 0x40)){timeout++; if(timeout > 4000)return(1);}
	}
	if(channel == 1){uint16_t timeout =0;
		TWIE.MASTER.DATA = data;
		while(!(TWIE.MASTER.STATUS & 0x40)){timeout++; if(timeout > 4000)return(1);}
	}	
}
	/*
// ircom
void IR_init(){};
void IR_read(){};
void IR_write(){};*/
	
// I2C /TWI

	/*
// SPI
void SPI_init(){};
void SPI_read(){};
void SPI_write(){};
// TIMERS_PWM
void Timer_init(){};
void Timer_pwm(){};
//reset
void Reset_init(){};
// watchdog
void watchdog_init(){};
void Wachdog_reset(){};
//crc
void CRC_init(){};
void CRC_calc(){};
//aes
void AES_init(){};
void AEC_decryp(){};
void AEC_encryp(){};
//dma
void DMA_init(){};
//evsys
void EVENT_init(){};
*/
	/*

	
	
	
	void Interrupt_init(){	
	PMIC.CTRL|=0x07;	
}

void timer_init(){
	TCC0.CTRLA =0x01;
	TCC0.CTRLB =0x00;
	TCC0.CTRLC =0x00;
	TCC0.CTRLD =0x00;
	TCC0.CTRLE =0x00;
	TCC0.INTCTRLA =0x03;
	TCC0.PER= 3200;
	TCC0.INTFLAGS = 0x01;
	TCD0.CTRLA =0x07; // clk /1024
	TCD0.CTRLB =0x00;
	TCD0.CTRLC =0x00;
	TCD0.CTRLD =0x00;
	TCD0.CTRLE =0x00;
	TCD0.INTCTRLA =0x01;
	TCD0.PER= 8000;
	TCD0.INTFLAGS = 0x01;
}


void RTC_init (){
	VBAT.CTRL =0x0A;
	VBAT.CTRL =0x0A;
	RTC32.CTRL = 0x01;
	RTC32.PER =0xffffffff;
}

*/