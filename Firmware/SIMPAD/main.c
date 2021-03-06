/*
	Simple Paudak Programmer V0.01
	Routines to program the Padauk PMS150C, PFS154C, PMS154C MCUs.

	Jan 2019 cpldpcu
	
	Uses Arduino Nano (ATMega168PA)
	
	
	This is still in a very early stage. No host communication to a PC has been implemented yet.
*/

#include <atmel_start.h>
#include <stdio.h>
#include <util/delay.h>
#include "adc_basic.h"
#include "pwm_basic.h"

/*
		  6-wire    5-wire
	PWM = VPP     = ICVPP
	PB0 = VDD     = VDD  
	PB1 = MISO    = ICPDA      (data output of MCU or bidirectional)
	PB2 = MOSI    = Not used   (data input of MCU)
	PB3 = SCLK    = ICPCK 
	
	PA6 = ADC6 = voltage monitor input (5k1/10k divider)
*/

#define P_VDD  PB0
#define P_MISO PB1
#define P_MOSI PB2
#define P_SCLK PB3

#define P_ICPDA  PB1
#define P_ICPCK  PB3

static uint8_t cur_dutycycle=0;

typedef enum {fivewire, sixwire,eightwire} physical_interface;
/*
// PMS150C
static const physical_interface device_phy=sixwire;
static const uint8_t device_datalen=13;
static const uint8_t device_adrlen=12;
static const uint16_t device_vpp_read=(uint16_t)(70.7*7.5);  // 7.5 V
static const uint16_t device_vpp_write=(uint16_t)(70.7*10.5);  // 11 V
static const uint16_t device_vpp_erase=(uint16_t)(70.7*8.0);  // 8.0 V
static const uint16_t device_memend=0x3ff;
static const uint16_t device_id=0xa16;
static const char device_string[]="PMS150C";
*/

// PFC154
static const physical_interface device_phy=fivewire;
static const uint8_t device_datalen=14;
static const uint8_t device_adrlen=13;
static const uint16_t device_vpp_read=(uint16_t)(70.7*7.5);  // 7.5 V
static const uint16_t device_vpp_write=(uint16_t)(70.7*7.5);  // 7.5 V
static const uint16_t device_vpp_erase=(uint16_t)(70.7*8.0);  // 8.0 V
static const uint16_t device_memend=0x7ff;
static const uint16_t device_id=0xaa1;
static const char device_string[]="PFS154";

/*
// PMS154C
static const physical_interface device_phy=sixwire;
static const uint8_t device_datalen=14;
static const uint8_t device_adrlen=12;
static const uint16_t device_vpp_read=(uint16_t)(70.7*7.5);  // 7.5 V
static const uint16_t device_vpp_write=(uint16_t)(70.7*11);  // 11 V
static const uint16_t device_memend=0x7ff;
static const uint16_t device_id=0xe06;
static const char device_string[]="PMS154C";
*/


/*
Initialize ports and turn MCU off.
*/
void SP_init(void) {

	PWM_0_enable_output_ch0();
	PWM_0_load_duty_cycle_ch0(0x00);
	PWM_0_load_counter(0);
	cur_dutycycle=0;

	PORTB_set_pin_dir(P_VDD,PORT_DIR_IN);
	PORTB_set_pin_level(P_VDD,true);

	PORTB_set_pin_dir(P_SCLK,PORT_DIR_OUT);
	PORTB_set_pin_level(P_SCLK,false);

	if (device_phy==fivewire) {
		PORTB_set_pin_dir(P_ICPDA,PORT_DIR_IN);
		PORTB_set_pin_level(P_ICPDA,false);
		PORTB_set_pin_dir(P_MOSI,PORT_DIR_IN);		
	} else 	{
		PORTB_set_pin_dir(P_MISO,PORT_DIR_IN);
		PORTB_set_pin_dir(P_MOSI,PORT_DIR_OUT);
		PORTB_set_pin_level(P_MOSI,false);
	}

	_delay_ms(1);
}

