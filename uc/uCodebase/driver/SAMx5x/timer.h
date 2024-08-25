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

#include "genericClockController.h"

typedef uint32_t timer_time_t;

typedef enum {
	TIMER_DIV1,
	TIMER_DIV2,
	TIMER_DIV4,
	TIMER_DIV8,
	TIMER_DIV16,
	TIMER_DIV64,
	TIMER_DIV256,
	TIMER_DIV1024
} timer_prescaler_t;

typedef enum{
	TIMER_COUNTER_16_BIT,
	TIMER_COUNTER_8_BIT,
	TIMER_COUNTER_32_BIT
}timer_counter_mode_t;

void timer_init(Tc *timer, gclk_generator_t clkGenerator,  timer_prescaler_t prescaler, bool enable);
void timer_enable(Tc *timer);
void timer_disable(Tc *timer);
void timer_setPeriod(Tc *timer, uint32_t period);

void timer_capture0Interrupt(Tc *timer, bool enable);
void timer_overflowInterrupt(Tc *timer, bool enable);


#ifdef __cplusplus
}
#endif

#endif /* SYSTICKTIMER_H_ */