//**********************************************************************************************************************
// FileName : cobs.hpp
// FilePath : utility/
// Author   : Christian Marty
// Date		: 25.05.2023
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#ifndef COBS_H_
#define COBS_H_

#include "common/typedef.h"

//**********************************************************************************************************************
// Adds COBS Encoding to destination data
//
// Note: Destination needs to be 2 Bytes larger than source
// Return: Length of encoded data
//**********************************************************************************************************************
template<uint8_t COBS_DELIMITER = 0>
static inline uint8_t cobs_encode(uint8_t *destination, const uint8_t *source, uint8_t source_length)
{
    uint8_t nullIndex = 0;
    uint8_t i = 0;

    for(i = 1; i<source_length+1; i++)
    {
        destination[i] = source[i-1];
        if(destination[i] == COBS_DELIMITER)
        {
            destination[nullIndex]= (i-nullIndex);
            nullIndex = i;
        }
    }
    destination[nullIndex]= (i-nullIndex);
    destination[i] = COBS_DELIMITER;
    return i+1;
}


//**********************************************************************************************************************
// Remove COBS decoding form source data
//
// Note: Destination can be 2 bytes shorter than source
// Return: Length of decoded data
//**********************************************************************************************************************
template<uint8_t COBS_DELIMITER = 0>
static inline uint8_t cobs_decode(uint8_t *destination, const uint8_t *source, uint8_t length)
{
    if(length < 2) return  0; // If size to short for valid frame -> empty frame / no data
    if(source[0] == COBS_DELIMITER) return  0; // If first byte is COBS_DELIMITER -> empty frame / no data
    if(source[length-1] != COBS_DELIMITER) return  0; // If last byte is not COBS_DELIMITER -> No valid data

    uint8_t i = 0;
    uint8_t nullIndex = (source[0]-1);

    for(i=0; i<=length; i++)
    {
        destination[i] = source[i+1]; // +1 because first byte is cobs overhead
        if(nullIndex == i)
        {
            if(source[i+1] == COBS_DELIMITER) return i;

            nullIndex += source[i+1];
            destination[i] = COBS_DELIMITER;
        }
        else if(source[i+1] == COBS_DELIMITER) // In case a delimiter is in a position it should not be.
        {
            return 0;
        }
    }
    return i-3;
}


//**********************************************************************************************************************
// Remove COBS decoding from source data in real time
//
// Note: Destination can be 2 bytes shorter than source
// Return: Length of decoded data
//**********************************************************************************************************************
typedef struct {
    uint8_t dataIndex;
    uint8_t nullIndex;
} cobs_decodeStream_t;

static inline void cobs_decodeStreamStart(cobs_decodeStream_t *decodeStream)
{
    decodeStream->dataIndex = 0;
    decodeStream->nullIndex = 0;
}
template<uint8_t COBS_DELIMITER = 0>
static inline uint16_t cobs_decodeStream(cobs_decodeStream_t *decodeStream, uint8_t data, uint8_t *destination)
{
    if(data == COBS_DELIMITER) // End of frame detection
    {
        if(decodeStream->nullIndex == decodeStream->dataIndex){
            uint16_t tmp = decodeStream->dataIndex;
            cobs_decodeStreamStart(decodeStream);
            if(tmp == 0) return 0; // In case of empty frame
            return tmp-1; // Remove cobs byte from size
        }else{
            cobs_decodeStreamStart(decodeStream);
            return 0; // In case of framing error
        }
    }

    if(decodeStream->nullIndex == decodeStream->dataIndex){
        decodeStream->nullIndex += data;
        if(decodeStream->nullIndex != COBS_DELIMITER){
			destination[decodeStream->dataIndex-1] = 0; // Ignore first COBS delimiter
		}
    }else{
        destination[decodeStream->dataIndex-1] = data;
    }

    decodeStream->dataIndex++;
    return 0;
}

#endif /* COBS_H_ */