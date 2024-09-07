//**********************************************************************************************************************
// FileName : encoderSwitch.h
// FilePath : addOn/
// Author   : Christian Marty
// Date		: 05.09.2024
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#ifndef ENCODER_SWITCH_MODUL_H_
#define ENCODER_SWITCH_MODUL_H_

#include "common/kernel.h"
#include "driver/SAMx5x/i2cMaster.h"

#define encoderSwitch_readInterval 100

typedef struct {
	uint8_t up;
	uint8_t down;
	
	bool button;
	bool oldButton;
}encoderSwitch_userInput_t;

typedef struct{
	uint8_t address;
	uint8_t port;
	
	uint16_t brightness;	
	
	i2cMaster_c *i2c;
	i2cMaster_c::i2c_transactionNumber_t _i2cTransaction;
	
	uint8_t _led;
	uint8_t _ledOld;
	
	uint32_t _readTimer;
	uint8_t _txBuffer[4];
	uint8_t _rxBuffer[7];
	
	encoderSwitch_userInput_t _userInput;
}encoderSwitch_t;

void encoderSwitch_init(encoderSwitch_t *encoderSwitch);
void encoderSwitch_handler(encoderSwitch_t *encoderSwitch);

encoderSwitch_userInput_t encoderSwitch_getUserInput(encoderSwitch_t *encoderSwitch);

void encoderSwitch_setRingLed(encoderSwitch_t *encoderSwitch, uint8_t ring);
void encoderSwitch_setPowerLed(encoderSwitch_t *encoderSwitch, bool power);
void encoderSwitch_setLedBrightness(encoderSwitch_t *encoderSwitch, uint16_t brightness);

#endif /* ENCODER_SWITCH_MODUL_H_ */