//**********************************************************************************************************************
// FileName : string.h
// FilePath : utility/
// Author   : Christian Marty
// Date		: 02.08.2025
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#ifndef STRING2_H_
#define STRING2_H_

#ifdef __cplusplus
extern "C" {
#endif
	
#include <stdint.h>
#include <stdbool.h>

#include "bytearray.h"

uint8_t string_getLength(const char *str);

char *string_fromInt32(int32_t value, uint8_t comma, char *outStr);

char *string_fromFloat(float value, uint8_t decimalPlaces, char *outStr);

char *string_bytearrayToHex(bytearray_t *value, char *outStr);

char *string_uInt8ToHex(uint8_t value, char *outStr);

char *string_uInt16ToHex(uint32_t value, char *outStr);

char *string_uInt32ToHex(uint32_t value, char *outStr);

char *string_uInt64ToHex(uint64_t value, char *outStr);

char *string_append(char *str1, const char *str2);


bool string_isEqual(const char *str1, const char *str2, uint8_t maxLength);

int string_parseInteger(const char *string, uint8_t maxLength);

float string_parseNumber(const char *string, uint8_t maxLength);


void string_removeFront(char *string, uint8_t removeLength, uint8_t maxLength);

/* Possible line endings
 * windows: \r\n
 * old mac: \r
 * unix: \n
 * for completeness: \n\r
 */
uint16_t string_readLine(const char *string, uint16_t maxLength);

#ifdef __cplusplus
	}
#endif

#endif /* STRING2_H_ */