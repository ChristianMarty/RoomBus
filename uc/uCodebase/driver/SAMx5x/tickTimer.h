//**********************************************************************************************************************
// FileName : tickTimer.h
// FilePath : uCodebase/driver/SAMx5x/
// Author   : Christian Marty
// Date		: 12.01.2019
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#ifndef TICK_TIMER_H_
#define TICK_TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "common/typedef.h"

typedef uint32_t tickTimer_t;

void tickTimer_init(uint32_t CPUclockFreq);

void tickTimer_interruptHandler(void);

void tickTimer_reset(tickTimer_t *counter);

bool tickTimer_delay1ms(tickTimer_t *counter, uint32_t delay);

tickTimer_t tickTimer_getTickTime(void);

uint16_t tickTimer_getTick_us(void);

void tickTimer_sleep1ms(uint32_t delay);

#ifdef __cplusplus
}
#endif

#endif /* TICK_TIMER_H_ */