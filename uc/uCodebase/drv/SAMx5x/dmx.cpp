/*
 * dmx.cpp
 *
 * Created: 11.09.2018 22:34:12
 *  Author: Christian
 */ 


#include "dmx.h"



dmx_c::dmx_c()
{

}

void dmx_c::init(gclk_generator_t clkGenerator, Sercom *sercom_p)
{
	// Setup clock
	if(sercom_p == SERCOM0)
	{
		gclk_peripheralEnable(GCLK_SERCOM0_CORE,clkGenerator);
		MCLK->APBAMASK.bit.SERCOM0_ = true;
	}
	else if(sercom_p == SERCOM1)
	{
		gclk_peripheralEnable(GCLK_SERCOM1_CORE,clkGenerator);
		MCLK->APBAMASK.bit.SERCOM1_ = true;
	}
	else if(sercom_p == SERCOM2)
	{
		gclk_peripheralEnable(GCLK_SERCOM2_CORE,clkGenerator);
		MCLK->APBBMASK.bit.SERCOM2_ = true;
	}
	else if(sercom_p == SERCOM3)
	{
		gclk_peripheralEnable(GCLK_SERCOM3_CORE,clkGenerator);
		MCLK->APBBMASK.bit.SERCOM3_ = true;
	}
	else if(sercom_p == SERCOM4)
	{
		gclk_peripheralEnable(GCLK_SERCOM4_CORE,clkGenerator);
		MCLK->APBDMASK.bit.SERCOM4_ = true;
	}
	else if(sercom_p == SERCOM5)
	{
		gclk_peripheralEnable(GCLK_SERCOM5_CORE,clkGenerator);
		MCLK->APBDMASK.bit.SERCOM5_ = true;
	}
		
	_sercom_p = sercom_p;
	_samplesPerBit = 16;

	uint32_t ctra = 0x42120084;
	uint32_t ctrb = 0x00010040;

	
	_sercom_p->USART.CTRLA.reg = ctra;
	_sercom_p->USART.CTRLB.reg = ctrb;
	_sercom_p->USART.CTRLC.bit.HDRDLY = 2;
	_sercom_p->USART.CTRLC.bit.BRKLEN = 3;

	//_sercom_p->USART.INTENSET.bit.TXC = true;
	
	setBaudRate(250000, 16000000);
	_sercom_p->USART.CTRLA.bit.ENABLE = true;
	
	
}

void dmx_c::sendBreak(void)
{
	_sercom_p->USART.CTRLB.bit.LINCMD = 1;
	
}

void dmx_c::setBaudRate(uint32_t baud, uint32_t clockFrequency)
{
	_sercom_p->USART.CTRLA.bit.ENABLE = false;
	_sercom_p->USART.BAUD.reg = (65536*(float)(1-(float)((float)_samplesPerBit*(float)baud/(float)clockFrequency)));
	_sercom_p->USART.CTRLA.bit.ENABLE = true;
}

/*
void uart_c::writeByte(uint8_t data)
{
	_sercom_p->USART.DATA.reg = data;
}

void uart_c::sendString(uint8_t *string)
{
	
}

bool uart_c::txBusy(void)
{
	return _txBusy;
}

void uart_c::sendData(uint8_t *data,uint8_t size)
{
	txData = data;
	_txSize = size;
	_txIndex = 0;
	_txBusy = true;
	writeByte(txData[_txIndex]);
	_txIndex++;
}

uint8_t uart_c::RxInterrupt(void)
{
	uint8_t data = _sercom_p->USART.DATA.reg;
	
	_sercom_p->USART.INTFLAG.bit.RXC = true;
	return data;
}

void uart_c::TxInterrupt(void)
{
	if(_txIndex<_txSize)
	{
		writeByte(txData[_txIndex]);
		_txIndex++;
	}
	else
	{
		_txBusy = false;
	}
	
	_sercom_p->USART.INTFLAG.bit.DRE = true;	
}

void uart_c::clearTxFlag(void)
{
	_sercom_p->USART.INTFLAG.bit.DRE = true;
}
*/