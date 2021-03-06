// I/O include file for Padauk PMS232
// Version: 0.11a
// Automatically generated on 2019-09-29 16:10:55.836291

#ifndef __PDK_IO_PMS232_H
#define __PDK_IO_PMS232_H

#include <pdk/io_common.h>

#define __PDK_IOEND 127

#define PDK_USE_FACTORY_TRIMMING() {#error 'this device does not support factory trimming'}

#define FLAG    	flag // __sfr __at(0x00) FLAG
#define FPPEN   	fppen // __sfr __at(0x01) FPPEN
#define SP      	sp // __sfr __at(0x02) SP
#define CLKMD   	clkmd // __sfr __at(0x03) CLKMD
#define INTEN   	inten // __sfr __at(0x04) INTEN
#define INTRQ   	intrq // __sfr __at(0x05) INTRQ
#define T16M    	t16m // __sfr __at(0x06) T16M
#define GDIO    	gdio // __sfr __at(0x07) GDIO
#define TM2B    	tm2b // __sfr __at(0x09) TM2B
#define EOSCR   	eoscr // __sfr __at(0x0a) EOSCR
#define IHRCR   	ihrcr // __sfr __at(0x0b) IHRCR
#define INTEGS  	integs // __sfr __at(0x0c) INTEGS
#define PADIER  	padier // __sfr __at(0x0d) PADIER
#define PBDIER  	pbdier // __sfr __at(0x0e) PBDIER
#define (PCDIER)	(pcdier) // __sfr __at(0x0f) (PCDIER)
#define PA      	pa // __sfr __at(0x10) PA
#define PAC     	pac // __sfr __at(0x11) PAC
#define PAPH    	paph // __sfr __at(0x12) PAPH
#define PB      	pb // __sfr __at(0x14) PB
#define PBC     	pbc // __sfr __at(0x15) PBC
#define PBPH    	pbph // __sfr __at(0x16) PBPH
#define PC      	pc // __sfr __at(0x17) PC
#define PCC     	pcc // __sfr __at(0x18) PCC
#define PCPH    	pcph // __sfr __at(0x19) PCPH
#define RSTST   	rstst // __sfr __at(0x25) RSTST
#define TM2S    	tm2s // __sfr __at(0x37) TM2S
#define A_IHRC  	a_ihrc // __sfr __at(0x38) A_IHRC
#define BGTR    	bgtr // __sfr __at(0x39) BGTR
#define MISC    	misc // __sfr __at(0x3b) MISC
#define TM2C    	tm2c // __sfr __at(0x3c) TM2C
#define TM2CT   	tm2ct // __sfr __at(0x3d) TM2CT

__sfr __at(0x00)	flag;
__sfr __at(0x01)	fppen;
__sfr __at(0x02)	sp;
__sfr __at(0x03)	clkmd;
__sfr __at(0x04)	inten;
__sfr __at(0x05)	intrq;
__sfr __at(0x06)	t16m;
__sfr __at(0x07)	gdio;

__sfr __at(0x09)	tm2b;
__sfr __at(0x0a)	eoscr;
__sfr __at(0x0b)	ihrcr;
__sfr __at(0x0c)	integs;
__sfr __at(0x0d)	padier;
__sfr __at(0x0e)	pbdier;
__sfr __at(0x0f)	(pcdier);

__sfr __at(0x10)	pa;
__sfr __at(0x11)	pac;
__sfr __at(0x12)	paph;
__sfr __at(0x14)	pb;
__sfr __at(0x15)	pbc;
__sfr __at(0x16)	pbph;
__sfr __at(0x17)	pc;

__sfr __at(0x18)	pcc;
__sfr __at(0x19)	pcph;

__sfr __at(0x25)	rstst;


__sfr __at(0x37)	tm2s;

__sfr __at(0x38)	a_ihrc;
__sfr __at(0x39)	bgtr;
__sfr __at(0x3b)	misc;
__sfr __at(0x3c)	tm2c;
__sfr __at(0x3d)	tm2ct;









#endif
