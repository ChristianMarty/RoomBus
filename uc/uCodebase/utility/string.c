//**********************************************************************************************************************
// FileName : string.c
// FilePath : utility/
// Author   : Christian Marty
// Date		: 24.09.2018
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#include "string.h"

#ifdef __cplusplus
	extern "C" {
#endif

uint8_t string_getLength(const char *str)
{
	uint8_t i = 0;
	while((str[i] != 0) && ((uint8_t)str[i] != 0xFF))
	{
		i++;
		if(i>250) return 0;
	}
	return i;
}

static char *_fromInt_nextDigit(char *dest, int32_t value, uint8_t comma) 
{
	if (value <= -10) 
	{
		comma--;
		dest = _fromInt_nextDigit(dest, value/10,comma);
		if(comma== 0) *dest++ =  '.';
	}
	
	*dest++ = '0' - value%10;
	return dest;
}

char *string_fromInt32(int32_t value, uint8_t comma, char *outStr)
{
	char *temp = outStr;

	if(value < 0) *temp++ = '-';
	else value = value *-1;
	
	uint8_t i = 0;
	int32_t x = value;
	
	while(x) {
		x=(x/10);
		i++; 
	}
	
	if(i<comma)
	{
		*temp++ =  '0';
		*temp++ =  '.';
		
		while(i<comma){
			i++;
			*temp++ = '0';
		}
	}

	temp = _fromInt_nextDigit(temp,value,comma);
	
	*temp++  = 0x00;		
	return &outStr[0];
}

char *string_fromFloat(float value, uint8_t decimalPlaces, char *outStr)
{
	string_fromInt32((uint32_t)value, 0, &outStr[0]);
	
	uint32_t multi = 1;
	for(uint8_t i = 0; i<decimalPlaces; i++)
	{
		multi*=10;
	}
	
	uint32_t decimals = (value- (float)((uint32_t)value))*multi;
	
	uint8_t pos = string_getLength(outStr);
	outStr[pos] = '.';
	
	string_fromInt32(decimals, 0, &outStr[pos+1]);
		
	return &outStr[0];
}

char *string_uInt32ToHex(uint32_t value, char *outStr)
{
	string_uInt8ToHex((value >>24)&0xFF, &outStr[0]);
	string_uInt8ToHex((value >>16)&0xFF, &outStr[2]);
	string_uInt8ToHex((value >>8)&0xFF, &outStr[4]);
	string_uInt8ToHex((value)&0xFF, &outStr[6]);
	
	return &outStr[0];
}

char *string_uInt64ToHex(uint64_t value, char *outStr)
{
	string_uInt8ToHex((value >>56)&0xFF, &outStr[0]);
	string_uInt8ToHex((value >>48)&0xFF, &outStr[2]);
	string_uInt8ToHex((value >>40)&0xFF, &outStr[4]);
	string_uInt8ToHex((value >>32)&0xFF, &outStr[6]);
	
	string_uInt8ToHex((value >>24)&0xFF, &outStr[8]);
	string_uInt8ToHex((value >>16)&0xFF, &outStr[10]);
	string_uInt8ToHex((value >>8)&0xFF, &outStr[12]);
	string_uInt8ToHex((value)&0xFF, &outStr[14]);
    
	return &outStr[0];
}

char *string_uInt8ToHex(uint8_t value, char *outStr)
{
	uint8_t digit = (value>>4)&0x0F;
	if(digit<10) outStr[0] = 0x30 + digit;
	else outStr[0] = 0x41 + digit -10;
	
	digit = (value&0x0F);
	if(digit<10) outStr[1] = 0x30 + digit;
	else outStr[1] = 0x41 + digit - 10;
	
	outStr[2] = 0;
	
	return &outStr[0];	
}

char *string_bytearrayToHex(bytearray_t *value, char *outStr)
{
	uint16_t j = 0;
	for(uint8_t i = 0; i< value->size; i++)
	{
		string_uInt8ToHex(value->data[i], &outStr[j]);
		outStr[j+2] = ' ';
		j +=3;
	}
	outStr[j] = 0x00;
	
	return &outStr[0];
}

char *string_append(char *str1, const char *str2)
{
	char *out = str1;
	
	while(*str1++ != 0);
	str1--;
	
	while(*str2 != 0)
	{
		*str1 =  *str2;
		str1++;
		str2++;
	}
	
	*str1 = 0;
	
	return out;
}

bool string_isEqual(const char *str1, const char *str2, uint8_t maxLength)
{
    uint8_t j = 0;
    while((str1[j] != 0) && (str2[j] != 0) && (str1[j] == str2[j]) && (j<maxLength)){
        j++;
    }

    if(str1[j] == str2[j]) return true;
    else return false;
}

int string_parseInteger(const char *string, uint8_t maxLength)
{
    int output = 0;
    uint8_t i = 0;
    while(string[i] != 0 && i<maxLength){
        if(string[i] < '0' || string[i] > '9'){
            return 0;
        }
        output *= 10;
        output += string[i]-'0';
        i++;
    }
    return output;
}

float string_parseNumber(const char *string, uint8_t maxLength)
{
    float output = 0;
    int decimal = 0;
    uint8_t i = 0;
    while(string[i] != 0 && i<maxLength){
        if((string[i] < '0' || string[i] > '9') && string[i] != '.'){
            return 0;
        }
        if(string[i] == '.'){
            decimal = i;
        }else {
            output *= 10;
            output += (float)(string[i] - '0');
        }
        i++;
    }
    decimal = i-1-decimal;

    int divisor = 1;
    while(decimal--){
        divisor*=10;
    }
    output = output/(float)divisor;
    if(string[0] == '-'){
        output *= -1;
    }

    return output;
}

void string_removeFront(char *string, uint8_t removeLength, uint8_t maxLength)
{
    if(removeLength>maxLength) string[0] = 0;

    uint8_t i;
    for(i= 0; i < maxLength-removeLength; i++){
        string[i] = string[i+removeLength];
    }

    string[i] = 0;
}

uint16_t string_readLine(const char *string, uint16_t maxLength)
{
    uint16_t length = 0;
    while(string[length] != '\r' && string[length] != '\n')
    {
        length++;
        if(length >= maxLength || string[length] == 0x00){
            return 0;
        }
    }
    return length;
}



#ifdef __cplusplus
	}
#endif
