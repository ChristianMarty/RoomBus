//**********************************************************************************************************************
// FileName : encoderSwitch.cpp
// FilePath : addOn/
// Author   : Christian Marty
// Date		: 05.09.2024
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#include "addOn/encoderSwitch.h"

void encoderSwitch_init(encoderSwitch_t *encoderSwitch)
{
	kernel.tickTimer.reset(&encoderSwitch->_readTimer);
	
	encoderSwitch->_led = 0;
	encoderSwitch->_ledOld = encoderSwitch->_led;
	
	encoderSwitch->_rxBuffer[0] = 0;
	encoderSwitch->_rxBuffer[1] = 0;
	encoderSwitch->_rxBuffer[2] = 0;
	encoderSwitch->_rxBuffer[3] = 0;
	encoderSwitch->_rxBuffer[4] = 0;
	encoderSwitch->_rxBuffer[5] = 0;
	encoderSwitch->_rxBuffer[6] = 0;
}

void encoderSwitch_handler(encoderSwitch_t *encoderSwitch)
{
	if(!encoderSwitch->i2c->busy())
	{
 		if(kernel.tickTimer.delay1ms(&encoderSwitch->_readTimer, encoderSwitch_readInterval))
		{
			encoderSwitch->_i2cTransaction = encoderSwitch->i2c->transaction(encoderSwitch->address, 0x00, 0, 0, &encoderSwitch->_rxBuffer[0], sizeof(encoderSwitch->_rxBuffer), 0);
		}
		else if(encoderSwitch->_ledOld != encoderSwitch->_led)
		{
			encoderSwitch->_txBuffer[0] = encoderSwitch->_led & 0xFF;
			encoderSwitch->_txBuffer[1] = (encoderSwitch->_led >> 8) & 0xFF;
			encoderSwitch->_txBuffer[2] = encoderSwitch->brightness & 0xFF;
			encoderSwitch->_txBuffer[3] = (encoderSwitch->brightness >> 8) & 0xFF;
		
			encoderSwitch->_i2cTransaction = encoderSwitch->i2c->transaction(encoderSwitch->address, 0x00, &encoderSwitch->_txBuffer[0], 4, 0, 0, 0);
			
			encoderSwitch->_ledOld = encoderSwitch->_led;
		}
	}
	
	if(encoderSwitch->i2c->getCurrentTransactionNumber() == encoderSwitch->_i2cTransaction)
	{
		if(encoderSwitch->i2c->isUnreachable())
		{
			encoderSwitch->i2c->closeTransaction(encoderSwitch->_i2cTransaction);
		}
		else if(!encoderSwitch->i2c->busy())
		{
			encoderSwitch->i2c->closeTransaction(encoderSwitch->_i2cTransaction);
		
			encoderSwitch->_userInput.up += encoderSwitch->_rxBuffer[4];
			encoderSwitch->_userInput.down += encoderSwitch->_rxBuffer[5];
			encoderSwitch->_userInput.button = (bool)encoderSwitch->_rxBuffer[6];
			
			encoderSwitch->_rxBuffer[4] = 0;
			encoderSwitch->_rxBuffer[5] = 0;
		}
	}
}

void encoderSwitch_setRingLed(encoderSwitch_t *encoderSwitch, uint8_t ring)
{
	encoderSwitch->_ledOld = encoderSwitch->_led;
	
	if(ring > 15) ring = 15; 
	
	uint16_t temp = 0x00;
	for(uint8_t i = 0; i< ring; i++)
	{
		temp = temp << 1;
		temp |= 1;
	}
	temp = temp << 1; // last led is outside of bar graph
	
	encoderSwitch->_led &= 0x0001;
	encoderSwitch->_led |= temp;	
}

void encoderSwitch_setPowerLed(encoderSwitch_t *encoderSwitch, bool power)
{
	encoderSwitch->_ledOld = encoderSwitch->_led;
	
	if(power) encoderSwitch->_led |= 0x01;
	else encoderSwitch->_led &= 0xFFFE;
}

void encoderSwitch_setLedBrightness(encoderSwitch_t *encoderSwitch, uint16_t brightness)
{
	encoderSwitch->brightness = brightness;
}

encoderSwitch_userInput_t encoderSwitch_getUserInput(encoderSwitch_t *encoderSwitch)
{
	encoderSwitch_userInput_t output = encoderSwitch->_userInput;
	encoderSwitch->_userInput.up = 0;
	encoderSwitch->_userInput.down = 0;
	encoderSwitch->_userInput.oldButton = encoderSwitch->_userInput.button;
	return output;	
}