/*
Ramp VPP to target voltage. Needs to be replaced with better controller

adctarget=Vpp*70.06   (Aref=5V, Rdivier 5k1/10k)
*/
void SP_RampVpp(uint16_t adctarget) {

	uint16_t adcout;

	adcout = ADC_0_get_conversion(6);
	if (adcout>=adctarget) return;
	/*
	if (pwmstart) {
	cur_dutycycle = pwmstart;
	PWM_0_load_duty_cycle_ch0(cur_dutycycle);
	_delay_ms(10);
	}
	*/

	adcout = ADC_0_get_conversion(6);
	if (adcout>=adctarget) {
		printf("VPP already above target! (last Vpp:%i but target was:%i)!!\n",adcout,adctarget);
		return;
	}

	do {
		cur_dutycycle +=1;
		PWM_0_load_duty_cycle_ch0(cur_dutycycle);
		_delay_ms(5);
		adcout = ADC_0_get_conversion(6);
		if (cur_dutycycle==255) {
			PWM_0_load_duty_cycle_ch0(0);
			printf("VPP RAMP OVERFLOW (last Vpp:%i but target was:%i)!!\n",adcout,adctarget);
			return;
		}
	} while(adcout<adctarget);

	return;
}

/*
Send out up to 16 bits on MOSI.
MSB first
"length" has number of bits to send
*/

void SP_SendWord(uint16_t word, uint8_t length) {
	uint8_t i;
	word=word<<(16-length);
	
	if (device_phy==fivewire) {
		PORTB_set_pin_dir(P_ICPDA,PORT_DIR_OUT);
		for (i=0; i<length; i++) {
			PORTB_set_pin_level(P_ICPCK,false);
			PORTB_set_pin_level(P_ICPDA,word&0x8000);
			word<<=1;
			_delay_us(3);
			PORTB_set_pin_level(P_ICPCK,true);
			_delay_us(3);
		}
		PORTB_set_pin_level(P_ICPCK,false);
		PORTB_set_pin_dir(P_ICPDA,PORT_DIR_IN);
	} else {
		for (i=0; i<length; i++) {
			PORTB_set_pin_level(P_SCLK,false);
			PORTB_set_pin_level(P_MOSI,word&0x8000);
			word<<=1;
			_delay_us(3);
			PORTB_set_pin_level(P_SCLK,true);
			_delay_us(3);
		}
		PORTB_set_pin_level(P_SCLK,false);
	}
}

/*
Receive up to 16 bits on MISO.
MSB first
"length" has number of bits to send
*/

uint16_t SP_ReceiveWord(uint8_t length) {
	uint8_t i;
	uint16_t word=0;

	if (device_phy==fivewire) {
		for (i=0; i<length; i++) {
			PORTB_set_pin_level(P_ICPCK,false);
			_delay_us(3);
			PORTB_set_pin_level(P_ICPCK,true);
			_delay_us(2);
			word<<=1;
			if (PORTB_get_pin_level(P_ICPDA)) word|=0x1;   // Read after delay to fix for first bit issue.
			_delay_us(1);
		}
		PORTB_set_pin_level(P_ICPCK,false);
		_delay_us(2);
		PORTB_set_pin_level(P_ICPCK,true);   // reads have one additional clock cycle for the bidirectional data bus. Direction reversal cycle, so the slave can switch off the driver?
		_delay_us(2);
		PORTB_set_pin_level(P_ICPCK,false);
		_delay_us(2);
	} else {
		for (i=0; i<length; i++) {
			PORTB_set_pin_level(P_SCLK,false);
			_delay_us(3);
			PORTB_set_pin_level(P_SCLK,true);
			_delay_us(2);
			word<<=1;
			if (PORTB_get_pin_level(P_MISO)) word|=0x1;   // Read after delay to fix for first bit issue.
			_delay_us(1);
		}
		PORTB_set_pin_level(P_SCLK,false);
	}
	return word;
}

/*
Enter programming mode.
Command=A6 for reading, A7 for writing
Returns response code/deviceid for fivewire devices
*/

