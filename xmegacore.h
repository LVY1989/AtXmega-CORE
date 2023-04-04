/* UWAGA! biblioteka przeznaczona dla procesora z rodziny atxmegaXXXA3U TYLKO!*/

#ifndef xmegacore_h
#define xmegacore_h
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <avr/wdt.h>

//// moduły zegara
void Osc32MHz(void); 								/* procek bedzie pracowal na 32 MHZ*/

void Osc2MHz(void);									/* procek bedzie pracowal na 2 MHZ 	*/

void OscXtal(void);									/* procek bedzie pracowal na zewnetrznym kwarcu */

													/* procek bedzie pracowal w pętli pll, gdzie factor to mnoznik 0-31 
													source 0: generator 2  MHZ 
													source 1: generator 32 MHZ */
void OscPLL(uint8_t pllfactor, uint8_t CSource); 
													
													
uint8_t ReadCalibrationByte(uint8_t index);			/* do odczytywania bitów kalibracyjnych */
													/* source; 		0: 1V, 			1: VCC/1,6, 	2: AREFA,  	3: AREFB, 4:VCC/2
													 conv mode; 	0: bez znaku,  	1: ze znakiem
													 freemode; 	0: jednorazowo	1: tryb ciagly
													 prescaler: 	0: /4	1: /8 	2: /16	3: /32	4: /64 	5: /128	6: /256	7: /512 */
void ADCA_INIT(uint8_t source, uint8_t conv_mode, uint8_t freemode, uint8_t prescaler, uint8_t analogpin);

void ADCA_DISABLE();

void ADCACH0_start();
uint8_t ADCACH0_ready();
													/* ready 0: read without checking ready, 1: read register */
int16_t ADCACH0_read(uint8_t ready);
													/* gain: 0: 1x, 1:2x, 2:4x, 3:8x, 4:16x, 5:32x, 6:64x, 7:1/2x;
													 inputmode : 0: internal, 1: singleended, 2:differential, 3: differential with gain
													 posend in external mode: PA0, PA1... PB6, PB7; 
													 posend in internal mode: 0:temperature, 1: bandgap, 2: 1/10 VCC 3: DAC 
													 negend: watch in config! */
void ADCACH0_init(uint8_t gain, uint8_t inputmode, uint8_t posend, uint8_t negend);				



void ADCACH1_start();
uint8_t ADCACH1_ready();
													/* ready 0: read without checking ready, 1: read register */
int16_t ADCACH1_read(uint8_t ready);
													/* gain: 0: 1x, 1:2x, 2:4x, 3:8x, 4:16x, 5:32x, 6:64x, 7:1/2x;
													 inputmode : 0: internal, 1: singleended, 2:differential, 3: differential with gain
													 posend in external mode: PA0, PA1... PB6, PB7; 
													 posend in internal mode: 0:temperature, 1: bandgap, 2: 1/10 VCC 3: DAC 
													 negend: watch in config! */
void ADCACH1_init(uint8_t gain, uint8_t inputmode, uint8_t posend, uint8_t negend);		


void ADCACH2_start();
uint8_t ADCACH2_ready();
													/* ready 0: read without checking ready, 1: read register */
int16_t ADCACH2_read(uint8_t ready);
													/* gain: 0: 1x, 1:2x, 2:4x, 3:8x, 4:16x, 5:32x, 6:64x, 7:1/2x;
													 inputmode : 0: internal, 1: singleended, 2:differential, 3: differential with gain
													 posend in external mode: PA0, PA1... PB6, PB7; 
													 posend in internal mode: 0:temperature, 1: bandgap, 2: 1/10 VCC 3: DAC 
													 negend: watch in config! */
void ADCACH2_init(uint8_t gain, uint8_t inputmode, uint8_t posend, uint8_t negend);		


void ADCACH3_start();
uint8_t ADCACH3_ready();
													/* ready 0: read without checking ready, 1: read register */
int16_t ADCACH3_read(uint8_t ready);
													/* gain: 0: 1x, 1:2x, 2:4x, 3:8x, 4:16x, 5:32x, 6:64x, 7:1/2x;
													 inputmode : 0: internal, 1: singleended, 2:differential, 3: differential with gain
													 posend in external mode: PA0, PA1... PB6, PB7; 
													 posend in internal mode: 0:temperature, 1: bandgap, 2: 1/10 VCC 3: DAC 
													 negend: watch in config! */
void ADCACH3_init(uint8_t gain, uint8_t inputmode, uint8_t posend, uint8_t negend);									




													/* source; 		0: 1V, 			1: VCC/1,6, 	2: AREFA,  	3: AREFB, 4:VCC/2
													 conv mode; 	0: bez znaku,  	1: ze znakiem
													 freemode; 	0: jednorazowo	1: tryb ciagly
													 prescaler: 	0: /4	1: /8 	2: /16	3: /32	4: /64 	5: /128	6: /256	7: /512 */
