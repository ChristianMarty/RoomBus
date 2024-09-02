
#ifndef BUTTON_SWITCH_MODUL_H_
#define BUTTON_SWITCH_MODUL_H_

#include "common/kernel.h"
#include "driver/SAMx5x/i2cMaster.h"

typedef enum {
	buttonSwitch_state_init = 0, 
	buttonSwitch_state_init_pending = 1, 
	buttonSwitch_state_run = 2, 
	buttonSwitch_state_error = 0xFF
} buttonSwitch_state_t;
	
typedef struct{
	
	uint8_t address;
	uint8_t port;
	
	uint8_t led;
	uint16_t brightness;
	
	uint8_t button;
	
	bool hasChange;
	uint8_t rxBuffer[1];
	
	i2cMaster_c *i2c;
	i2cMaster_c::i2c_transactionNumber_t i2cTransaction;
	
	uint32_t readTimer;
	
	bool init;
	
	buttonSwitch_state_t state;
		
}buttonSwitch_t;


void buttonSwitch_init(buttonSwitch_t *buttonSwitch);
void buttonSwitch_handler(buttonSwitch_t *buttonSwitch);

void buttonSwitch_setLed(buttonSwitch_t *buttonSwitch, uint8_t ledIndex, bool status);
void buttonSwitch_setLeds(buttonSwitch_t *buttonSwitch, uint8_t ledStatus);
void buttonSwitch_toggleLed(buttonSwitch_t *buttonSwitch, uint8_t ledIndex);

#endif /* BUTTON_SWITCH_MODUL_H_ */