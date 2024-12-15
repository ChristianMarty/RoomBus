/*
 * app.cpp
 *
 * Created: 20.04.2019 20:05:30
 * Author : Christian
 */ 


#include "sam.h"
#include "kernel.h"
#include "busController_IO.h"

#include "uart.h"
#include "pin.h"
#include "softCRC.h"
#include "string.h"

#include "valueReportProtocol.h"

#include "dmx.h"

#include "dma.h"

int main(const kernel_t *kernel);
void onReceive(const kernel_t *kernel, uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size);
kernel_t const *_kernel;

__attribute__((section(".appHeader"))) appHead_t appHead ={
/*appCRC	 */ 0xAABBCCDD, // Will be written by Bootload tool
/*appSize	 */ 0xEEFF0000, // Will be written by Bootload tool
/*appRevMaj	 */ 0x01,
/*appRevMin	 */ 0x00,
/*appName[60]*/ "DMX",
/*main		 */ main,
/*onRx		 */ onReceive
};


dmx_c dmx;
uint8_t txBuffer[514];


uint8_t ememData[256];
void onReceive(const kernel_t *kernel, uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size)
{	

}

dma_transfer_t *trns;

uint32_t timer;
uint8_t step;
int main(const kernel_t *kernel)
{
	_kernel = kernel;
	
	// App init Code
	if(kernel->kernelSignals->initApp)
	{
		Reset_Handler();
		
		dmx.init(kernel->clk_16MHz, SERCOM5);
		
		// Uart
		pin_enablePeripheralMux(IO_D12, PIN_FUNCTION_C);
		pin_enablePeripheralMux(IO_D13, PIN_FUNCTION_C);
		
		//kernel->assignInterruptHandler(DMAC_0_IRQn, transfer);
			
		/*kernel->eemem_read(&ememData[0]);*/
		
		kernel->appSignals->appReady = true;
		
		for(uint16_t i = 1; i<100;i++)
		{
			txBuffer[i+1] = i;
			
		}
		txBuffer[0] = 0;
		txBuffer[1] = 0;
		txBuffer[2] = 200;
		txBuffer[3] = 200;
		txBuffer[4] = 200;
		txBuffer[5] = 200;
		txBuffer[6] = 200;
		
		step = 1;
		
		dma_init();
		
		trns = dma_getTransfer(DMA_CH0);
	}

	// Main code here
	if(kernel->appSignals->appReady == true)
	{
		
		if(kernel->tickTimer_delay1ms(&timer,25))
		{
			dmx.sendBrak();
			dma_transfer_dataToPeripheral(trns, &txBuffer[0],514,&SERCOM5->USART.DATA.reg);
			dma_channel_init(DMA_CH0,DMA_SERCOM5_TX, DMA_BURST_TRIGGER);
			
			txBuffer[step+1]++;
			if(txBuffer[step+1] >= 255)
			{
				txBuffer[step+1] = 0;
				 step++;
			}
			if(step >4) step = 1;
		}
		
	}
	
	// App deinit code
    if(kernel->kernelSignals->shutdownApp)
    {
		//kernel->eemem_write(&ememData[0]);
		kernel->appSignals->shutdownReady = true;
    }
}