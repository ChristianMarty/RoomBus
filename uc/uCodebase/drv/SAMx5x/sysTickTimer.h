/*
 * sysTickTimer.c
 *
 * Created: 12.01.2019 21:38:44
 *  Author: Christian
 */ 


#ifndef SYSTICKTIMER_H_
#define SYSTICKTIMER_H_

#ifdef __cplusplus
extern "C" {
	#endif
	
#include "sam.h"
#include <stdbool.h>

void sysTick_init(uint32_t CPUclockFreq);

void sysTick_interruptHandler(void);

void sysTick_resetDelayCounter(uint32_t *counter);

bool sysTick_delay1ms(uint32_t *counter, uint32_t delay);

uint32_t sysTick_getTickTime(void);

uint16_t sysTick_getTick_us(void);

void sysTick_sleep1ms(uint32_t delay);

#ifdef __cplusplus
}
#endif

#endif /* SYSTICKTIMER_H_ */