
#ifndef LIGHT_MACRO_H_
#define LIGHT_MACRO_H_

#include "sam.h"
#include "kernel/kernel.h"


typedef struct {
	uint8_t *script;
	
	
}lightMacro_t;

typedef enum {
	LMPE_NONE,
	LMPE_INSTRUCTION
}lightMacroParserError_t;

typedef void (*lightMacroParser_execute_t)(uint8_t *registers, uint16_t registersSize);

typedef struct {
	uint8_t *script;
	uint16_t programCounter;
	
	uint8_t *registers;
	uint16_t registersSize;
	
	uint32_t sleepTime;
	uint32_t sleepTimer;
	bool sleep;
	
	bool loop;
	bool run;
	
	lightMacroParser_execute_t execute;
	lightMacroParserError_t error;
	
}lightMacroParser_t;



void lightMacro_run(const kernel_t *kernel, lightMacroParser_t * lightMacroParser);


#endif /* LIGHT_MACRO_H_ */