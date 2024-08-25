/*
 * uart.h
 *
 * Created: 11.09.2018 22:34:27
 *  Author: Christian
 */ 


#ifndef UART_H_
#define UART_H_

#include "sam.h"
#include "genericClockController.h"
#include "utility/bytearray.h"

class uart_c
{
	public:
		enum sercomParity_t{none,even,odd};
		
		uart_c();
		void initUart(gclk_generator_t clkGenerator, Sercom *sercom_p, uint32_t baudrate, sercomParity_t parity); 
		void setBaudRate(uint32_t baud, uint32_t clockFrequency); 
		void sendString(uint8_t *string);
		void sendData(uint8_t *data, uint8_t size);
		void sendBytearray(bytearray_t *data);
		
		bool txBusy(void);
		
		uint8_t RxInterrupt(void);
		void TxInterrupt(void);
		void writeByte(uint8_t data);
		
		void clearTxFlag(void);
		
	private:
		Sercom *_sercom_p;
		
		volatile uint8_t *txData;
		volatile uint8_t _txSize;
		volatile uint8_t _txIndex;
		volatile bool _txBusy;
};



#endif /* UART_H_ */