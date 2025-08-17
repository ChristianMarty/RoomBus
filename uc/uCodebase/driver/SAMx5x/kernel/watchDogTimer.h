//**********************************************************************************************************************
// FileName : watchDogTimer.h
// FilePath : uCodebase/driver/SAMx5x/kernel/
// Author   : Christian Marty
// Date		: 11.11.2018
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#ifndef WATCH_DOG_TIMER_H_
#define WATCH_DOG_TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif
	
#include "common/typedef.h"
#include "interrupt.h"

#define WDT_CLEAR_KEY 0xA5

static inline void watchDogTimer_off(void)
{
	WDT->CTRLA.bit.ENABLE = false;
}
	
static inline void watchDogTimer_clear(void)
{
	if(!WDT->SYNCBUSY.bit.CLEAR){
		WDT->CLEAR.reg = WDT_CLEAR_KEY;
	}
}
		
static inline void watchDogTimer_initialize(interruptHandler_t earlyWarningHandler)
{
	WDT->CONFIG.bit.PER = 0x8; // 2s timeout until reset
	WDT->EWCTRL.bit.EWOFFSET = 0x7; // 1s timeout until Early Warning 
		
	nvic_assignInterruptHandler(WDT_IRQn, earlyWarningHandler);
		
	WDT->INTENSET.bit.EW = true;
		
	WDT->CTRLA.bit.WEN = false;
	WDT->CTRLA.bit.ENABLE = true;
		
	watchDogTimer_clear();
}

#ifdef __cplusplus
}
#endif

#endif /* WATCH_DOG_TIMER_H_ */