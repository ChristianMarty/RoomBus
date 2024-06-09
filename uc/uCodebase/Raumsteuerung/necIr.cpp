
#include "necIr.h"
#include "drv/SAMx5x/timer.h"

uint8_t msg[69];
volatile uint8_t txPos;

#define multyply 35

#define ACTIVE_LEVEL false

pin_port_t _portNr;
uint8_t _pinNr;

void necIR_timer_interrupt(void)
{
	TC0->COUNT16.INTFLAG.reg = 0xFF; // Clear flag
	
	pin_tglOutput(_portNr,_pinNr);
	TC0->COUNT16.COUNT.reg = 0;
	timer_setPeriod(TC0, msg[txPos]*multyply);
	txPos ++;
	
	if(txPos>= sizeof(msg))
	{
		timer_disable(TC0);
		pin_setOutput(_portNr,_pinNr, ACTIVE_LEVEL);
	}
}

void necIR_init(const kernel_t *kernel, pin_port_t portNr, uint8_t pinNr)
{
	timer_init(TC0, kernel->clk_16MHz, TIMER_DIV256, false);
	
	kernel->nvic.assignInterruptHandler(TC0_IRQn, necIR_timer_interrupt);
	timer_capture0Interrupt(TC0, true);
	
	_portNr = portNr;
	_pinNr = pinNr;
	
	txPos = sizeof(msg);
	
	pin_enableOutput(_portNr,_pinNr);
	pin_setOutput(_portNr,_pinNr,ACTIVE_LEVEL);
}

bool necIR_busy(void)
{
	return (txPos < sizeof(msg));
}

void necIR_transmitCode(const kernel_t *kernel, uint8_t customCode, uint8_t dataCode)
{	
	msg[0] = 16;
	msg[1] = 8;
	
	for(uint8_t i = 0; i<16; i +=2)
	{
		// Add customCode
		msg[i+2] = 1;
		if(customCode & 0x01) msg[i+3] = 3;
		else msg[i+3] = 1;
		
		// Add customCode
		msg[i+18] = 1;
		if(customCode & 0x01) msg[i+19] = 1;
		else msg[i+19] = 3;
		
		customCode = (customCode>>1);
	}
	
	for (uint8_t i = 0; i<16; i +=2)
	{
		// Add dataCode
		msg[i+34] = 1;
		if(dataCode & 0x01) msg[i+35] = 3;
		else msg[i+35] = 1;
		
		// Add ~dataCode
		msg[i+50] = 1;
		if(dataCode & 0x01) msg[i+51] = 1;
		else msg[i+51] = 3;
		
		dataCode = (dataCode>>1);
	}
	
	msg[66] = 1; // Stop Bit
	msg[67] = 3;
	msg[68] = 1;
	
	txPos = 0;
	timer_setPeriod(TC0, msg[txPos]*multyply);
	txPos ++;
	timer_enable(TC0);
	pin_setOutput(_portNr,_pinNr, !ACTIVE_LEVEL);
}