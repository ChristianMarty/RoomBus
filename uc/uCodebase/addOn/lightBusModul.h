#ifndef LIGHT_BUS_MODUL_H_
#define LIGHT_BUS_MODUL_H_

#include "sam.h"
#include "common/kernel.h"
#include "common/io_same51jx.h"
#include "common/typedef.h"

#include "interface/lightBus.h"

#include "utility/cobs.hpp"

#include "driver/SAMx5x/dmx.h"
#include "driver/SAMx5x/dma.h"
#include "driver/SAMx5x/uart.h"

typedef enum {
	lightBusModul_channel_1,
	lightBusModul_channel_2
}lightBusModul_channel_t;

typedef void (*lightBusModul_onReceive_t)(lightBusModul_channel_t channel, const uint8_t *data, uint8_t size, bool error);

typedef struct {
	
	bool dmx_enable;
	uint8_t *dmx_data;
	uint16_t dmx_data_size;
	
/*	bool uart_enable;
	uint8_t *uart_data;
	uint16_t uart_data_size;*/
	
	lightBusModul_onReceive_t lightBusModul_onReceive;
	
	
}lightBusModul_t;


void lightBusModul_init(lightBusModul_t *lightBusModul);
void lightBusModul_handler(lightBusModul_t *lightBusModul);

void lightBusModul_send(lightBusModul_channel_t channel, const uint8_t *data, uint8_t size);
void lightBusModul_set(lightBusModul_channel_t channel, uint8_t address, lightBus_fade_t fade0, uint16_t ch0, lightBus_fade_t fade1, uint16_t ch1, lightBus_fade_t fade2, uint16_t ch2, lightBus_fade_t fade3, uint16_t ch3);
void lightBusModul_execute(lightBusModul_channel_t channel, uint8_t address);

uint16_t lightBusModul_uart_dataAvailable(lightBusModul_t *lightBusModul);
void lightBusModul_uart_dataClear(lightBusModul_t *lightBusModul);
uint8_t *lightBusModul_uart_data(lightBusModul_t *lightBusModul);


#endif /* LIGHT_BUS_MODUL_H_ */