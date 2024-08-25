/*
 * rand.h
 *
 * Created: 11.11.2018 18:31:02
 *  Author: Christian
 */ 


#ifndef RAND_H_
#define RAND_H_

#ifdef __cplusplus
extern "C" {
	#endif
	
	
#include "sam.h"
	
static inline void rand_init(void)
{
	MCLK->APBCMASK.bit.TRNG_ = true; // Enable Clock
	TRNG->CTRLA.bit.ENABLE = true; // Enable TRNG
}

static inline bool rand_isReady(void)
{
	if(TRNG->INTFLAG.bit.DATARDY) return true;
	else return false;
}

static inline uint32_t rand_getRand(void)
{
	while(!rand_isReady());
	return TRNG->DATA.reg;	
}

static inline uint32_t rand_getRandRange(uint32_t min, uint32_t max)
{
	uint32_t temp = rand_getRand();
	uint32_t range = max-min;
	if(range != 0xFFFFFFFF) temp = (temp % (range+1));
	temp += min;
	return temp;
}	

#ifdef __cplusplus
}
#endif


#endif /* RAND_H_ */