void ADCB_INIT(uint8_t source, uint8_t conv_mode, uint8_t freemode, uint8_t prescaler, uint8_t analogpin);

void ADCB_DISABLE();

void ADCBCH0_start();
uint8_t ADCBCH0_ready();
													/* ready 0: read without checking ready, 1: read register */
int16_t ADCBCH0_read(uint8_t ready);
													/* gain: 0: 1x, 1:2x, 2:4x, 3:8x, 4:16x, 5:32x, 6:64x, 7:1/2x;
													 inputmode : 0: internal, 1: singleended, 2:differential, 3: differential with gain
													 posend in external mode: PA0, PA1... PB6, PB7; 
													 posend in internal mode: 0:temperature, 1: bandgap, 2: 1/10 VCC 3: DAC 
													 negend: watch in config! */
void ADCBCH0_init(uint8_t gain, uint8_t inputmode, uint8_t posend, uint8_t negend);				



void ADCBCH1_start();
uint8_t ADCBCH1_ready();
													/* ready 0: read without checking ready, 1: read register */
int16_t ADCBCH1_read(uint8_t ready);
													/* gain: 0: 1x, 1:2x, 2:4x, 3:8x, 4:16x, 5:32x, 6:64x, 7:1/2x;
													 inputmode : 0: internal, 1: singleended, 2:differential, 3: differential with gain
													 posend in external mode: PA0, PA1... PB6, PB7; 
													 posend in internal mode: 0:temperature, 1: bandgap, 2: 1/10 VCC 3: DAC 
													 negend: watch in config! */
void ADCBCH1_init(uint8_t gain, uint8_t inputmode, uint8_t posend, uint8_t negend);		


void ADCBCH2_start();
uint8_t ADCBCH2_ready();
													/* ready 0: read without checking ready, 1: read register */
int16_t ADCBCH2_read(uint8_t ready);
													/* gain: 0: 1x, 1:2x, 2:4x, 3:8x, 4:16x, 5:32x, 6:64x, 7:1/2x;
													 inputmode : 0: internal, 1: singleended, 2:differential, 3: differential with gain
													 posend in external mode: PA0, PA1... PB6, PB7; 
													 posend in internal mode: 0:temperature, 1: bandgap, 2: 1/10 VCC 3: DAC 
													 negend: watch in config! */
void ADCBCH2_init(uint8_t gain, uint8_t inputmode, uint8_t posend, uint8_t negend);		


void ADCBCH3_start();
uint8_t ADCBCH3_ready();
													/* ready 0: read without checking ready, 1: read register */
int16_t ADCBCH3_read(uint8_t ready);
													/* gain: 0: 1x, 1:2x, 2:4x, 3:8x, 4:16x, 5:32x, 6:64x, 7:1/2x;
													 inputmode : 0: internal, 1: singleended, 2:differential, 3: differential with gain
													 posend in external mode: PA0, PA1... PB6, PB7; 
													 posend in internal mode: 0:temperature, 1: bandgap, 2: 1/10 VCC 3: DAC 
													 negend: watch in config! */
void ADCBCH3_init(uint8_t gain, uint8_t inputmode, uint8_t posend, uint8_t negend);									

													/*source: 0: internal 1V, 1: AVcc, 2: AREFA. 3: AREFB 
													mode0: 0: disabled, 1: enabled on pin, 2: enabled for internal comparator
													mode1: 0: disabled, 1: enabled on pin
													low_pwr: 0: disabled, 1: enabled
													leftadj: 0: normal data, 1: left adjusted data
													*/
void DAC_init(uint8_t source,  uint8_t mode0, uint8_t mode1, uint8_t low_pwr, uint8_t leftadj);
 
													/*kanal: 0: channel0 1: channel1 
													wartosc: value
													return 0: when writing is completed
													return 1: when cannot write 
													*/ 
uint8_t DAC_write(uint8_t kanal, uint16_t wartosc);
 													/*channel: 0:  ACA0 1: ACA1, 2: ACB0, 3: ACB1 
													int_mode: 0: jakakolwiek zmiana stanu, 1: zmiana na zero, 2: zmiana na 1 
													int_lvl: poziom przerwania (0-3)
													speed: 0: normal speed 1:high speed (more current)
													hysteresis: 0: none, 1:small, 2:large
													muxpos: 0:pin0 ~ 6:pin6, 7: DAC  
													muxneg: 0:pin0, 1:pin1, 2:pin3, 3:pin5, 4:pin7, 5:DAC, 6:BANDGAP, 7:VCC_scaller
													outpin: 0: AC internal output, 1: output on pin
													scalefactor: 0-63, divides the vcc for negative input
													windowmode: 0:disable, 1:above window, 2:inside window, 3:below window, 4:outside window
													int_window: imterrupt level for window interrupts (0-3)
													current: not implemented
													*/
