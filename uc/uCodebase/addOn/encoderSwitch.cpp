
#include "addOn/encoderSwitch.h"

#include "drv/SAMx5x/pin.h"
#include "kernel/busController_IO.h"

void encoderSwitch_init(encoderSwitch_t *encoderSwitch, const kernel_t *kernel)
{
	kernel->tickTimer.reset(&encoderSwitch->readTimer);
	encoderSwitch->hasChange = true;
	
	encoderSwitch->rxBuffer[0] = 0;
	encoderSwitch->rxBuffer[1] = 0;
	encoderSwitch->rxBuffer[2] = 0;
	encoderSwitch->rxBuffer[3] = 0;
	encoderSwitch->rxBuffer[4] = 0;
	encoderSwitch->rxBuffer[5] = 0;
	encoderSwitch->rxBuffer[6] = 0;
	
}

void encoderSwitch_handler(encoderSwitch_t *encoderSwitch, const kernel_t *kernel)
{
	if(!encoderSwitch->i2c->busy())
	{
 		if(kernel->tickTimer.delay1ms(&encoderSwitch->readTimer, 100))
		{
			encoderSwitch->i2cTransaction = encoderSwitch->i2c->transaction(encoderSwitch->address, 0x00, 0, 0, &encoderSwitch->rxBuffer[0], 7, 0);
		}
		else if(encoderSwitch->hasChange == true)
		{
			encoderSwitch->txBuffer[0] = encoderSwitch->led & 0xFF;
			encoderSwitch->txBuffer[1] = (encoderSwitch->led >> 8) & 0xFF;
			encoderSwitch->txBuffer[2] = encoderSwitch->brightness & 0xFF;
			encoderSwitch->txBuffer[3] = (encoderSwitch->brightness >> 8) & 0xFF;
		
			encoderSwitch->i2cTransaction = encoderSwitch->i2c->transaction(encoderSwitch->address, 0x00, &encoderSwitch->txBuffer[0], 4, 0, 0, 0);
			
			encoderSwitch->hasChange = false;
		}
	}
	
	if(encoderSwitch->i2c->getCurrentTransactionNumber() == encoderSwitch->i2cTransaction)
	{
		if(encoderSwitch->i2c->isUnreachable())
		{
			encoderSwitch->i2c->closeTransaction(encoderSwitch->i2cTransaction);
		}
		else if(!encoderSwitch->i2c->busy())
		{
			encoderSwitch->i2c->closeTransaction(encoderSwitch->i2cTransaction);
		
			encoderSwitch->up = encoderSwitch->rxBuffer[4];
			encoderSwitch->down = encoderSwitch->rxBuffer[5];
			encoderSwitch->button = (bool)encoderSwitch->rxBuffer[6];
			
			encoderSwitch->rxBuffer[4] = 0;
			encoderSwitch->rxBuffer[5] = 0;
		}
	}
}

void encoderSwitch_setRingLed(encoderSwitch_t *encoderSwitch, uint8_t ring)
{
	if(ring > 15) ring = 15; 
	
	uint16_t temp = 0x00;
	for(uint8_t i = 0; i< ring; i++)
	{
		temp = temp << 1;
		temp |= 1;
	}
	temp = temp << 1; // last led is outside of bar graph
	
	encoderSwitch->led &= 0x0001;
	encoderSwitch->led |= temp;	
	
	encoderSwitch->hasChange = true;
}

void encoderSwitch_setPowerLed(encoderSwitch_t *encoderSwitch, bool power)
{
	if(power) encoderSwitch->led |= 0x01;
	else encoderSwitch->led &= 0xFFFE;
	
	encoderSwitch->hasChange = true;
}

void encoderSwitch_setLedBrightness(encoderSwitch_t *encoderSwitch, uint16_t brightness)
{
	encoderSwitch->brightness = brightness;
	encoderSwitch->hasChange = true;
}
