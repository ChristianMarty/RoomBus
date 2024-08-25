/*
 * interrupt.h
 *
 * Created: 12.01.2019 21:38:44
 *  Author: Christian
 */ 


#ifndef INTERRUPT_H_
#define INTERRUPT_H_

#ifdef __cplusplus
extern "C" {
	#endif
	
#include "sam.h"

typedef void (*interruptHandler_t)(void);

void nvic_init(void);

void nvic_assignInterruptHandler(IRQn_Type interruptVector, interruptHandler_t handler);

void nvic_removeInterruptHandler(IRQn_Type interruptVector);

void nvic_setInterruptPriority(IRQn_Type interruptVector, uint8_t priority);

#ifdef __cplusplus
}
#endif

#endif /* INTERRUPT_H_ */