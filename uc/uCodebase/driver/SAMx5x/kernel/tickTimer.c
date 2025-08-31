//**********************************************************************************************************************
// FileName : tickTimer.c
// FilePath : uCodebase/driver/SAMx5x/kernel/
// Author   : Christian Marty
// Date		: 12.01.2019
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#ifdef __cplusplus
extern "C" {
#endif

#include "tickTimer.h"
#include "interrupt.h"

volatile uint32_t tickCounter;

void tickTimer_interruptHandler(void);

void tickTimer_initialize(void)
{
#ifndef TEST_RUN
	SysTick->LOAD = (120000000/1000); // Set Time to 1 ms
	SysTick->CTRL = 0x00000003; // enable
	
	nvic_assignInterruptHandler(SysTick_IRQn, tickTimer_interruptHandler);
#endif
	tickCounter = 0;
}

void tickTimer_interruptHandler(void)
{
	tickCounter++;
}

void tickTimer_reset(tickTimer_t *counter)
{
	*counter = tickCounter;
}

bool tickTimer_delay1ms(tickTimer_t *counter, uint32_t delay)
{
	if((tickCounter-*counter) < delay){
		return false;
	}else{
		*counter = tickCounter;
		return true;
	}
}

uint16_t tickTimer_getTick_us(void)
{
#ifndef TEST_RUN
	return (SysTick->LOAD - SysTick->VAL)/120;
#else
	return 0xFFFF;
#endif
}

tickTimer_t tickTimer_getTickTime(void)
{
	return tickCounter;
}

#ifdef __cplusplus
}
#endif