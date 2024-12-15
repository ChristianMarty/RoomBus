//**********************************************************************************************************************
// FileName : buttonSwitch.h
// FilePath : addOn/
// Author   : Christian Marty
// Date		: 05.09.2024
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#ifndef BUTTON_SWITCH_MODUL_H_
#define BUTTON_SWITCH_MODUL_H_

#include "common/kernel.h"
#include "driver/SAMx5x/i2cMaster.h"

#define buttonSwitch_readInterval 100

typedef enum {
	buttonSwitch_state_init = 0, 
	buttonSwitch_state_init_pending = 1, 
	buttonSwitch_state_run = 2, 
	buttonSwitch_state_error = 0xFF
} buttonSwitch_state_t;

typedef struct {
	uint8_t current;
	uint8_t old;
}buttonSwitch_userInput_t;
	
typedef struct{
	uint8_t address;
	uint8_t port;
	
	uint16_t brightness;
	i2cMaster_c *i2c;
	
	uint32_t _readTimer;
	i2cMaster_c::i2c_transactionNumber_t _i2cTransaction;
	uint8_t _rxBuffer[1];
	
	uint8_t _led;
	uint8_t _ledOld;
	buttonSwitch_state_t _state;
	
	buttonSwitch_userInput_t _userInput;
	uint8_t _byteBuffer;
}buttonSwitch_t;


void buttonSwitch_init(buttonSwitch_t *buttonSwitch);
void buttonSwitch_handler(buttonSwitch_t *buttonSwitch);

buttonSwitch_userInput_t buttonSwitch_getUserInput(buttonSwitch_t *buttonSwitch);

void buttonSwitch_setLed(buttonSwitch_t *buttonSwitch, uint8_t ledIndex, bool state);
void buttonSwitch_setLeds(buttonSwitch_t *buttonSwitch, uint8_t ledStatus);
void buttonSwitch_toggleLed(buttonSwitch_t *buttonSwitch, uint8_t ledIndex);

#endif /* BUTTON_SWITCH_MODUL_H_ */