/*
 * GenericClockController.c
 *
 * Created: 03.02.2019 12:15:41
 *  Author: Christian
 */ 

#include "genericClockController.h"
#include "sam.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void gclk_reset(void)
{
	GCLK->CTRLA.reg = 0x01;
}

gclk_generator_t gclk_getFreeGenerator(void)
{
	for(uint8_t i = 1; i < GCLK_GENERATOR_SIZE; i++)
	{
		if(!(GCLK->GENCTRL[i].reg & 0x100)) return i;
	}
	return 0;
}

gclk_error_t gclk_generatorEnable(gclk_generator_t generator, gclk_source_t clkSource, uint16_t divisonFactor)
{
	if(generator >= GCLK_GENERATOR_SIZE) return GCLK_ERROR_generatorOutOfRange;
	if(clkSource >= GCLK_SRC_SIZE) return GCLK_ERROR_clkSourceOutOfRange;
	if(GCLK->GENCTRL[generator].reg & 0x100) return GCLK_ERROR_generatorAlreadyUsed;
	
	uint32_t temp = 0;
	temp |= clkSource;
	temp |=  ((uint32_t)divisonFactor<<16);
	temp |= 0x100;
	
	GCLK->GENCTRL[generator].reg = temp;
	
	return GCLK_ERROR_noError;
}

gclk_error_t glck_generatorDisable(gclk_generator_t generator)
{
	if(generator >= GCLK_GENERATOR_SIZE) return GCLK_ERROR_generatorOutOfRange;
	GCLK->GENCTRL[generator].reg = 0;
	
	return GCLK_ERROR_noError;
}

gclk_error_t gclk_peripheralEnable(gclk_peripheral_t peripheralChannel,gclk_generator_t clkGenerator)
{
	if(peripheralChannel >= GCLK_PERIPHERAL_SIZE) return GCLK_ERROR_peripheralChannelOutOfRange;
	if(clkGenerator >= GCLK_GENERATOR_SIZE) return GCLK_ERROR_generatorOutOfRange;
	if(GCLK->PCHCTRL[peripheralChannel].reg & 0x40) return GCLK_ERROR_peripheralChannelAlreadyUsed;
	
	GCLK->PCHCTRL[peripheralChannel].bit.GEN = clkGenerator;
	GCLK->PCHCTRL[peripheralChannel].bit.CHEN = true;
	return GCLK_ERROR_noError;
}

gclk_error_t gclk_peripheralDisable(gclk_peripheral_t peripheralChannel)
{
	if(GCLK->PCHCTRL[peripheralChannel].reg & 0x40) return GCLK_ERROR_peripheralChannelAlreadyUsed;
	GCLK->PCHCTRL[peripheralChannel].reg = 0;
	
	return GCLK_ERROR_noError;	
}
	
#ifdef __cplusplus
}
#endif