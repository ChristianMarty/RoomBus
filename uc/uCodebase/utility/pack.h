/*
 * pack.h
 *
 * Created: 24.09.2018 21:42:17
 *  Author: Christian
 */ 


#ifndef PACK_H_
#define PACK_H_

#ifdef __cplusplus
	extern "C" {
#endif


static inline uint32_t unpack_uint32(const uint8_t *data)
{
	return (((uint32_t)data[0]<<24)|((uint32_t)data[1]<<16)|((uint32_t)data[2]<<8)|((uint32_t)data[3]));
}

static inline uint16_t unpack_uint16(const uint8_t *data)
{
	return (((uint16_t)data[0]<<8)|((uint16_t)data[1]));
}

static inline uint16_t unpack_uint16_le(const uint8_t *data)
{
	return (((uint16_t)data[1]<<8)|((uint16_t)data[0]));
}

#ifdef __cplusplus
	}
#endif


#endif /* PACK_H_ */