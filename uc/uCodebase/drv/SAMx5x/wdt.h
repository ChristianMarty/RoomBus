/*
 * wdt.h
 *
 * Created: 11.11.2018 18:31:02
 *  Author: Christian
 */ 


#ifndef WDT_H_
#define WDT_H_

#include "sam.h"
#include "interrupt.h"

#define WDT_CLEAR_KEY 0xA5


#ifdef __cplusplus
extern "C" {
	#endif
	
	static inline void wdt_off(void)
	{
		WDT->CTRLA.bit.ENABLE = false;
	}
	
	static inline void wdt_clear(void)
	{
		if(!WDT->SYNCBUSY.bit.CLEAR)WDT->CLEAR.reg = WDT_CLEAR_KEY;		
	}
		
	static inline void wdt_init(interruptHandler_t earlyWarningHandler)
	{
		WDT->CONFIG.bit.PER = 0x8; // 2s timeout until reset
		WDT->EWCTRL.bit.EWOFFSET = 0x7; // 1s timeout until Early Warning 
		
		nvic_assignInterruptHandler(WDT_IRQn, earlyWarningHandler);
		
		WDT->INTENSET.bit.EW = true;
		
		WDT->CTRLA.bit.WEN = false;
		WDT->CTRLA.bit.ENABLE = true;
		
		wdt_clear();
	}
 
 
#ifdef __cplusplus
}
#endif


#endif /* WDT_H_ */