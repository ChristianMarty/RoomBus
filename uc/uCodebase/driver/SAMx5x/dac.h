/*
 * dac.h
 *
 * Created: 02.11.2022 23:38:44
 *  Author: Christian
 */ 


#ifndef DAC_H_
#define DAC_H_

#ifdef __cplusplus
extern "C" {
	#endif
	
#include "sam.h"
#include <stdbool.h>

#include "genericClockController.h"

typedef enum{
	DAC_0,
	DAC_1
}dac_channel_t;

typedef struct{

}dac_t;

void dac_init(dac_t *dac, gclk_generator_t clkGenerator, bool channel_0_enable, bool channel_1_enable);

void dac_channel_set(dac_t *dac, dac_channel_t channel, uint16_t value);

#ifdef __cplusplus
}
#endif

#endif /* DAC_H_ */