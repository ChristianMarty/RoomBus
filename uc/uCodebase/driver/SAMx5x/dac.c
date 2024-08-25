/*
 * dac.c
 *
 * Created: 02.11.2022 23:38:44
 *  Author: Christian
 */ 


#ifdef __cplusplus
extern "C" {
#endif

#include "dac.h"
#include "drv/SAMx5x/pin.h"
#include "kernel/busController_IO.h"

void dac_init(dac_t *dac, gclk_generator_t clkGenerator, bool channel_0_enable, bool channel_1_enable)
{
	gclk_peripheralEnable(GCLK_DAC,clkGenerator);
	MCLK->APBDMASK.bit.DAC_ = true;
	
	DAC->CTRLA.bit.SWRST = true;
	while(DAC->CTRLA.bit.SWRST == true);
	
	DAC->CTRLB.bit.REFSEL = 0x03; // Use internal bandgap reference (typ. 1V)
	
	if(channel_0_enable)
	{
		pin_enablePeripheralMux(IO_A00, PIN_FUNCTION_B);
		DAC->DACCTRL[0].bit.ENABLE = true;
	}
	
	if(channel_1_enable)
	{
		pin_enablePeripheralMux(IO_A03, PIN_FUNCTION_B);
		DAC->DACCTRL[1].bit.ENABLE = true;
	}
	
	DAC->CTRLA.bit.ENABLE = true;
}

void dac_channel_set(dac_t *dac, dac_channel_t channel, uint16_t value)
{
	if(channel == DAC_0)
	{
		DAC->DATA[0].reg = value;
	}
	else if(channel == DAC_1)
	{
		DAC->DATA[1].reg = value;
	}
}

 


#ifdef __cplusplus
}
#endif