//**********************************************************************************************************************
// FileName : bytearray.h
// FilePath : /utility
// Author   : Christian Marty
// Date		: 24.09.2018
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#ifndef BYTEARRAY_H_
#define BYTEARRAY_H_

#include <stdBool.h>
//#include <main.h>

#ifdef __cplusplus
	extern "C" {
#endif

typedef struct{
	uint8_t *data;
	uint8_t size;
	uint8_t capacity;
} bytearray_t;

static inline void bytearray_init(bytearray_t *bytearray, uint8_t *buffer, uint8_t bufferSize)
{
	bytearray->capacity = bufferSize;
	bytearray->data = buffer;
	bytearray->size = 0;
}

static inline bool bytearray_copy(bytearray_t *destination, const bytearray_t *source)
{
	if(destination->capacity < source->capacity) return false;
	
	destination->size = 0;
	for(uint8_t i = 0; i<source->size; i++)
	{
		destination->data[i] =  source->data[i];
	}
	destination->size =  source->size;
		
	return true;
}

static inline bool bytearray_pushByte(bytearray_t *bytearray, uint8_t data)
{
	if(bytearray->size >= bytearray->capacity) return false;
	
	bytearray->data[bytearray->size] = data;
	bytearray->size++;
	
	return true;
}

static inline bool bytearray_pushArray(bytearray_t *bytearray, uint8_t *data, uint8_t length)
{
	if(bytearray->size + length >= bytearray->capacity) return false;
	
	for(uint8_t i = 0; i<length; i++)
	{
		bytearray->data[i+bytearray->size] =  data[i];
	}
	bytearray->size += length;
	
	return true;
}

static inline bool bytearray_pushBytearray(bytearray_t *bytearray, const bytearray_t *data)
{
	if(bytearray->size + data->size >= bytearray->capacity) return false;
	
	for(uint8_t i = 0; i<data->size; i++)
	{
		bytearray->data[i+bytearray->size] =  data->data[i];
	}
	bytearray->size += data->size;
	
	return true;
}

static inline bool bytearray_pushU16(bytearray_t *bytearray, uint16_t data)
{
	if(bytearray->size + 2 > bytearray->capacity) return false;

    bytearray->data[bytearray->size] = (data>>8) & 0xFF;
    bytearray->data[bytearray->size+1] = data & 0xFF;
	bytearray->size += 2;
	
	return true;
}

static inline bool bytearray_pushU16_LE(bytearray_t *bytearray, uint16_t data)
{
	if(bytearray->size + 2 > bytearray->capacity) return false;

	bytearray->data[bytearray->size] = data & 0xFF;
	bytearray->data[bytearray->size+1] = (data>>8) & 0xFF;
	bytearray->size += 2;

	return true;
}


static inline uint8_t bytearray_getSize(bytearray_t *bytearray)
{
	return bytearray->size;
}

static inline uint8_t bytearray_getCapacity(bytearray_t *bytearray)
{
	return bytearray->capacity;
}

static inline uint8_t bytearray_getSpace(bytearray_t *bytearray)
{
	return bytearray->capacity - bytearray->size;
}

static inline void bytearray_clear(bytearray_t *bytearray)
{
	bytearray->size = 0;
}

static inline void bytearray_flush(bytearray_t *bytearray)
{
	bytearray->size = 0;
}


static inline uint32_t bytearray_decodeU32_LE(bytearray_t *bytearray, uint8_t index)
{
	uint32_t raw = 0;
	raw |= bytearray->data[index];
	raw |= ((uint32_t)bytearray->data[index+1] <<  8);
	raw |= ((uint32_t)bytearray->data[index+2] << 16);
	raw |= ((uint32_t)bytearray->data[index+3] << 24);
	return raw;
}

static inline uint32_t bytearray_decodeU32_BE(bytearray_t *bytearray, uint8_t index)
{
	uint32_t raw = 0;
	raw |= bytearray->data[index+3];
	raw |= ((uint32_t)bytearray->data[index+2] <<  8);
	raw |= ((uint32_t)bytearray->data[index+1] << 16);
	raw |= ((uint32_t)bytearray->data[index] << 24);
	return raw;
}


#ifdef __cplusplus
	}
#endif


#endif /* BYTEARRAY_H_ */