
#ifndef ENCODER_SWITCH_MODUL_H_
#define ENCODER_SWITCH_MODUL_H_

#include "sam.h"
#include "kernel/kernel.h"
#include "drv/SAMx5x/pin.h"
#include "kernel/busController_IO.h"

#include "drv/SAMx5x/i2cMaster.h"

typedef struct{
	
	uint8_t address;
	uint8_t port;
	
	uint16_t led;
	uint16_t brightness;
	
	uint8_t up;
	uint8_t down;
	bool button;
	
	bool hasChange;
	uint8_t txBuffer[4];
	uint8_t rxBuffer[7];
	
	i2cMaster_c *i2c;
	i2cMaster_c::i2c_transactionNumber_t i2cTransaction;
	
	uint32_t readTimer;
}encoderSwitch_t;

void encoderSwitch_init(encoderSwitch_t *encoderSwitch, const kernel_t *kernel);
void encoderSwitch_handler(encoderSwitch_t *encoderSwitch, const kernel_t *kernel);

void encoderSwitch_setRingLed(encoderSwitch_t *encoderSwitch, uint8_t ring);
void encoderSwitch_setPowerLed(encoderSwitch_t *encoderSwitch, bool power);
void encoderSwitch_setLedBrightness(encoderSwitch_t *encoderSwitch, uint16_t brightness);

#endif /* ENCODER_SWITCH_MODUL_H_ */