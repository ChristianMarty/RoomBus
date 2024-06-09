/*
 * RS485.cpp
 *
 * Created: 05.11.2023
 * Author: Christian
 */ 
#include "rs485.h"

static inline void _sendByte(const rs485_t *rs485, uint8_t data)
{
	rs485->sercom->USART.DATA.reg = data;
}

static inline void _clearTxFlag(const rs485_t *rs485)
{
	rs485->sercom->USART.INTFLAG.bit.DRE = true;
}

void rs485_init(const kernel_t *kernel, rs485_t *rs485, gclk_generator_t clkGenerator, Sercom *sercom, uint32_t baudrate, sercom_parity_t parity)
{
	rs485->sercom = sercom;
	sercom_initClock(clkGenerator,rs485->sercom);

	uint32_t ctra = 0x40130084;
	uint32_t ctrb = 0x00030000;
	
	//uint32_t ctra = 0x40120084;
	//uint32_t ctrb = 0x00030000;
	
	if(parity != sercom_parity_none) ctra |= 0x01000000;
	if(parity == sercom_parity_odd) ctrb |= 0x00002000;
	
	rs485->sercom->USART.CTRLA.reg = ctra;
	rs485->sercom->USART.CTRLB.reg = ctrb;
	
	rs485->sercom->USART.CTRLC.bit.GTIME = 7; // 7 bit guard time
	
	rs485->sercom->USART.INTENSET.bit.RXC = true;
	rs485->sercom->USART.INTENSET.bit.TXC = true;
	
	sercom_setBaudRate(rs485->sercom, 16000000, baudrate);
	rs485->sercom->USART.CTRLA.bit.ENABLE = true;
	
	rs485->state = rs485_state_idle;
	
	kernel->tickTimer.reset(&rs485->timeoutTimer);
}

void rs485_handler(const kernel_t *kernel, rs485_t *rs485)
{	
	if(rs485->state == rs485_state_received)
	{
		rs485->rxCompleted = true;		
		rs485->state = rs485_state_received_pause;
		kernel->tickTimer.reset(&rs485->pauseTimer);
	}
	
	if(rs485->state ==  rs485_state_received_pause && kernel->tickTimer.delay1ms(&rs485->pauseTimer, 150) )
	{
		rs485->state = rs485_state_idle;
	}
}

bool rs485_send(const kernel_t *kernel, rs485_t *rs485)
{
	if(rs485->state != rs485_state_idle) return false;
	
	rs485->rxCompleted = false;
	
	kernel->tickTimer.reset(&rs485->timeoutTimer);
	rs485->state = rs485_state_transmit;
	
	bytearray_clear(rs485->rx);
	
	_sendByte(rs485, rs485->tx->data[0]);
	rs485->txIndex = 1;
	
	return true;
}

bool rs485_busy(rs485_t *rs485)
{
	if(rs485->state == rs485_state_idle) return false;
	else return true;
}

void rs485_reset(rs485_t *rs485)
{
	rs485->state == rs485_state_idle;
}

bool rs485_rxCompleted(rs485_t *rs485)
{
	if(rs485->rxCompleted){
		rs485->rxCompleted = false;
		return true;
	}
	return false;
}

void rs485_rxInterrupt(rs485_t *rs485)
{	
	uint8_t data = rs485->sercom->USART.DATA.reg;
	
	if(rs485->state == rs485_state_receive)
	{
		bytearray_pushByte(rs485->rx, data);
		
		if(rs485->rxSize == rs485->rx->size){
			rs485->state = rs485_state_received;
		}
	}
	
	rs485->sercom->USART.INTFLAG.bit.RXC = true;
}

void rs485_txInterrupt(rs485_t *rs485)
{
	if(rs485->txIndex < rs485->tx->size)
	{
		_sendByte(rs485, rs485->tx->data[rs485->txIndex]);
		rs485->txIndex++;
	}
	else
	{
		rs485->state = rs485_state_receive;
		rs485->sercom->USART.INTFLAG.bit.DRE = true;
	}
}
