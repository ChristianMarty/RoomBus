/*
 * SERCOM.cpp
 *
 * Created: 11.09.2018 22:34:12
 *  Author: Christian
 */ 


#include "uart.h"
#include "sercom.h"

uart_c::uart_c()
{

}

void uart_c::initUart(gclk_generator_t clkGenerator, Sercom *sercom_p, uint32_t baudrate, sercomParity_t parity)
{
	sercom_initClock(clkGenerator,sercom_p);
		
	_sercom_p = sercom_p;

	uint32_t ctra = 0x40120084;
	uint32_t ctrb = 0x00030000;
	
	bool stop2 = false;
	
	if(stop2)
	{
		ctrb |= 0x40;
	}
	
	if(parity != none) ctra |= 0x01000000;
	if(parity == odd) ctrb |= 0x00002000;
	
	_sercom_p->USART.CTRLA.reg = ctra;
	_sercom_p->USART.CTRLB.reg = ctrb;
	
	_sercom_p->USART.INTENSET.bit.RXC = true;
	_sercom_p->USART.INTENSET.bit.TXC = true;
	
	sercom_setBaudRate(sercom_p,16000000, baudrate);
	
	_sercom_p->USART.CTRLA.bit.ENABLE = true;
	
	_txBusy = false;
}


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

void uart_c::sendBytearray(bytearray_t *data)
{
	uart_c::sendData(data->data,data->size);
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
