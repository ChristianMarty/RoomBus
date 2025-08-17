//**********************************************************************************************************************
// FileName : interrupt.h
// FilePath : uCodebase/driver/SAMx5x/kernel/
// Author   : Christian Marty
// Date		: 12.01.2019
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#ifndef INTERRUPT_H_
#define INTERRUPT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "common/typedef.h"

typedef void (*interruptHandler_t)(void);

void nvic_initialize(void);

void nvic_assignInterruptHandler(IRQn_Type interruptVector, interruptHandler_t handler);

void nvic_removeInterruptHandler(IRQn_Type interruptVector);

void nvic_setInterruptPriority(IRQn_Type interruptVector, uint8_t priority);

#ifdef __cplusplus
}
#endif

#endif /* INTERRUPT_H_ */