uint16_t SP_EnterPGMmode(uint8_t command) {
	SP_init();
	
	PORTB_set_pin_dir(P_VDD,PORT_DIR_OUT);	  //
	PORTB_set_pin_level(P_VDD,false);	  // discharge VDD cap.
	_delay_us(1000);
	PORTB_set_pin_dir(P_VDD,PORT_DIR_IN);	  // High-Z

	SP_RampVpp(device_vpp_read);
	
	PORTB_set_pin_dir(P_VDD,PORT_DIR_OUT);	  // turn on VDD
	PORTB_set_pin_level(P_VDD,false);	  // reset
	_delay_us(500);
	PORTB_set_pin_level(P_VDD,true);	  //

	_delay_us(500);
	SP_SendWord(0xA5A5,16);
	SP_SendWord(0xA5A5,8);
	SP_SendWord(command,8);
	
	if (device_phy==fivewire) {
		SP_SendWord(0x0,3);  // Send three fillbits
		return SP_ReceiveWord(13)&0xfff;
		} else {
		return 0;
	}
}

/*
Read memory word
Attention: Read mode needs to be active
*/

uint16_t SP_ReadWord(uint16_t address) {
	SP_SendWord(address,device_adrlen);
	return SP_ReceiveWord(device_datalen);
}

/*
Write memory word. Memory is written in pairs of two words.
Attention: Write mode needs to be active
*/

void SP_WriteWords(uint16_t word1,uint16_t word2, uint16_t address) {
	
	if (device_phy==fivewire) {
		if (address&0x0002) {   // write either upper or lower part of page at the same time.
			SP_SendWord(0xffff,device_datalen);
			SP_SendWord(0xffff,device_datalen);
			SP_SendWord(word1,device_datalen);
			SP_SendWord(word2,device_datalen);
		} else {
			SP_SendWord(word1,device_datalen);
			SP_SendWord(word2,device_datalen);
			SP_SendWord(0x0ffff,device_datalen);
			SP_SendWord(0x0ffff,device_datalen);
		}
		SP_SendWord(address&0xfffc,device_adrlen);

		// Write execution sequence (slow clock on SCLK)
		//SP_SendWord(0,1);  // Leading zero -> is part of address
		PORTB_set_pin_dir(P_ICPDA,PORT_DIR_OUT);
		_delay_us(1);
		PORTB_set_pin_level(P_ICPDA,true);   // Toggle data line. Not sure if this is needed, but this happens in the original programmer
		_delay_us(1);
		PORTB_set_pin_level(P_ICPDA,false);
		_delay_us(1);
		for (uint8_t i=0; i<8; i++) {
			PORTB_set_pin_level(P_SCLK,true);
			_delay_us(22);
			PORTB_set_pin_level(P_SCLK,false);
			_delay_us(22);
		}
		_delay_us(1);
		PORTB_set_pin_dir(P_ICPDA,PORT_DIR_IN);
		SP_SendWord(0,1);  // Trailing zero
		
		} else {
		SP_SendWord(word1,device_datalen);			// Write two words in consecutive steps to improve reliability
		SP_SendWord(0xffff,device_datalen);
		SP_SendWord(address,device_adrlen);

		// Write execution sequence (stretched SCLK cycle and slow clock on MOSI)
		SP_SendWord(0,1);  // Leading zero
		PORTB_set_pin_level(P_SCLK,true);
		_delay_us(1);
		for (uint8_t i=0; i<8; i++) {
			PORTB_set_pin_level(P_MOSI,true);
			_delay_us(31);
			PORTB_set_pin_level(P_MOSI,false);
			_delay_us(31);
		}
		PORTB_set_pin_level(P_SCLK,false);
		_delay_us(1);
		SP_SendWord(0,1);  // Trailing zero

		SP_SendWord(0xffff,device_datalen);
		SP_SendWord(word2,device_datalen);
		SP_SendWord(address,device_adrlen);

		// Write execution sequence (stretched SCLK cycle and slow clock on MOSI)
		SP_SendWord(0,1);  // Leading zero
		PORTB_set_pin_level(P_SCLK,true);
		_delay_us(1);
		for (uint8_t i=0; i<8; i++) {
			PORTB_set_pin_level(P_MOSI,true);
			_delay_us(31);
			PORTB_set_pin_level(P_MOSI,false);
			_delay_us(31);
		}
		PORTB_set_pin_level(P_SCLK,false);
		_delay_us(1);
		SP_SendWord(0,1);  // Trailing zero

	}
}

