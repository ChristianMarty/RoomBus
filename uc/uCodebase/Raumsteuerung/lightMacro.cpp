#include "lightMacro.h"

typedef enum {
	LMI_END = 0x00,
	LMI_SLEEP_DIRECT = 0x01,
	LMI_SLEEP = 0x02,
	LMI_EXECUTE = 0x03,
	
	LMI_LOAD_8BIT = 0x10,
	LMI_LOAD_ARRAY = 0x11,
	LMI_MOVE_8BIT = 0x12,
	
}lightMacroInstruction_t;

void lightMacro_run(const kernel_t *kernel, lightMacroParser_t * lightMacroParser)
{
	if(lightMacroParser->run != true) return; 
	
	if(lightMacroParser->sleep == true)
	{
		if( !kernel->tickTimer.delay1ms(&(lightMacroParser->sleepTimer), lightMacroParser->sleepTime) ) return;
		else lightMacroParser->sleep = false;
	}
	
	uint8_t instruction = lightMacroParser->script[lightMacroParser->programCounter];
	lightMacroParser->programCounter++;
	
	switch(instruction)
	{
		case LMI_END:	
		{
			if(!lightMacroParser->loop)lightMacroParser->run = false;
			lightMacroParser->programCounter = 0;
			
		}break;
		
		case LMI_SLEEP_DIRECT:
		{
			uint16_t sleep = lightMacroParser->script[(lightMacroParser->programCounter)++];
			sleep  = (sleep<<8);
			sleep |= lightMacroParser->script[(lightMacroParser->programCounter)++];
			
			lightMacroParser->sleep = true;
			lightMacroParser->sleepTime = sleep;
			kernel->tickTimer.reset(&(lightMacroParser->sleepTimer));
			
		}break;
		
		case LMI_EXECUTE:
		{
			lightMacroParser->execute(lightMacroParser->registers, lightMacroParser->registersSize);
		}break;
				
		case LMI_LOAD_8BIT:	
		{
			uint8_t adr = lightMacroParser->script[(lightMacroParser->programCounter)++];
			uint8_t val = lightMacroParser->script[(lightMacroParser->programCounter)++];
			
			lightMacroParser->registers[adr] = val;
			
		}break;
			
		case LMI_LOAD_ARRAY:
		{
			uint8_t adr = lightMacroParser->script[(lightMacroParser->programCounter)++];
			uint8_t size = lightMacroParser->script[(lightMacroParser->programCounter)++];
			uint8_t *data = &lightMacroParser->script[lightMacroParser->programCounter]; //Do not increment PC here
			lightMacroParser->programCounter += size;
			
			for(uint8_t i = 0; i < size; i++)
			{
				lightMacroParser->registers[adr+i] = data[i];
			}
			
			
		}break;
			
		case LMI_MOVE_8BIT:
		{
			uint8_t *src = (uint8_t*)lightMacroParser->script[(lightMacroParser->programCounter)++];
			uint8_t *dst = (uint8_t*)lightMacroParser->script[(lightMacroParser->programCounter)++];
			
			dst = src ;
			
		}break;
		
		
		default:
		{
			lightMacroParser->error = LMPE_INSTRUCTION;
			lightMacroParser->run = false;
		}
	}
	
}