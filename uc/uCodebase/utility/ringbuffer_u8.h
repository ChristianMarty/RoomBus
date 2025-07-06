//**********************************************************************************************************************
// FileName : ringbuffer_u8.h
// FilePath : utility/
// Author   : Christian Marty
// Date		: 04.01.2025
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#ifndef RINGBUFFER_U8_H_
#define RINGBUFFER_U8_H_
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint8_t *_data;
	uint8_t _capacity;
	uint8_t _count;
	uint8_t _readIndex;
	uint8_t _writeIndex;
} ringbufferU8_t;

static inline void ringbufferU8_init(ringbufferU8_t *ringbuffer, uint8_t *data, uint8_t size)
{
	ringbuffer->_readIndex = 0;
	ringbuffer->_writeIndex = 0;
	
	ringbuffer->_data = data;
	ringbuffer->_count = 0;
	ringbuffer->_capacity = size;
}

static inline bool ringbufferU8_empty(ringbufferU8_t *ringbuffer)
{
	return (ringbuffer->_count == 0);
}

static inline uint8_t ringbufferU8_count(ringbufferU8_t *ringbuffer)
{
	return ringbuffer->_count;
}

static inline uint8_t ringbufferU8_space(ringbufferU8_t *ringbuffer)
{
	return (ringbuffer->_capacity - ringbuffer->_count);
}

static inline bool ringbufferU8_full(ringbufferU8_t *ringbuffer)
{
	return (ringbuffer->_count == ringbuffer->_capacity);
}

static inline void ringbufferU8_write(ringbufferU8_t *ringbuffer, uint8_t data)
{
	if(ringbuffer->_count == ringbuffer->_capacity) return;
	
	ringbuffer->_count++;
	
	ringbuffer->_data[ringbuffer->_writeIndex] = data;
	ringbuffer->_writeIndex++;
	
	if(ringbuffer->_writeIndex >= ringbuffer->_capacity){
		ringbuffer->_writeIndex = 0;
	}
}

static inline uint8_t ringbufferU8_read(ringbufferU8_t *ringbuffer)
{
	if(ringbuffer->_count == 0) return 0;
	
	ringbuffer->_count--;
	
	uint8_t readIndex = ringbuffer->_readIndex;
	
	ringbuffer->_readIndex++;
	if(ringbuffer->_readIndex >= ringbuffer->_capacity){
		ringbuffer->_readIndex = 0;
	}
	
	return ringbuffer->_data[readIndex];
}

static inline uint8_t ringbufferU8_get(ringbufferU8_t *ringbuffer, uint8_t offset)
{
	if(ringbuffer->_count == 0) return 0;

	uint8_t readIndex = ringbuffer->_readIndex;
	readIndex += offset;
	
	if(readIndex >= ringbuffer->_capacity){
		readIndex -= ringbuffer->_capacity;
	} 
	
	return ringbuffer->_data[readIndex];
}

static inline void ringbufferU8_remove(ringbufferU8_t *ringbuffer, uint8_t count)
{
	if(ringbuffer->_count == 0) return;
	
	ringbuffer->_readIndex += count;
}

#ifdef __cplusplus
}
#endif

#endif /* RINGBUFFER_U8_H_ */