/*
	Read Device ID Sequence
*/

uint16_t SP_ReadDeviceIDSequence(void) {
	
	uint16_t DeviceID;
	DeviceID=SP_EnterPGMmode(0xA7);   // Activate write mode
	
	if (device_phy==sixwire) {
		SP_SendWord(0x0000,device_datalen);  // Send Dummydata 2x13 bits
		SP_SendWord(0x0000,device_datalen);
		_delay_us(3);
		DeviceID=SP_ReceiveWord(device_adrlen);  // Receive 12 bit Device ID during address phase
		_delay_us(20);
	}

	SP_init();  // Turn MCU off again.
	_delay_ms(10);
	
	return DeviceID;
}

/*
	Erase flash based device.
*/
void SP_EraseDevice(void) {

	uint16_t DeviceID,adcout;
	DeviceID=SP_EnterPGMmode(0xA3);   // Activate erase mode
	printf("Erase init response: %.4X\n",DeviceID);

	adcout = ADC_0_get_conversion(6);
	printf("Vpp initial: %.1f V\tPWM: %i\n",(float)adcout*0.01427f,cur_dutycycle);
	SP_RampVpp(device_vpp_erase);  // Ramp VPP to write voltage
	_delay_ms(10);  // stabilize
	adcout = ADC_0_get_conversion(6);
	printf("Vpp erase mode: %.1f V\tPWM: %i\n",(float)adcout*0.01427f,cur_dutycycle);
	
	for (uint8_t i=0; i<2; i++) {
		PORTB_set_pin_level(P_SCLK,true);
		_delay_us(5000);
		PORTB_set_pin_level(P_SCLK,false);
		_delay_us(2);
		PORTB_set_pin_level(P_SCLK,true);
		_delay_us(2);
		PORTB_set_pin_level(P_SCLK,false);
		_delay_us(2);
	}

	adcout = ADC_0_get_conversion(6);
	printf("Vpp after erase: %.1f V\tPWM: %i\n",(float)adcout*0.01427f,cur_dutycycle);

	SP_init();  // Turn MCU off again.
	_delay_ms(10);
}

/*
Resets and starts the MCU to execute the user program
*/
void SP_StartMCU(void) {
	uint16_t adcout;
	
	SP_init();
	_delay_ms(20);
	
	adcout = ADC_0_get_conversion(6);
	printf("Vpp before starting MCU: %.1f V\n",(float)adcout*0.01427f);

	PORTB_set_pin_dir(P_MISO,PORT_DIR_IN);	  // all connections to input
	PORTB_set_pin_dir(P_MOSI,PORT_DIR_IN);
	PORTB_set_pin_dir(P_SCLK,PORT_DIR_IN);

	PORTB_set_pin_dir(P_VDD,PORT_DIR_OUT);	  // turn on VDD
	PORTB_set_pin_level(P_VDD,false);		  // reset
	_delay_us(500);
	PORTB_set_pin_level(P_VDD,true);	  //
	printf("MCU is active now.\n");
}

/*
	Receive and transmit SPI from programming interface.
	ICPCK and ICPDA are CLK and MOSI, respecively. The programmer is SPI slave
*/

