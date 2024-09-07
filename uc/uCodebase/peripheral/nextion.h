//**********************************************************************************************************************
// FileName : nextion.h
// FilePath : peripheral/
// Author   : Christian Marty
// Date		: 05.09.2024
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#ifndef NEXTION_H_
#define NEXTION_H_

#include "common/kernel.h"

#include "driver/SAMx5x/pin.h"
#include "driver/SAMx5x/uart.h"

#define nextion_displayTimeout 10000

typedef void (*touchEventHandler_t)(uint8_t index, uint8_t page, uint8_t component);
typedef void (*displayOnOffCallback_t)(void);

typedef enum  {
	nextion_state_off,
	nextion_state_waitForOn,
	nextion_state_on
}nextion_state_t;

typedef struct  {
	uint8_t page;
	uint8_t component;
	uint8_t objName[4];
	touchEventHandler_t eventHandler;
}nextion_touchEventAction_t;

typedef struct  {
	uint8_t state:1;
	uint8_t oldState:1;
}nextion_buttonState_t;

typedef struct{
	const nextion_touchEventAction_t *touchEventAction;
	nextion_buttonState_t *buttonState;
	
	uint8_t touchEventActionSize;
	
	uart_c *uart;
	
	displayOnOffCallback_t displayOn;
	displayOnOffCallback_t displayOff;
	
	uint8_t _txBuffer[64];
	uint8_t _txCounter;

	uint8_t _rxBuffer[32];
	uint8_t _rxCounter;
	uint8_t _ffCounter;
	
	uint32_t _displayOnTimer;
	uint32_t _displayComTimer;
	
	uint8_t _currentPage;
	uint8_t _lastPage;
	
	nextion_state_t _state;
}nextion_t;

void nextion_init(nextion_t *nextion);
void nextion_handler(nextion_t *nextion);
void nextion_rxHandler(nextion_t *nextion, uint8_t byte);

void nextion_setPage(nextion_t *nextion, uint8_t page);

#endif /* NEXTION_H_ */