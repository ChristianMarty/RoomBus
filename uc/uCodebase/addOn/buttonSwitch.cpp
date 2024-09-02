//**********************************************************************************************************************
// FileName : buttonSwitch.h
// FilePath : addOn/
// Author   : Christian Marty
// Date		: 01.09.2024
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#include "buttonSwitch.h"

#include "common/kernel.h"
#include "common/io_same51jx.h"

#include "driver/SAMx5x/i2cMaster.h"
#include "driver/SAMx5x/pin.h"

#include "addOn/i2cModul_rev2.h"


void buttonSwitch_init(buttonSwitch_t *buttonSwitch)
{
	buttonSwitch->state = buttonSwitch_state_init;
	buttonSwitch->button = 0x00;
}

void buttonSwitch_handler(buttonSwitch_t *buttonSwitch)
{
	if(!buttonSwitch->i2c->busy())
	{
		 if(kernel.tickTimer.delay1ms(&buttonSwitch->readTimer, 100))
		 {
			if(buttonSwitch->state == buttonSwitch_state_init)
			{
				uint8_t temp = 0x03;
				buttonSwitch->i2cTransaction = buttonSwitch->i2c->transaction(buttonSwitch->address,06,&temp,1,0,0,0);
				buttonSwitch->state = buttonSwitch_state_init_pending;
			}
			else if(buttonSwitch->state == buttonSwitch_state_run)
			{
				buttonSwitch->i2cTransaction = buttonSwitch->i2c->transaction(buttonSwitch->address, 0x01, 0, 0, &buttonSwitch->rxBuffer[0], 1, 0);
			}	
		}
		else if(buttonSwitch->state == buttonSwitch_state_run && buttonSwitch->hasChange == true)
		{
			buttonSwitch->i2cTransaction = buttonSwitch->i2c->transaction(buttonSwitch->address,0x02,&buttonSwitch->led,1,0,0,0);
			buttonSwitch->hasChange = false;
		}
	}
	
	if(buttonSwitch->i2c->getCurrentTransactionNumber() == buttonSwitch->i2cTransaction)
	{
		if(buttonSwitch->i2c->isUnreachable() )
		{
			buttonSwitch->state = buttonSwitch_state_init;
			buttonSwitch->i2c->closeTransaction(buttonSwitch->i2cTransaction);
		}
		else if( !buttonSwitch->i2c->busy())
		{
			if(buttonSwitch->state == buttonSwitch_state_run)
			{
				buttonSwitch->button = ((~buttonSwitch->rxBuffer[0] >> 2) & 0x3F);
			}
			
			if(buttonSwitch->state == buttonSwitch_state_init_pending)
			{
				buttonSwitch->state = buttonSwitch_state_run; // Discard first reading, since it is likely wrong
			}
			
			buttonSwitch->i2c->closeTransaction(buttonSwitch->i2cTransaction);
		}
	}
}

void buttonSwitch_setLed(buttonSwitch_t *buttonSwitch, uint8_t ledIndex, bool status)
{
	if(status)  buttonSwitch->led &= ~(0x80>>ledIndex);
	else buttonSwitch->led |= (0x80>>ledIndex);
	
	buttonSwitch->hasChange = true;
}

void buttonSwitch_setLeds(buttonSwitch_t *buttonSwitch, uint8_t leds)
{
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
	buttonSwitch->led = ~temp;
	
	buttonSwitch->hasChange = true;
}

void buttonSwitch_toggleLed(buttonSwitch_t *buttonSwitch, uint8_t ledIndex)
{
	buttonSwitch->led ^= (1<<ledIndex);
	
	buttonSwitch->hasChange = true;
}