void SPIMonitor(void) {
	uint16_t shiftreg=0;
	uint8_t  bitcount=0;
	uint16_t timeout=0x10000;
	bool	 lastclk=true;

	PORTB_set_pin_dir(P_ICPDA,PORT_DIR_IN);	  // all connections to input
	PORTB_set_pin_dir(P_ICPCK,PORT_DIR_IN);

	
	while(1) {
		bool newclk=PORTB_get_pin_level(P_ICPCK);
		
		if (newclk&&!lastclk) {  // rising edge detected
			shiftreg=(shiftreg<<1)|(PORTB_get_pin_level(P_ICPDA)&1);
			bitcount++;
			timeout=0xffff;
			
			if (bitcount==16) {
				printf("%u\n",shiftreg);
				bitcount=0;
				shiftreg=0;						
			}
		}
							
		if (!timeout--) {
			timeout=0xffff;
			bitcount=0;
			shiftreg=0;			
		}				
		
		lastclk=newclk;		
	}	
}

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	printf("Initializing...\n");
	uint16_t adcout,DeviceID;

	DeviceID=SP_ReadDeviceIDSequence();
	adcout = ADC_0_get_conversion(6);
	printf("DeviceID: %X \tVpp Standby: %.1f V\n",DeviceID,(float)adcout*0.01427f);
	if (DeviceID==device_id) {
		printf("DeviceID is match to: %s\n",device_string);
		} else {
		printf("DeviceID mismatch. Aborting!\n");
		while(1);
	}

	_delay_ms(1);
	SP_init();
	_delay_ms(20);

	printf("Dumping memory...\n");
	SP_EnterPGMmode(0xA6);  // Enter read mode
	
	adcout = ADC_0_get_conversion(6);
	printf("Vpp read mode: %.1f V\n",(float)adcout*0.01427f);

	uint16_t address;
	uint8_t i;
	
	for (address=0x000; address<device_memend; address+=16) {
		printf("%.4X: ",address);
		for (i=0; i<16; i++) {
			printf("%.4X ",SP_ReadWord(address+i));
		}
		printf ("\n");
	}
	SP_init();
	_delay_ms(20);

	printf("Erasing device...\n");
	SP_EraseDevice();
	
	printf("Writing to memory...\n");
	SP_EnterPGMmode(0xA7);  // Enter write mode

	adcout = ADC_0_get_conversion(6);
	printf("Vpp initial: %.1f V\tPWM: %i\n",(float)adcout*0.01427f,cur_dutycycle);
	SP_RampVpp(device_vpp_write);  // set Vpp pgm voltage
	_delay_ms(10);  // stabilize
	adcout = ADC_0_get_conversion(6);
	printf("Vpp write mode: %.1f V\tPWM: %i\n",(float)adcout*0.01427f,cur_dutycycle);

	// blinkc for PFS154
	SP_WriteWords(0x0000,0x1301,0x0000);
	SP_WriteWords(0x2F00,0x2801,0x0002);
	SP_WriteWords(0x006A,0x006B,0x0004);
	SP_WriteWords(0x0182,0x381D,0x0006);
	SP_WriteWords(0x3012,0xFFFF,0x0008);
	SP_WriteWords(0x007B,0x3021,0x0010);
	SP_WriteWords(0x2F00,0x0B80,0x0012);
	SP_WriteWords(0x2F00,0x2800,0x0014);
	SP_WriteWords(0x1780,0x301C,0x0016);
	SP_WriteWords(0x2F00,0x0380,0x0018);
	SP_WriteWords(0x1200,0x3014,0x001A);
	SP_WriteWords(0x3011,0x2F78,0x001C);
	SP_WriteWords(0x0183,0x0200,0x001E);
	SP_WriteWords(0x007A,0x2F01,0x0020);
	SP_WriteWords(0x0191,0x2F30,0x0022);
	SP_WriteWords(0x0B80,0x2F75,0x0024);
	SP_WriteWords(0x0B81,0x1280,0x0026);
	SP_WriteWords(0x1081,0x0F80,0x0028);
	SP_WriteWords(0x0E81,0x2A00,0x002A);
	SP_WriteWords(0x3027,0x2F00,0x002C);
	SP_WriteWords(0x0190,0x2F30,0x002E);
	SP_WriteWords(0x0B80,0x2F75,0x0030);
	SP_WriteWords(0x0B81,0x1280,0x0032);
	SP_WriteWords(0x1081,0x0F80,0x0034);
	SP_WriteWords(0x0E81,0x2A00,0x0036);
	SP_WriteWords(0x3033,0x2F01,0x0038);
	SP_WriteWords(0x0190,0x3023,0x003A);
	SP_WriteWords(0x007A,0xFFFF,0x003C);


/*
	for (uint8_t i=0; i<0x60; i+=2) {
		SP_WriteWords(0x0000,0x0000,i);
		
	}
*/
	
	adcout = ADC_0_get_conversion(6);
	printf("Vpp after writing: %.1f V\n",(float)adcout*0.01427f);
	
	SP_StartMCU();
	
//	SPIMonitor();
	while(1);
}
