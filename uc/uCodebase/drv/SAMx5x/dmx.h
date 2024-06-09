/*
 * dmx.h
 *
 * Created: 11.09.2018 22:34:27
 *  Author: Christian
 */ 


#ifndef DMX_H_
#define DMX_H_

#include "sam.h"
#include "genericClockController.h"

typedef struct{
	
} dmx_t;


class dmx_c
{
	public:
		
		dmx_c();
		void init(gclk_generator_t clkGenerator, Sercom *sercom_p);
		void setBaudRate(uint32_t baud, uint32_t clockFrequency); 
		
		void sendBreak(void);
		
	/*	void sendString(uint8_t *string);
		void sendData(uint8_t *data, uint8_t size);
		bool txBusy(void);
		
		uint8_t RxInterrupt(void);
		void TxInterrupt(void);
		void writeByte(uint8_t data);
		
		void clearTxFlag(void);*/
	private:
		
		
		Sercom *_sercom_p;
		uint8_t _samplesPerBit;
};



#endif /* DMX_H_ */