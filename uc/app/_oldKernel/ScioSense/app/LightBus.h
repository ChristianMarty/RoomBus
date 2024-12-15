#ifndef LIGHT_BUS_H_
#define LIGHT_BUS_H_

#include "sam.h"
#include "kernel/kernel.h"

#include "kernel/busController_IO.h"

void lightBus_init(const kernel_t *kernel);

void lightBus_handler(const kernel_t *kernel);

typedef struct{
	uint8_t wfe :1;
	uint8_t fadetime : 7;
} lightBus_fade_t;

void lightBus_set(const kernel_t *kernel, uint8_t address, lightBus_fade_t fade0, uint16_t ch0, lightBus_fade_t fade1, uint16_t ch1, lightBus_fade_t fade2, uint16_t ch2, lightBus_fade_t fade3, uint16_t ch3);
void lightBus_execute(const kernel_t *kernel, uint8_t address);

void lightBus_send(const kernel_t *kernel, const uint8_t *data, uint8_t size);

#endif /* LIGHT_BUS_H_ */