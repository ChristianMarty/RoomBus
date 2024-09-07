//**********************************************************************************************************************
// FileName : buttonSwitch.cpp
// FilePath : addOn/
// Author   : Christian Marty
// Date		: 05.09.2024
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#include "buttonSwitch.h"

void buttonSwitch_init(buttonSwitch_t *buttonSwitch)
{
	buttonSwitch->_state = buttonSwitch_state_init;
	buttonSwitch->_userInput.current = 0x00;
	buttonSwitch->_userInput.old = buttonSwitch->_userInput.current;
}

void buttonSwitch_handler(buttonSwitch_t *buttonSwitch)
{
	if(!buttonSwitch->i2c->busy())
	{
		 if(kernel.tickTimer.delay1ms(&buttonSwitch->_readTimer, buttonSwitch_readInterval))
		 {
			if(buttonSwitch->_state == buttonSwitch_state_init)
			{
				uint8_t temp = 0x03;
				buttonSwitch->_i2cTransaction = buttonSwitch->i2c->transaction(buttonSwitch->address, 0x06, &temp, 1, 0, 0, 0);
				buttonSwitch->_state = buttonSwitch_state_init_pending;
			}
			else if(buttonSwitch->_state == buttonSwitch_state_run)
			{
				buttonSwitch->_i2cTransaction = buttonSwitch->i2c->transaction(buttonSwitch->address, 0x01, 0, 0, &buttonSwitch->_rxBuffer[0], sizeof(buttonSwitch->_rxBuffer), 0);
			}	
		}
		else if(buttonSwitch->_state == buttonSwitch_state_run && (buttonSwitch->_ledOld != buttonSwitch->_led))
		{
			buttonSwitch->_i2cTransaction = buttonSwitch->i2c->transaction(buttonSwitch->address, 0x02, &buttonSwitch->_led, 1, 0, 0, 0);
			buttonSwitch->_ledOld= buttonSwitch->_led;
		}
	}
	
	if(buttonSwitch->i2c->getCurrentTransactionNumber() == buttonSwitch->_i2cTransaction)
	{
		if(buttonSwitch->i2c->isUnreachable() )
		{
			buttonSwitch->_state = buttonSwitch_state_init;
			buttonSwitch->i2c->closeTransaction(buttonSwitch->_i2cTransaction);
		}
		else if( !buttonSwitch->i2c->busy())
		{
			if(buttonSwitch->_state == buttonSwitch_state_run)
			{
				buttonSwitch->_userInput.current = ((~buttonSwitch->_rxBuffer[0] >> 2) & 0x3F);
			}
			
			if(buttonSwitch->_state == buttonSwitch_state_init_pending)
			{
				buttonSwitch->_state = buttonSwitch_state_run; // Discard first reading, since it is likely wrong
			}
			
			buttonSwitch->i2c->closeTransaction(buttonSwitch->_i2cTransaction);
		}
	}
}

void buttonSwitch_setLed(buttonSwitch_t *buttonSwitch, uint8_t ledIndex, bool state)
{
	buttonSwitch->_ledOld= buttonSwitch->_led;
	
	if(state)  buttonSwitch->_led &= ~(0x80>>ledIndex);
	else buttonSwitch->_led |= (0x80>>ledIndex);
}

void buttonSwitch_setLeds(buttonSwitch_t *buttonSwitch, uint8_t leds)
{
	buttonSwitch->_ledOld= buttonSwitch->_led;
	
	leds &= 0x3F;
	uint8_t temp = 0;
	
	for(uint8_t i= 0; i<6; i++)
	{
		if(leds & 0x01) temp |= 0x01;
		else temp &= 0xFE;
		
		temp = (temp<<1);
		leds = (leds>>1);
	}
	
	temp = (temp<<1);
	buttonSwitch->_led = ~temp;
}

void buttonSwitch_toggleLed(buttonSwitch_t *buttonSwitch, uint8_t ledIndex)
{
	buttonSwitch->_ledOld= buttonSwitch->_led;
	buttonSwitch->_led ^= (1<<ledIndex);
}

buttonSwitch_userInput_t buttonSwitch_getUserInput(buttonSwitch_t *buttonSwitch)
{
	buttonSwitch_userInput_t output = buttonSwitch->_userInput;
	buttonSwitch->_userInput.old = buttonSwitch->_userInput.current;
	return output;
}