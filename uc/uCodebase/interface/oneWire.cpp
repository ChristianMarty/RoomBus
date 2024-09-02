

#include "oneWire.h"
#include "utility/softCRC.h"

#include "drv/SAMx5x/uart.h"

uart_c uartCom;
uint8_t channel;
uint8_t loop;
uint32_t readTimer;

void oneWire_init(const kernel_t *kernel)
{
	uartCom.initUart(kernel->clk_16MHz, SERCOM5, 9600, uart_c::none);
		
	channel = 0;
	loop = 0;
	
}


uint8_t txBuffer[500];
uint16_t txBufferSize;

uint8_t rxBuffer[500];
int16_t rxBufferPos;
uint16_t rxBufferSize;

uint8_t parseByte(uint8_t *rxRawBuffer);

void oneWire_onRx(void)
{
	uint8_t data = uartCom.RxInterrupt();
	
	if(rxBufferPos >= 0) rxBuffer[rxBufferPos] = data;
	rxBufferPos++;
}

void oneWire_onTx(void)
{
	uartCom.TxInterrupt();
}

void oneWire_sendReset(void)
{
	uartCom.setBaudRate(9600,16000000);
	
	uint8_t txData[] = {0xF0};
	uartCom.sendData(&txData[0], sizeof(txData));
}


oneWire_familyCode_t oneWire_getTypeFromRomcode(uint64_t romcode)
{
	return (oneWire_familyCode_t)(romcode&0xFF);
}

void oneWire_readRun(const kernel_t *kernel, oneWire_readTemperature_t *readList, uint8_t readListSize)
{
	if(kernel->tickTimer.delay1ms(&readTimer,40))
	{
		if(loop == 0)
		{
			oneWire_sendReset();
			channel = 0;
		}
		else if(loop == 1)
		{
			oneWire_startConversion();
		}
		else if(channel < readListSize) 
		{
			if(!(loop %2))
			{
				oneWire_sendReset();
			}
			else
			{
				oneWire_readScratchpad(readList[channel].romcode); 
			}
		}
		
		loop++;
		if(loop > 100) loop = 0;
	}
		
	if(rxBufferSize != 0 && rxBufferPos == rxBufferSize)
	{
		int16_t tempVal = oneWire_parseTemperatureData(&rxBuffer[0]);
		if(tempVal == (int16_t)0xFFFF) 
		{
			kernel->log.warning("OneWire CRC Error");
		}
		else
		{
			float outVal = 0;
			oneWire_familyCode_t type = oneWire_getTypeFromRomcode(readList[channel].romcode);
			
			if( type == oneWire_DS18B20)
			{
				outVal = (float)((tempVal*625)/1000);
				outVal = outVal/10;
			}
			else if (type == oneWire_DS18S20)
			{
				uint8_t remain = parseByte(&rxBuffer[48]);
				uint8_t count = parseByte(&rxBuffer[56]);
				
				outVal = ((float)tempVal/2)-0.25;+(((float)count-(float)remain)/(float)count);
			}
			
			*(readList[channel].Temperature) = outVal;
			channel++;
		}
		rxBufferSize = 0;
	}	
}


void transaction(uint8_t *txData, uint8_t txSize, uint8_t rxSize)
{
	uartCom.setBaudRate(115200,16000000);
	
	txBufferSize = 0;
	
	for(uint8_t i = 0; i <txSize; i++)
	{
		uint8_t temp = txData[i];
		
		for(uint8_t j = 0; j < 8; j++)
		{
			if(temp & 0x01) txBuffer[txBufferSize] = 0xFF;
			else txBuffer[txBufferSize] = 0x00;
			txBufferSize++;
			
			temp = (temp>>1);
		}
	}
	
	for(uint8_t i = 0; i< (rxSize*8); i++)
	{
		txBuffer[txBufferSize] = 0xFF;
		txBufferSize++;
	}
	
	rxBufferPos = 0;
	rxBufferSize = 0;
	if(rxSize) 
	{
		rxBufferPos = (-8*txSize);
		rxBufferSize = rxSize*8;
	}
	
	uartCom.sendData(&txBuffer[0], txBufferSize);
}

void oneWire_startConversion(void)
{
	uint8_t txData[] = {0xCC,0x44};
	transaction(&txData[0], sizeof(txData) ,0);
}

void oneWire_readScratchpad(uint64_t romcode)
{
	uint8_t txData[] = {0x55,
		(uint8_t)(romcode),
		(uint8_t)(romcode>>8),
		(uint8_t)(romcode>>16),
		(uint8_t)(romcode>>24),
		(uint8_t)(romcode>>32),
		(uint8_t)(romcode>>40),
		(uint8_t)(romcode>>48),
		(uint8_t)(romcode>>56),
		0xBE
	};
	transaction(&txData[0], sizeof(txData) ,9);
}

void oneWire_readRomCode(void)
{
	uint8_t txData[] = {0x33};
	transaction(&txData[0], 1 ,8);
}


uint8_t *oneWire_getRxBuffer(void)
{
	return &rxBuffer[0];
}

oneWire_device_t oneWire_device_init(uint64_t romcode)
{
	oneWire_device_t device;
	device.family = oneWire_VOID;
	device.uid = 0;
	device.romcode = 0;
	
	uint8_t * crc_p = (uint8_t*)&romcode;
	
	uint8_t crc  =  crc8_oneWire(&crc_p[0],8);
	
	if(crc != 0) return device;
	
	device.family = (oneWire_familyCode_t)(romcode&0xFF);
	device.uid = (romcode>>8)&0x0000FFFFFFFFFFFF;
	device.romcode = romcode;
	
	return device;
}

uint8_t parseByte(uint8_t *rxRawBuffer)
{
	uint8_t temp = 0;
	for(uint8_t j = 0; j< 8; j++)
	{
		temp = (temp>>1);
		if(rxRawBuffer[j] >= 0xFE) temp |= 0x80;
	}
	return temp;
}

int16_t oneWire_parseTemperatureData(uint8_t *rxRawBuffer)
{
	uint8_t rxMsg[9];
	
	for(uint8_t i = 0; i< 9; i++)
	{
		rxMsg[i] = parseByte(&rxRawBuffer[i*8]);
	}
	
	int16_t temperatureData = (((uint16_t) rxMsg[1])<<8) | rxMsg[0];
	
	uint8_t crc  =  crc8_oneWire(&rxMsg[0],8);
	
	if( crc != rxMsg[8]) return (int16_t)0xFFFF;
	else return temperatureData;
}

uint64_t oneWire_parseRomData(uint8_t *rxRawBuffer)
{
	uint8_t rxMsg[8];
	
	for(uint8_t i = 0; i< 8; i++)
	{
		rxMsg[i] = parseByte(&rxRawBuffer[i*8]);
	}
	
	uint64_t romcode = 0;
	romcode |= (((uint64_t) rxMsg[0]));
	romcode |= (((uint64_t) rxMsg[1])<<8);
	romcode |= (((uint64_t) rxMsg[2])<<16);
	romcode |= (((uint64_t) rxMsg[3])<<24);
	romcode |= (((uint64_t) rxMsg[4])<<32);
	romcode |= (((uint64_t) rxMsg[5])<<40);
	romcode |= (((uint64_t) rxMsg[6])<<48);
	romcode |= (((uint64_t) rxMsg[7])<<56);
	
	uint8_t crc  =  crc8_oneWire(&rxMsg[0],7);
	
	if( crc != rxMsg[7]) return (uint64_t)0xFFFFFFFFFFFFFFFF;
	else return romcode;
}