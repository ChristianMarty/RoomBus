

#include "lichtschalter.h"
uint8_t initData[5] = {0xFF,0x00,0xFF,0x00,0xFF};

void lightswitch_c::init(i2cMaster_c *i2c, uint8_t address)
{
	_I2C = i2c;
	_address = address;
	uint8_t temp = 0x03;
	_button = 0x00;
	_led = 0xFF;
	
	//_I2C->transaction(_address,03,&initData[0],5,0,0);
	_I2C->transaction(_address,06,&temp,1,0,0,0);
}

void lightswitch_c::updateButton(void)
{
	_I2C->transaction(_address,0x01,0,0,&_button,1,0);
}

void lightswitch_c::updateLed(void)
{
	_I2C->transaction(_address,0x02,&_led,1,0,0,0);
}

void lightswitch_c::setLed(uint8_t ledStatus)
{
	ledStatus &= 0x3F;
	uint8_t temp = 0;
	
	for(uint8_t i= 0; i<6; i++)
	{
		if(ledStatus & 0x01) temp |= 0x01;
		else temp &= 0xFE;
		
		temp = (temp<<1);
		ledStatus = (ledStatus>>1);
	}
	
	temp = (temp<<1);
	_led = ~temp;
}

void lightswitch_c::setLed(uint8_t ledNr, bool status)
{
	if(status)  _led &= ~(0x80>>ledNr);
	else _led |= (0x80>>ledNr);
}

void lightswitch_c::toggleLed(uint8_t ledNr)
{
	_led ^= (1<<ledNr);
}

uint8_t lightswitch_c::getButtons(void)
{
	return ((~_button >> 2) & 0x3F);	
}