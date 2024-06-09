#ifndef LIGHT_BUS_H_
#define LIGHT_BUS_H_

#include "sam.h"
#include "kernel/kernel.h"

#include "utility/cobs.h"
#include "kernel/busController_IO.h"
#include "drv/SAMx5x/uart.h"



typedef void (*lb_onReceive_t)(const uint8_t *data, uint8_t size, bool error);
typedef enum {
		lightBus_state_idel,
		lightBus_state_transmit,
		lightBus_state_receive
} lightBus_state_t;

typedef struct{
	uart_c uartCom;
	uint32_t com_timer;
	lb_onReceive_t onReceive;
	cobs_decodeStream_t cobsDecodeStream;
	lightBus_state_t state;
}lightBus_t;


void lightBus_init(const kernel_t *kernel, lightBus_t *lightBus, Sercom *sercom_p);
void lightBus_handler(const kernel_t *kernel, lightBus_t *lightBus);

void lightBus_onRx(lightBus_t *lightBus);
void lightBus_onTx(lightBus_t *lightBus);


typedef struct{
	uint8_t wfe:1;
	uint8_t fadetime:7;
} lightBus_fade_t;

void lightBus_set(const kernel_t *kernel, lightBus_t *lightBus, uint8_t address, lightBus_fade_t fade0, uint16_t ch0, lightBus_fade_t fade1, uint16_t ch1, lightBus_fade_t fade2, uint16_t ch2, lightBus_fade_t fade3, uint16_t ch3);
void lightBus_execute(const kernel_t *kernel, lightBus_t *lightBus, uint8_t address);
bool lightBus_send(const kernel_t *kernel, lightBus_t *lightBus, const uint8_t *data, uint8_t size);

#endif /* LIGHT_BUS_H_ */