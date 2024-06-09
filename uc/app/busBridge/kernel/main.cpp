/*
 * kernel.cpp
 *
 * Created: 19.04.2019 18:52:57
 * Author : Christian
 */ 


#include "sam.h"
#include "kernel/standardIO.h"
#include "drv/SAMx5x/sysTickTimer.h"
#include "drv/SAMx5x/interrupt.h"
#include "drv/SAMx5x/system.h"
#include "drv/SAMx5x/pin.h"
#include "drv/SAMx5x/uart.h"
#include "drv/SAMx5x/genericClockController.h"
#include "kernel/bus.h"
#include "kernel/kernel.h"
#include "utility/cobs.h"
#include "utility/softCRC.h"

uint32_t ledTimer;
uint32_t greenLedTimer;
uint32_t redLedTimer;
bool led = false;

uart_c rs232;
uint8_t rs232RxData[100];
uint8_t rs232RxCounter = 0;

#define BUS_UNFILTERED

void bus_onTransmitt(void)
{
	
}

void uart_onRx(void)
{
	uint8_t data = rs232.RxInterrupt();
	
	if(data == 0)
	{
		if(rs232RxCounter > 2)
		{
			cobsDecode(&rs232RxData[0],&rs232RxData[1],rs232RxCounter);
			
			if(crc16(&rs232RxData[0], rs232RxCounter-1) == 0)
			{
				sysTick_resetDelayCounter(&redLedTimer);
				stdIO_redLED(true);
				
				uint32_t temp = 0;
				
				temp |= ((uint32_t)rs232RxData[0]<<24);
				temp |= ((uint32_t)rs232RxData[1]<<16);
				temp |= ((uint32_t)rs232RxData[2]<<8);
				temp |= ((uint32_t)rs232RxData[3]);
				
				bus_message_t rs232toCanMsg;
				bus_getMessageSlot(&rs232toCanMsg);
				
				bus_writeId(&rs232toCanMsg,temp);
				bus_pushArray(&rs232toCanMsg, &rs232RxData[4], rs232RxCounter-7);
				bus_send(&rs232toCanMsg);
				
			}
		}
		rs232RxCounter = 0;
	}
	else
	{
		rs232RxData[rs232RxCounter+1] = data;
		rs232RxCounter ++;
	}
}

void uart_onTx(void)
{
	rs232.TxInterrupt();
}

gclk_generator_t clk_16Mhz;

uint8_t rxData[100];

int main(void)
{
    /* Initialize the SAM system */
	system_init();
	sysTick_init(120000000);
	stdIO_init();
	
	nvic_init();
	nvic_assignInterruptHandler(SysTick_IRQn, sysTick_interruptHandler);
	nvic_assignInterruptHandler(CAN0_IRQn,bus_interrupt);
	
	clk_16Mhz = gclk_getFreeGenerator();
	gclk_generatorEnable(clk_16Mhz,GCLK_SRC_XOSC1,1);

	rs232.initUart(clk_16Mhz,SERCOM2,614400,uart_c::none);
	
	// Uart
	pin_enablePeripheralMux(PIN_PORT_A,12,PIN_FUNCTION_C);
	pin_enablePeripheralMux(PIN_PORT_A,13,PIN_FUNCTION_C);
	nvic_assignInterruptHandler(SERCOM2_2_IRQn,uart_onRx);
	nvic_assignInterruptHandler(SERCOM2_1_IRQn,uart_onTx);
	
	//Can
	gclk_peripheralEnable(GCLK_CAN0,clk_16Mhz);
	pin_enableOutput(PIN_PORT_B,9);
	pin_enablePeripheralMux(PIN_PORT_A,22,PIN_FUNCTION_I);
	pin_enablePeripheralMux(PIN_PORT_A,23,PIN_FUNCTION_I);
	
	bus_init(0);

	stdIO_redLED(led);
	
    while (1) 
    {	
	
		if(sysTick_delay1ms(&redLedTimer,10))
		{
			stdIO_redLED(false);
		}
		
		if(sysTick_delay1ms(&greenLedTimer,10))
		{
			stdIO_greenLED(false);
		}
		
		if(!bus_rxBufferEmpty())
		{
			if(!rs232.txBusy())
			{
				sysTick_resetDelayCounter(&greenLedTimer);
				stdIO_greenLED(true);
			
				uint8_t rxIndex = bus_getRxFrameIndex();
				can_rxFrame_t *rx = bus_getRxFrame(rxIndex);
				
				for(uint8_t i = 0; i< sizeof(rxData); i++)
				{
					rxData[i] = 0;
				}
				
				rxData[0] = 0;
				rxData[1] = 0;
			
				rxData[2] = (uint8_t)((rx->ID>>24) & 0x1F);
				rxData[3] = (uint8_t)((rx->ID>>16) & 0xFF);
				rxData[4] = (uint8_t)((rx->ID>>8) & 0xFF);
				rxData[5] = (uint8_t)((rx->ID) & 0xFF);
				
				uint8_t i = 0;
				
				for (i = 0; i<bus_getRxDataSize(rx); i++)
				{
					rxData[6+i] = rx->data[i];
				}
			
				uint16_t crc = crc16(&rxData[2], i+4);
				rxData[6+i] = (crc>>8);
				rxData[7+i] = (crc&0xFF);
			
				cobsEncode(&rxData[1], &rxData[2], 5+i);
				
				rs232.sendData(&rxData[0], 9+i); 

				bus_freeRxFrame(rxIndex);
			}	
		}
    }
}
