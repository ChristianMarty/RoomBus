/*
 * buffer.h
 *
 * Created: 16.09.2018 22:06:38
 *  Author: Christian
 */ 


#ifndef BUFFER_H_
#define BUFFER_H_

#include "sam.h"

enum error_t{error, noError}; 

template <typename T, uint16_t BUFFER_SIZE>
class buffer_c
{
	
	public:
		buffer_c(): readIndex(0),writeIndex(BUFFER_SIZE-1)
		{}
		
		void write(T data)
		{
			writeIndex ++;
			if(writeIndex >= BUFFER_SIZE) writeIndex = 0;
			buffer[writeIndex] = data;
		}
		
		void writeBlock(T *data, uint16_t length)
		{
			for(uint16_t i = 0; i < length; i++)
			{
				write(data[i]);
			}
		}
		
		T read(void)
		{
			readIndex++;
			if(readIndex >= BUFFER_SIZE) readIndex = 0;
			return buffer[readIndex];
		}
		
		uint16_t freeSize(void)
		{	
			if(readIndex < writeIndex) return (writeIndex - readIndex);	
			else return (BUFFER_SIZE - (writeIndex - readIndex));
		}
		
		bool empty(void)
		{
			return (readIndex == writeIndex);
		}
		
		void flush(void)
		{
			readIndex = 0;
			writeIndex = BUFFER_SIZE-1;
		}
	
	private:
		T  buffer[BUFFER_SIZE];
		uint16_t readIndex;
		uint16_t writeIndex;
};

template <typename T, uint16_t ARRAY_SIZE>
class array_c
{
	public:
		array_c(): size(ARRAY_SIZE)
		{}
		
		void flush(void)
		{
			for (uint16_t i= 0; i<size; i++)
			{
				data[i] = 0;
			}
		}
		
		T  data[ARRAY_SIZE];
		const uint16_t size;
};

#endif /* BUFFER_H_ */