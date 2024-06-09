
#ifndef LIGHTSWITCH_H_
#define LIGHTSWITCH_H_

#include "sam.h"
#include "drv/SAMx5x/i2cMaster.h"
#include "drv/SAMx5x/pin.h"

class lightswitch_c
{
	public:
		void init(i2cMaster_c *i2c, uint8_t address);
		
		void updateButton(void);
		void updateLed(void);
		
		void setLed(uint8_t ledStatus);
		void setLed(uint8_t ledNr, bool status);
		void toggleLed(uint8_t ledNr);
		
		uint8_t getButtons(void);
			
	private:
	
		uint8_t _address;
		i2cMaster_c *_I2C;	
		
		uint8_t _button;
		uint8_t _led;
};
#endif /* LIGHTSWITCH_H_ */