void AC_init(uint8_t channel, uint8_t int_mode, uint8_t int_lvl, uint8_t speed, uint8_t hysteresis,uint8_t muxpos, uint8_t muxneg, uint8_t outpin, uint8_t scalefactor, uint8_t windowmode,  uint8_t int_window, uint8_t current );
													/*will disable the selected channel
													*/ 
void AC_disable(uint8_t channel);

													/*
													returns the output state of channels 0-3
													4: return output state on ACA window
													5: return output state on ACB window
													*/ 
uint8_t AC_state(uint8_t channel);
													/*
													channel: 0: C0, 1: C1, 2: D0,  3: D1, 4: E0, 5: F0
													char_mode: 0:5bit, 1:6bit, 2:7bit, 3:8bit(default), 4:9bit
													work_mode: 0:asynchronus, 1:synchronus, 2:IRCOM, SPI NOT IMPLEMENTED 
													mode_io: 0:all disabled, 1:rx enable, 2:tx enable, 3: both enable
													parity: 0: disable, 1:even, 2: odd
													stopbit: 0: 1 stopbit, 1: 2 stopbit
													bautrate: write used baudrate
													fcpu: frequency of cpu used in Hz
													int_tx : level of TX READY interrupt (0 ~3)
													int_rx : level of RX READY interrupt (0 ~3)
													int_ready : level of DATA READY interrupt (0 ~3)
													*/ 
void UART_init(uint8_t channel,uint8_t char_mode, uint8_t work_mode,uint8_t mode_io, uint8_t parity, uint8_t stopbit,   uint32_t baudrate, uint32_t fcpu, uint8_t int_tx, uint8_t int_rx ,uint8_t int_ready);
													/*
													channel: 0: C0, 1: C1, 2: D0,  3: D1, 4: E0, 5: F0
													1 on fast means that data is read without waiting for data ready 	
													returns an data according to used mode in initiation of uart
													*/ 
uint16_t Uart_read(uint8_t channel, uint8_t fast);
													/*
													channel: 0: C0, 1: C1, 2: D0,  3: D1, 4: E0, 5: F0
													data: data to transmit
													1 on fast means that data is write without waiting for data send ready 	
													returns 0 if data send completed, 0x80 if data was not send correctly
													*/ 
uint8_t Uart_write(uint8_t channel, uint16_t data, uint8_t fast);
													/*
													channel: 0: TWIC, 1: TWIE
													Intmode: 0:noint, 1:write only, 2: read only 3: both
													int lvl: 0 -3
													baudrate: bus freq
													fcpu: clock freq
													timeout: timeout on line 0:no (standard i2c) 1:50ns, 2:300ns, 3:400ns
													*/ 
void ETWI_MASTER_init(uint8_t channel, uint8_t int_mode,uint8_t int_lvl, uint32_t baudrate, uint32_t fcpu, uint8_t timeout);
													/*
													channel: 0: TWIC, 1: TWIE
													order: 0: START condition, 1: byterec ack, 2: bytrec nack, 3: STOP condtion
													*/ 
void ETWI_MASTER_order(uint8_t channel, uint8_t order);
													/*
													not implemented yet
													*/ 
void ETWI_SLAVE_init (uint8_t channel, uint8_t int_mode ,uint8_t int_lvl, uint8_t workmode, uint8_t adres, uint8_t adresmask );
													/*
													not implemented yet
													*/ 
void ETWI_SLAVE_order(uint8_t channel, uint8_t order);	
													/*
													channel: 0: TWIC, 1: TWIE
													adress: adress of device
													returns: 0: ok, 1: adress error, 2:buss error, 3:arbitration lost
													*/ 
uint8_t ETWI_read_init(uint8_t channel, uint8_t adress);
													/*
													channel: 0: TWIC, 1: TWIE
													returns data
													*/ 
uint8_t ETWI_read(uint8_t channel);			
													/*
													channel: 0: TWIC, 1: TWIE
													adress: adress of device
													returns: 0: ok, 1: adress error, 2:buss error, 3:arbitration lost, 4: slave do not ack!
													*/ 
uint8_t ETWI_write_init(uint8_t channel, uint8_t adress);
													/*
													channel: 0: TWIC, 1: TWIE
													data: data to write
													*/ 
uint8_t ETWI_write(uint8_t channel, uint8_t data);

#endif