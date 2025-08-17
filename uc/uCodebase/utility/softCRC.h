//**********************************************************************************************************************
// FileName : softCRC.h
// FilePath : /utility
// Author   : Christian Marty
// Date		: 24.09.2018
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#ifndef SOFT_CRC_H_
#define SOFT_CRC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "bytearray.h"

//**********************************************************************************************************************
// Name       : CCITT
// Polynomial : 0x1021
// Note       :
//**********************************************************************************************************************
static inline uint16_t crc16_addByte(uint16_t crc, uint8_t data)
{
	uint8_t x = crc >> 8 ^ data;
	x ^= x>>4;
	return (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x <<5)) ^ ((uint16_t)x);
}

static inline uint16_t crc16(uint8_t* data, uint8_t length) 
{
	uint16_t crc = 0xFFFF;

	while (length--)
	{
		crc = crc16_addByte(crc, *data++);
	}
	return crc;
}

static inline uint16_t crc16_bytearray(const bytearray_t* data) 
{
	uint16_t crc = 0xFFFF;

	for(uint8_t i = 0; i<data->size; i++)
	{
		crc = crc16_addByte(crc,  data->data[i]);
	}

	return crc;
}

//**********************************************************************************************************************
// Name        : Modbus
// Polynomial  : 0xA001
// Note        : For Modbus RTU
//**********************************************************************************************************************
static inline uint16_t crcModbus_addByte(uint16_t crc, uint8_t data)
{
	crc = crc ^ data;
	for (uint8_t j = 0; j < 8; j++)
	{
		if (crc & 0x0001)
		crc = (crc >> 1) ^ 0xA001;
		else
		crc >>= 1;
	}
	return crc;

	//uint8_t x = crc >> 8 ^ data;
	//x ^= x>>4;
	//return (crc << 8) ^ ((uint16_t)(x << 15)) ^ ((uint16_t)(x <<13)) ^ ((uint16_t)x);
}

static inline uint16_t crcModbus(uint8_t* data, uint8_t length) 
{
	uint16_t crc = 0xFFFF;

	while (length--)
	{
		crc = crcModbus_addByte(crc, *data++);
	}
	return crc;
}

static inline uint16_t crcModbus_bytearray(const bytearray_t* data) 
{
	uint16_t crc = 0xFFFF;

	for(uint8_t i = 0; i< data->size; i++)
	{
		crc = crcModbus_addByte(crc,  data->data[i]);
	}

	return crc;
}

//**********************************************************************************************************************
// Name       : oneWire
// Polynomial : 0x8C
// Note       : For oneWire
//**********************************************************************************************************************
static inline uint8_t crc8_oneWire(uint8_t* data, uint8_t length)
{
	uint8_t crc = 0x00;
	
	for(uint8_t i = 0; i<length; i++)
	{
		crc = crc ^ data[i];
		for (uint8_t j = 0; j < 8; j++)
		{
			if (crc & 0x01)
				crc = (crc >> 1) ^ 0x8C;
			else
				crc >>= 1;
		}
	}
	
	return crc;
}

#ifdef __cplusplus
}
#endif

#endif /* SOFT_CRC_H_ */