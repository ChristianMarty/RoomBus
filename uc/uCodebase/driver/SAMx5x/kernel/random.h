//**********************************************************************************************************************
// FileName : random.h
// FilePath : uCodebase/driver/SAMx5x/kernel/
// Author   : Christian Marty
// Date		: 11.11.2018
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#ifndef RANDON_H_
#define RANDON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "common/typedef.h"
	
static inline void rand_initialize(void)
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

#endif /* RANDON_H_ */