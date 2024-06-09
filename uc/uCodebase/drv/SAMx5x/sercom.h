/*
 * sercom.h
 *
 *  Author: Christian
 */ 


#ifndef SERCOM_H_
#define SERCOM_H_

#include "sam.h"
#include "genericClockController.h"
#include <stdbool.h>

typedef enum {
	sercom_parity_none,
	sercom_parity_even,
	sercom_parity_odd
}sercom_parity_t;

static inline void sercom_initClock(gclk_generator_t clkGenerator, const Sercom *sercom)
{
	// Setup clock
	if(sercom == SERCOM0)
	{
		gclk_peripheralEnable(GCLK_SERCOM0_CORE,clkGenerator);
		MCLK->APBAMASK.bit.SERCOM0_ = true;
	}
	else if(sercom == SERCOM1)
	{
		gclk_peripheralEnable(GCLK_SERCOM1_CORE,clkGenerator);
		MCLK->APBAMASK.bit.SERCOM1_ = true;
	}
	else if(sercom == SERCOM2)
	{
		gclk_peripheralEnable(GCLK_SERCOM2_CORE,clkGenerator);
		MCLK->APBBMASK.bit.SERCOM2_ = true;
	}
	else if(sercom == SERCOM3)
	{
		gclk_peripheralEnable(GCLK_SERCOM3_CORE,clkGenerator);
		MCLK->APBBMASK.bit.SERCOM3_ = true;
	}
	else if(sercom == SERCOM4)
	{
		gclk_peripheralEnable(GCLK_SERCOM4_CORE,clkGenerator);
		MCLK->APBDMASK.bit.SERCOM4_ = true;
	}
	else if(sercom == SERCOM5)
	{
		gclk_peripheralEnable(GCLK_SERCOM5_CORE,clkGenerator);
		MCLK->APBDMASK.bit.SERCOM5_ = true;
	}
}

static inline void sercom_setBaudRate( Sercom *sercom_p, uint32_t clockFrequency, uint32_t baud)
{
	uint8_t samplesPerBit = 16;
	sercom_p->USART.CTRLA.bit.ENABLE = false;
	sercom_p->USART.BAUD.reg = (65536*(float)(1-(float)((float)samplesPerBit*(float)baud/(float)clockFrequency)));
	sercom_p->USART.CTRLA.bit.ENABLE = true;
}

#endif /* SERCOM_H_ */