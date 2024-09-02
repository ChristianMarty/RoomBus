//**********************************************************************************************************************
// FileName : perlin.c
// FilePath : /utility
// Author   : Christian Marty
// Date		: 01.08.2024
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#include "string.h"

#ifdef __cplusplus
	extern "C" {
#endif

void perlin(uint8_t *seed, uint8_t *output, uint8_t size)
{
    uint8_t interval = 4;

    for (uint8_t x = 0; x < size; x++)
    {
        uint16_t noiseValue = 0;
        for (int i = 0; i < interval; i++)
        {
            uint8_t pitch = size >> i;
            uint8_t sample1 = (x / pitch) * pitch; // looks like NOP but isn't
            uint8_t sample2 = (sample1 + pitch) % size;

            uint8_t filler = ((x*0xFF - sample1*0xFF) / pitch);
            uint16_t sample = ((0xFF - filler) * seed[sample1] + filler * seed[sample2]);

            noiseValue += (sample / 0xFF);
        }

        output[x] = noiseValue / interval;
    }
}



#ifdef __cplusplus
	}
#endif
