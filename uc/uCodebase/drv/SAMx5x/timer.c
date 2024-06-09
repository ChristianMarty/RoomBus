/*
 * sysTickTimer.c
 *
 * Created: 12.01.2019 21:37:40
 *  Author: Christian
 */ 

#ifdef __cplusplus
extern "C" {
#endif

#include "timer.h"


void timer_init(Tc *timer, gclk_generator_t clkGenerator,  timer_prescaler_t prescaler, bool enable)
{	
	timer->COUNT16.CTRLA.bit.PRESCALER = prescaler;
	
	if(timer == TC0)
	{
		gclk_peripheralEnable(GCLK_TC0_TC1, clkGenerator);
		MCLK->APBAMASK.bit.TC0_ = true;
	}
	else if(timer == TC1)
	{
		gclk_peripheralEnable(GCLK_TC0_TC1,clkGenerator);
		MCLK->APBAMASK.bit.TC1_= true;
	}
	else if(timer == TC2)
	{
		gclk_peripheralEnable(GCLK_TC2_TC3,clkGenerator);
		MCLK->APBBMASK.bit.TC2_= true;
	}
	else if(timer == TC3)
	{
		gclk_peripheralEnable(GCLK_TC2_TC3,clkGenerator);
		MCLK->APBBMASK.bit.TC3_= true;
	}
	else if(timer == TC4)
	{
		gclk_peripheralEnable(GCLK_TC4_TC5,clkGenerator);
		MCLK->APBCMASK.bit.TC4_= true;
	}
	else if(timer == TC5)
	{
		gclk_peripheralEnable(GCLK_TC4_TC5,clkGenerator);
		MCLK->APBCMASK.bit.TC5_= true;
	}
	
	timer->COUNT16.CTRLA.bit.ENABLE = enable;
}

void timer_enable(Tc *timer)
{
	timer->COUNT16.CTRLA.bit.ENABLE = true;
}

void timer_disable(Tc *timer)
{
	timer->COUNT16.CTRLA.bit.ENABLE = false;
}

void timer_setPeriod(Tc *timer, uint32_t period)
{
	timer->COUNT16.CC->reg = period;
}

void timer_overflowInterrupt(Tc *timer, bool enable)
{
	if(enable) timer->COUNT16.INTENSET.bit.OVF = true;
	else timer->COUNT16.INTENCLR.bit.OVF = true;
}

void timer_capture0Interrupt(Tc *timer, bool enable)
{
	if(enable) timer->COUNT16.INTENSET.bit.MC0 = true;
	else timer->COUNT16.INTENCLR.bit.MC0 = true;
}

#ifdef __cplusplus
}
#endif