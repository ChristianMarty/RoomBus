
#ifndef ONEWIRE_H_
#define ONEWIRE_H_

#include "sam.h"
#include "kernel/kernel.h"

typedef enum {
	oneWire_DS18S20 = 0x10,
	oneWire_DS18B20 = 0x28,
	oneWire_VOID = 0xFF
}oneWire_familyCode_t;


typedef struct {
	uint64_t uid;
	oneWire_familyCode_t family;
	uint64_t romcode;
	
}oneWire_device_t;


typedef struct {
	uint64_t romcode;
	float *Temperature;
	uint32_t readTimer;
	uint16_t readTime;
}oneWire_readTemperature_t;


void oneWire_readRomCode(void);

void oneWire_readRun(const kernel_t *kernel, oneWire_readTemperature_t *readList, uint8_t readListSize);

uint8_t *oneWire_getRxBuffer();
void oneWire_sendReset(void);


void oneWire_init(const kernel_t *kernel);

uint64_t oneWire_parseRomData(uint8_t *rxRawBuffer);
int16_t  oneWire_parseTemperatureData(uint8_t *rxRawBuffer);


void oneWire_startConversion(void);
void oneWire_readScratchpad(uint64_t romcode);

oneWire_device_t oneWire_device_init(uint64_t romcode);

void oneWire_onTx(void);
void oneWire_onRx(void);

#endif /* ONEWIRE_H_ */