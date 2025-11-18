

#include "lightBusModul.h"
#include "interface/lightBus.h"

#include "driver/SAMx5x/uart.h"
#include "driver/SAMx5x/pin.h"
#include "utility/cobs.hpp"
#include "utility/softCRC.h"

#include "common/kernel.h"


lightBus_t lightBus_1;
lightBus_t lightBus_2;

lightBusModul_t *_lightBusModul = 0;

void uart_5_onRx(void)
{
	lightBus_onRx(&lightBus_1);
}

void uart_5_onTx(void)
{
	lightBus_onTx(&lightBus_1);
}

void uart_1_onRx(void)
{
	lightBus_onRx(&lightBus_2);
}

void uart_1_onTx(void)
{
	lightBus_onTx(&lightBus_2);
}

void lightBus_1_onReceive(const uint8_t *data, uint8_t size, bool error)
{
	if(_lightBusModul != 0) _lightBusModul->lightBusModul_onReceive(lightBusModul_channel_t::lightBusModul_channel_1, data, size, error);
}

void lightBus_2_onReceive(const uint8_t *data, uint8_t size, bool error)
{
	if(_lightBusModul != 0) _lightBusModul->lightBusModul_onReceive(lightBusModul_channel_t::lightBusModul_channel_2, data, size, error);
}

uart_c uart;

uint8_t uart_rxBuffer[1024];
uint16_t uart_rxBuffer_index;
uint16_t uart_rxSize = false;

cobs_decodeStream_t cobsDecodeStream;

void uart_rxInterrupt(void)
{
	uint8_t temp = uart.RxInterrupt();
	uart_rxSize = cobs_decodeStream(&cobsDecodeStream, temp, &uart_rxBuffer[0]); //, sizeof(uart_rxBuffer)
	
}


uint16_t lightBusModul_uart_dataAvailable(lightBusModul_t *lightBusModul)
{
	return uart_rxSize;
}

uint8_t *lightBusModul_uart_data(lightBusModul_t *lightBusModul)
{
	return uart_rxBuffer;
}

void lightBusModul_uart_dataClear(lightBusModul_t *lightBusModul)
{
	uart_rxSize = 0;
}

void lightBusModul_send(lightBusModul_channel_t channel, const uint8_t *data, uint8_t size)
{
	switch(channel){
		case lightBusModul_channel_1: lightBus_send(&lightBus_1, data, size); break;
		case lightBusModul_channel_2: lightBus_send(&lightBus_2, data, size); break;
	}
}


void lightBusModul_set(lightBusModul_channel_t channel, uint8_t address, lightBus_fade_t fade0, uint16_t ch0, lightBus_fade_t fade1, uint16_t ch1, lightBus_fade_t fade2, uint16_t ch2, lightBus_fade_t fade3, uint16_t ch3)
{
	switch(channel){
		case lightBusModul_channel_1: lightBus_set(&lightBus_1, address, fade0, ch0, fade1, ch1, fade2, ch2, fade3, ch3); break;
		case lightBusModul_channel_2: lightBus_set(&lightBus_2, address, fade0, ch0, fade1, ch1, fade2, ch2, fade3, ch3); break;
	}	
}

void lightBusModul_execute(lightBusModul_channel_t channel, uint8_t address)
{
	switch(channel){
		case lightBusModul_channel_1: lightBus_execute(&lightBus_1, address); break;
		case lightBusModul_channel_2: lightBus_execute(&lightBus_2, address); break;
	}
}


dma_transfer_t *dma_dmxTx;
dmx_c dmx;
uint32_t dmx_timer;

void lightBusModul_init(lightBusModul_t *lightBusModul)
{
		_lightBusModul = lightBusModul;
		
		if(lightBusModul->dmx_enable)
		{
			// Init DMX
			dmx.init(kernel.clk_16MHz, SERCOM2);
			pin_enablePeripheralMux(IO_D04, PIN_FUNCTION_C);
			pin_enablePeripheralMux(IO_D05, PIN_FUNCTION_C);
			
			// Enable Transaiver Power
			pin_enableOutput(IO_D07);
			pin_setOutput(IO_D07, true);
			
			// Enable Transmitter
			pin_enableOutput(IO_D06);
			pin_setOutput(IO_D06, true);
			
			dma_initialize();
			dma_dmxTx = dma_getTransfer(DMA_CH00);
			
			kernel.tickTimer.reset(&dmx_timer);
		
		}
			
		// Init Light 1 BUS
		lightBus_init(&lightBus_1, SERCOM5);
		pin_enablePeripheralMux(IO_D12, PIN_FUNCTION_C);
		pin_enablePeripheralMux(IO_D13, PIN_FUNCTION_C);
		kernel.nvic.assignInterruptHandler(SERCOM5_2_IRQn,uart_5_onRx);
		kernel.nvic.assignInterruptHandler(SERCOM5_1_IRQn,uart_5_onTx);
		
		lightBus_1.onReceive = lightBus_1_onReceive;
		
		// Init Light 2 BUS
		lightBus_init(&lightBus_2, SERCOM1);
		pin_enablePeripheralMux(IO_D08, PIN_FUNCTION_C);
		pin_enablePeripheralMux(IO_D09, PIN_FUNCTION_C);
		kernel.nvic.assignInterruptHandler(SERCOM1_2_IRQn,uart_1_onRx);
		kernel.nvic.assignInterruptHandler(SERCOM1_1_IRQn,uart_1_onTx);
		
		lightBus_2.onReceive = lightBus_2_onReceive;
			
		// Init UART Interface 
		uart.initUart(kernel.clk_16MHz, SERCOM4, 500000,uart_c::none);
		pin_enablePeripheralMux(IO_D00, PIN_FUNCTION_C);
		pin_enablePeripheralMux(IO_D01, PIN_FUNCTION_C);
			
		kernel.nvic.assignInterruptHandler(SERCOM4_2_IRQn, uart_rxInterrupt);
		

}


void lightBusModul_handler(lightBusModul_t *lightBusModul)
{
	lightBus_handler(&lightBus_1);
	lightBus_handler(&lightBus_2);
	
	if(lightBusModul->dmx_enable)
	{
		// DMX Update
		if(kernel.tickTimer.delay1ms(&dmx_timer,25))
		{
			lightBusModul->dmx_data[0] = 0x00; // Break
			lightBusModul->dmx_data[1] = 0x00;
			
			dmx.sendBreak();
			dma_transfer_dataToPeripheral(dma_dmxTx, &lightBusModul->dmx_data[0],sizeof(lightBusModul->dmx_data_size),&SERCOM2->USART.DATA.reg);
			dma_channel_initialize(DMA_CH00, DMA_SERCOM2_TX, DMA_BURST_TRIGGER);
		}
	}
}