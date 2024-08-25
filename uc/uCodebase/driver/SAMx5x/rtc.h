/*
 * rand.h
 *
 * Created: 11.11.2018 18:31:02
 *  Author: Christian
 */ 


#ifndef RAND_H_
#define RAND_H_

#ifdef __cplusplus
extern "C" {
	#endif
	
	
#include "sam.h"
	
static inline void rtc_init(void)
{
	OSC32KCTRL->XOSC32K.bit.EN1K = true; // Enable 1.024kHz output
	OSC32KCTRL->XOSC32K.bit.EN32K = true; // Enable 32kHz output
	OSC32KCTRL->XOSC32K.bit.ENABLE = true; // Enable Oscillator
	
	RTC->MODE2.CTRLA.reg = 0x8B08; // divide clock by 1024, select Clock/Calendar mode
	
	RTC->MODE2.CTRLA.bit.ENABLE = true;
	
}

static inline void rtc_sleep1s()
{
	uint8_t now = (uint8_t)(RTC->MODE2.CLOCK.reg & 0x3F);
	
	while(now == (uint8_t)(RTC->MODE2.CLOCK.reg & 0x3F));
}	
	#ifdef __cplusplus
}
#endif


#endif /* RAND_H_ */