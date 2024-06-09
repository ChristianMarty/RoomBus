/*
 * sysTickTimer.c
 *
 * Created: 12.01.2019 21:37:40
 *  Author: Christian
 */ 

#ifdef __cplusplus
extern "C" {
#endif

#include "sysTickTimer.h"

volatile uint32_t sysTickTime = 0;

void sysTick_init(uint32_t CPUclockFreq)
{
	// Init Sys Tick
	SysTick->LOAD = (CPUclockFreq/1000); // Set Time to 1 ms
	SysTick->CTRL = 0x00000003; // enable
}

void sysTick_interruptHandler(void)
{
	sysTickTime++;
}

void sysTick_resetDelayCounter(uint32_t *counter)
{
	*counter = sysTickTime;
}

bool sysTick_delay1ms(uint32_t *counter, uint32_t delay)
{
	if((sysTickTime-*counter) < delay)
	{
		return false;
	}
	else
	{
		*counter = sysTickTime;
		return true;
	}
}

uint16_t sysTick_getTick_us(void)
{
	return (SysTick->LOAD - SysTick->VAL)/120;
}

uint32_t sysTick_getTickTime(void)
{
	return sysTickTime;
}

void sysTick_sleep1ms(uint32_t delay)
{
	uint32_t start = sysTickTime;
	while((sysTickTime-start) < delay);
}

#ifdef __cplusplus
}
#endif