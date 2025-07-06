//**********************************************************************************************************************
// FileName : memory.h
// FilePath : utility/
// Author   : Christian Marty
// Date		: 19.01.2025
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#ifndef MEMORY_H_
#define MEMORY_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline void memory_copy(uint8_t *destination, uint8_t *source, uint8_t length)
{
    for(uint8_t i=0; i < length; i++){
        destination[i] = source[i];
    }
}

static inline uint8_t memory_removeFront(uint8_t removeLength, uint8_t *data, uint8_t dataLength)
{
    uint8_t newLength = 0;
    if(removeLength < dataLength){
        newLength = dataLength-removeLength;
    }

    for(uint8_t i=0; i < dataLength; i++){
        if(i<newLength)data[i] = data[i+removeLength];
        else data[i] = 0;
    }

    return newLength;
}

#ifdef __cplusplus
}
#endif

#endif /* MEMORY_H_ */