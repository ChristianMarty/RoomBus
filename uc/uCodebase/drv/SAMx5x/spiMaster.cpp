

#include "spiMaster.h"

#include "sercom.h"


void spiMaster_c::init(const kernel_t *kernel, Sercom *sercom_p, gclk_generator_t clkGenerator, uint8_t clockDevisor)
{
	sercom_initClock(clkGenerator,sercom_p);
	_clockDevisor = clockDevisor;
	
	_kernel = kernel;
	_sercom_p = sercom_p;
	_clkGenerator = clkGenerator;
	
	_sercom_p->SPI.CTRLA.bit.SWRST = true; // Reset
	while(_sercom_p->SPI.CTRLA.bit.SWRST);
	
	_sercom_p->SPI.CTRLA.bit.MODE = 0x03; // SPI Master mode
	
	//_sercom_p->SPI.CTRLB.bit.MSSEN = true; // SS control enable
	_sercom_p->SPI.BAUD.bit.BAUD = clockDevisor;
	
	_sercom_p->SPI.CTRLC.bit.ICSPACE = 01; // 1 clk Inter-Character Spacing
	
	while(_sercom_p->SPI.SYNCBUSY.bit.SWRST);
	
	_sercom_p->SPI.CTRLA.bit.ENABLE = true;
	

	
}

void spiMaster_c::handler(void)
{
	if(_sercom_p->SPI.INTFLAG.bit.TXC)
	{
		_sercom_p->SPI.INTFLAG.bit.TXC = 1;
		_busy = false;
	}

}

bool spiMaster_c::busy(void)
{
	return _busy;
}

void spiMaster_c::send(uint32_t data)
{
	_busy = true;
	_sercom_p->SPI.DATA.reg = data;
	
	while(!_sercom_p->SPI.INTFLAG.bit.TXC);
	_sercom_p->SPI.INTFLAG.bit.TXC = 1;
}

void spiMaster_c::interruptHandler_0(void)
{
	
}

void spiMaster_c::interruptHandler_1(void)
{
	
}

void spiMaster_c::interruptHandler_2(void)
{
	
}

void spiMaster_c::interruptHandler_3(void)
{
	
}

