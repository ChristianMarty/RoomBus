/*
 * BUS-Controller2.h
 *
 * Created: 15.02.2019 00:01:24
 *  Author: Christian
 */ 


#ifndef BUS_CONTROLLER2_H_
#define BUS_CONTROLLER2_H_

#include "sam.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void init_BusController2(void);

static inline void setLedGreen(bool state)
{
	if(state) PORT->Group[0].OUTCLR.reg= PORT_PA27;
	else  PORT->Group[0].OUTSET.reg = PORT_PA27;
}

static inline void setLedBlue(bool state)
{
	if(state) PORT->Group[0].OUTCLR.reg= PORT_PA01;
	else  PORT->Group[0].OUTSET.reg = PORT_PA01;
}

static inline void setLedRed(bool state)
{
	if(state) PORT->Group[0].OUTCLR.reg= PORT_PA00;
	else  PORT->Group[0].OUTSET.reg = PORT_PA00;
}

static inline void setLedRGB(bool r, bool g, bool b)
{
	setLedRed(r);
	setLedGreen(g);
	setLedBlue(b);
}


#ifdef __cplusplus
}
#endif
#endif /* BUS_CONTROLLER2_H_ */