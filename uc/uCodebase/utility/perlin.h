//**********************************************************************************************************************
// FileName : perlin.h
// FilePath : /utility
// Author   : Christian Marty
// Date		: 01.08.2024
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#ifndef PERLIN_H_
#define PERLIN_H_

#include <stdint.h>

#ifdef __cplusplus
	extern "C" {
#endif

void perlin(uint8_t *seed, uint8_t *output, uint8_t size);

#ifdef __cplusplus
	}
#endif

#endif /* PERLIN_H_ */