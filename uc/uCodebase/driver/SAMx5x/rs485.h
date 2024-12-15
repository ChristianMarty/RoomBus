/*
 * RS485.h
 *
 * Created: 05.11.2023
 * Author: Christian
 */ 


#ifndef RS485_H_
#define RS485_H_

#include "sam.h"
#include "genericClockController.h"
#include "utility/bytearray.h"
#include "sercom.h"
#include "common/kernel.h"

typedef enum {
	rs485_state_idle,
	rs485_state_transmit,
	rs485_state_receive,
	rs485_state_received,
	rs485_state_received_pause
} rs485_state_t;

typedef struct {
	Sercom *sercom;
	rs485_state_t state;
	
	uint32_t timeoutTimer;
	
	bytearray_t *tx;
	uint8_t txIndex;
	
	bytearray_t *rx;
	uint8_t rxSize;
	
	uint32_t pauseTimer;
	
	bool rxCompleted;
} rs485_t;

void rs485_init(const kernel_t *kernel, rs485_t *rs485, gclk_generator_t clkGenerator, Sercom *sercom, uint32_t baudrate, sercom_parity_t parity); 
void rs485_handler(const kernel_t *kernel, rs485_t *rs485);

bool rs485_send(const kernel_t *kernel, rs485_t *rs485);
bool rs485_rxCompleted(rs485_t *rs485);
bool rs485_busy(rs485_t *rs485);
void rs485_reset(rs485_t *rs485);

void rs485_rxInterrupt(rs485_t *rs485);
void rs485_txInterrupt(rs485_t *rs485);


#endif /* RS485_H_ */