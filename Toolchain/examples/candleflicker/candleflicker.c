//--------------------------------------------------------
// Electronic candle for Padauk PFS154C and SDASPDK (SDCC toolchain)
// LED is connected to PA4, PWM0 is used to drive LED
// Implements algorithm from:
// https://github.com/cpldcpu/CandleLEDhack/blob/master/Emulator/CandeflickerLED.c
//
// cpldcpu - Feb 7 2019
// Sep 21, 2019 cpldcpu - update to newer toolchain
//--------------------------------------------------------

#include <stdint.h>
#include <pdk/io_pfs154.h>

// #define candledebug

#ifdef candledebug 
// UART configuration 
#define TXPORT pa
#define TXPORTC pac
#define TXPIN 7
#define BAUDRATE 38400

#include "PDK_softuart.c"	// include softuart implementation

#endif

#define LEDPIN PA4

volatile uint16_t rnd_lfsr;
volatile uint16_t lowpass;

unsigned char _sdcc_external_startup(void)
{
	CLKMD = CLKMD_IHRC_DIV16 | CLKMD_ENABLE_IHRC; // 1 Mhz main clock
	PDK_USE_FACTORY_TRIMMING();
#ifdef candledebug 
	PDK_autobaud();
#endif
	return 0;
}

void lfsr_step()
{
	if (rnd_lfsr & 1)
	{
		rnd_lfsr = (rnd_lfsr >> 1) ^ 0x822B;
	}
	else
	{
		rnd_lfsr = (rnd_lfsr >> 1);
	}
}

void candle_init()
{
	PWMG1C = 0x87; // Enable PWMG1, set to PA4, src=IHR
	PWMG1S = 0x20; // prescaler=4, divider=1, no interrupt

	PWMG1CUBL = 0xff; // set PWM counter upper bound to 0xffff
	PWMG1CUBH = 0xff;

	rnd_lfsr = 0x55ce;
	lowpass = 0;
}

void candle_do()
{
	uint16_t newval;
	if (rnd_lfsr & 0x100)
	{
		newval = 255;
	}
	else
	{
		newval = (uint8_t)(rnd_lfsr & 255);
	}

	// lowpass = newval<<8;   // no filter
	lowpass = lowpass - (lowpass>>1) + (newval<<7);   // IIR filter with lag 2 (recommended)
	// lowpass = lowpass - (lowpass>>2) + (newval<<6);   // IIR filter with lag 4 (less flicker)
	// lowpass = lowpass - (lowpass>>3) + (newval<<5);   // IIR filter with lag 8 (even less flicker)

#ifdef candledebug 
	PDK_sendchar('\n');
	PDK_senduint16(lowpass);
#endif

	PWMG1DTL = lowpass&255;
	PWMG1DTH = lowpass>>8;

	for (char i = 0; i < 3; i++)
	{
		lfsr_step();
		if ((rnd_lfsr & 0xff) > 128)
			return;
	}
}

void main(void)
{
	candle_init();

	T16M = T16_CLK_DIV16 | T16_CLK_SYSCLK | T16_INTSRC_11BIT; // 1 MHZ / 16 / (2^11) = 30 Hz
	INTEN |= INTEN_T16;
	INTRQ = 0; 

	__asm__("engint"); // enable interrupts
	
	for (;;)
	{
	}
}

void handler(void) __interrupt(0)
{
	if (INTRQ & INTRQ_T16)
	{
		candle_do();
		intrq &= ~INTRQ_T16;
	}
}
