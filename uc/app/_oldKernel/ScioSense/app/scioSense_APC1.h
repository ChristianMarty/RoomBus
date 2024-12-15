
#ifndef MIDI_H_
#define MIDI_H_

#include "sam.h"
#include "kernel/kernel.h"
#include "drv/SAMx5x/uart.h"

typedef void (*midi_controllerChangeChange_t)(uint8_t index);

typedef struct{
	uint8_t channel;
	uint8_t command;
	uint8_t *value;
	
	midi_controllerChangeChange_t controllerChange;
}midi_controllerChange_t;

typedef struct{
	Sercom *sercom_p;
	midi_controllerChange_t *controllerChange;
	uint8_t controllerChangeSize;
	
	uart_c uartCom;

	volatile uint8_t rxIndex;
	volatile uint8_t rxMsgSize;
	volatile uint8_t rxData[5];
	
	uint8_t txData[5];
	
}midi_t;

void midi_init(const kernel_t *kernel, midi_t *midi);
void midi_handler(const kernel_t *kernel, midi_t *midi);

bool midi_sendControllerChange(midi_t *midi, uint8_t channel, uint8_t command, uint8_t value);

void midi_onRx(midi_t *midi);
void midi_onTx(midi_t *midi);

#endif /* MIDI_H_ */