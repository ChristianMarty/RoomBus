//**********************************************************************************************************************
// FileName : pin.h
// FilePath : uCodebase/driver/SAMx5x/kernel/
// Author   : Christian Marty
// Date		: 15.02.2019
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#ifndef PIN_H_
#define PIN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "common/typedef.h"
	
typedef enum{
	PIN_FUNCTION_A,
	PIN_FUNCTION_B,
	PIN_FUNCTION_C,
	PIN_FUNCTION_D,
	PIN_FUNCTION_E,
	PIN_FUNCTION_F,
	PIN_FUNCTION_G,
	PIN_FUNCTION_H,
	PIN_FUNCTION_I,

	PIN_FUNCTION_SIZE
} pin_peripheralFunction_t;

typedef enum{
	PIN_PORT_A,
	PIN_PORT_B,
	PIN_PORT_C,
	PIN_PORT_D,
	
	PIN_PORT_SIZE
} pin_port_t;

typedef struct{
	pin_port_t port;
	uint8_t pin;	
}pin_t;

static inline void pin_enablePullUp(uint8_t portNr, uint8_t pinNr)
{
	PORT->Group[portNr].PINCFG[pinNr].bit.PULLEN = true;
}

static inline void pin_disablePullUp(uint8_t portNr, uint8_t pinNr)
{
	PORT->Group[portNr].PINCFG[pinNr].bit.PULLEN = false;
}

static inline void pin_enableStrongDriver(uint8_t portNr, uint8_t pinNr)
{
	PORT->Group[portNr].PINCFG[pinNr].bit.DRVSTR = true;
}

static inline void pin_disableStrongDriver(uint8_t portNr, uint8_t pinNr)
{
	PORT->Group[portNr].PINCFG[pinNr].bit.DRVSTR = false;
}

static inline void pin_enableInput(pin_port_t portNr, uint8_t pinNr)
{
	PORT->Group[portNr].PINCFG[pinNr].bit.INEN = true;
}

static inline void pin_disableInput(pin_port_t portNr, uint8_t pinNr)
{
	PORT->Group[portNr].PINCFG[pinNr].bit.INEN = false;
}

static inline bool pin_getInput(pin_port_t portNr, uint8_t pinNr)
{
	return	(bool)(PORT->Group[portNr].IN.reg & (1<<pinNr));
}

static inline void pin_enableOutput(pin_port_t portNr, uint8_t pinNr)
{
	PORT->Group[portNr].DIRSET.reg = (1<<pinNr);
}

static inline void pin_disableOutput(pin_port_t portNr, uint8_t pinNr)
{
	PORT->Group[portNr].DIRCLR.reg = (1<<pinNr);
}

static inline void pin_setOutput(pin_port_t portNr, uint8_t pinNr, bool state)
{
	if(state) PORT->Group[portNr].OUTSET.reg = (1<<pinNr);
	else  PORT->Group[portNr].OUTCLR.reg = (1<<pinNr);
}

static inline void pin_tglOutput(pin_port_t portNr, uint8_t pinNr)
{
	PORT->Group[portNr].OUTTGL.reg = (1<<pinNr);
}

static inline void pin_enablePeripheralMux(pin_port_t portNr, uint8_t pinNr, pin_peripheralFunction_t function)
{
	uint8_t temp = PORT->Group[portNr].PMUX[pinNr/2].reg;
	
	temp &= (0x00F0>>(4*(pinNr%2)));
	temp |=(function<<(4*(pinNr%2)));
	
	PORT->Group[portNr].PMUX[pinNr/2].reg = temp;
	PORT->Group[portNr].PINCFG[pinNr].bit.PMUXEN = true;
}

static inline void pin_disablePeripheralMux(pin_port_t portNr, uint8_t pinNr)
{
	PORT->Group[portNr].PINCFG[pinNr].bit.PMUXEN = false;
}

#ifdef __cplusplus
}
#endif

#endif /* PIN_H_ */