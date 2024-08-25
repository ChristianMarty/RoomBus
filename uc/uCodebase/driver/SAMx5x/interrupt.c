/*
 * interrupt.cpp
 *
 * Created: 12.01.2019 21:37:40
 *  Author: Christian
 */ 

#ifdef __cplusplus
extern "C" {
#endif
	
#include "interrupt.h"

#define NUMBER_OF_INTERRUPTS 137
#define NUMBER_OF_EVENTS 16

interruptHandler_t vectors[NUMBER_OF_INTERRUPTS+NUMBER_OF_EVENTS]  __attribute__((section(".interruptVectorTable")));


void dummyHandler(void)
{
	while(1);
}

void nvic_init(void)
{
	uint32_t *stackTop = (0x00000000);
	vectors[0] = (interruptHandler_t) *stackTop;
	
	for(uint16_t i = 1; i<(NUMBER_OF_INTERRUPTS+NUMBER_OF_EVENTS); i++)
	{
		vectors[i] = dummyHandler;
	}
	
	SCB->VTOR = (uint32_t) &vectors[0]; // Remap Interrupt Vector Table
}

void nvic_assignInterruptHandler(IRQn_Type interruptVector, interruptHandler_t handler)
{
	vectors[NUMBER_OF_EVENTS+interruptVector] = (interruptHandler_t) (uint32_t)handler;
	
	NVIC_EnableIRQ(interruptVector);
}

void nvic_removeInterruptHandler(IRQn_Type interruptVector)
{
	NVIC_DisableIRQ(interruptVector);
	
	vectors[NUMBER_OF_EVENTS+interruptVector] = (interruptHandler_t) (uint32_t)dummyHandler;	
}

void nvic_setInterruptPriority(IRQn_Type interruptVector, uint8_t priority)
{
	NVIC_SetPriority(NUMBER_OF_EVENTS+interruptVector,priority);
}

#ifdef __cplusplus
}
#endif