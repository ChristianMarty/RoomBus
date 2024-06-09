/*
 * standardIO.h
 *
 * Created: 12.01.2019 21:37:40
 *  Author: Christian
 */ 

#ifdef __cplusplus
extern "C" {
#endif
	
#include "standardIO.h"

void stdIO_init(void)
{
	stdIO_redLED(false);
	stdIO_greenLED(false);
	stdIO_blueLED(false);

	PORT->Group[0].DIRSET.reg = 0x08000003; // Set pins for LED as output
	PORT->Group[0].PINCFG[31].bit.INEN = true; // enable input for Button pin
}

void stdIO_redLED(bool on)
{
	if(on) PORT->Group[0].OUTCLR.reg= PORT_PA00;
	else  PORT->Group[0].OUTSET.reg = PORT_PA00;
}

void stdIO_greenLED(bool on)
{
	if(on) PORT->Group[0].OUTCLR.reg = PORT_PA27;
	else  PORT->Group[0].OUTSET.reg = PORT_PA27;
}

void stdIO_blueLED(bool on)
{
	if(on) PORT->Group[0].OUTCLR.reg = PORT_PA01;
	else  PORT->Group[0].OUTSET.reg = PORT_PA01;
}

bool stdIO_getButton(void)
{
	return (PORT->Group[0].IN.reg & 0x80000000);
}

void stdIO_setLedRGB(bool r, bool g, bool b)
{
	stdIO_redLED(r);
	stdIO_greenLED(g);
	stdIO_blueLED(b);
}

#ifdef __cplusplus
}
#endif