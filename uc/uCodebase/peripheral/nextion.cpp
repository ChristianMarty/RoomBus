

#include "nextion.h"
#include "driver/SAMx5x/pin.h"
#include "driver/SAMx5x/uart.h"

uart_c uartCom;
uint8_t txBuffer[32];

uint8_t rxBuffer[32];
uint8_t rxCounter;

uint8_t ffCounter;

void rxParse(const kernel_t *kernel, uint8_t *data, uint8_t size);

void uart_onRx(void)
{
	uint8_t data = uartCom.RxInterrupt();
	
	rxBuffer[rxCounter] = data;
	rxCounter++;
	
	if(data != 0xFF) ffCounter = 0;
	else ffCounter ++;
	
	if(rxCounter > sizeof(rxBuffer)) // If buffer overflow -> reset rx buffer
	{
		rxCounter = 0;
		ffCounter = 0;
	}
}

void uart_onTx(void)
{
	uartCom.TxInterrupt();
}

void nextion_handler(const kernel_t *kernel)
{
	if(ffCounter == 3)
	{
		rxParse(kernel, &rxBuffer[0],rxCounter-3);
		nextion_resetCom();
	}
}

void rxParse(const kernel_t *kernel, uint8_t *data, uint8_t size)
{
	if(data[0] == 0x65 && data[3] == 0x01)
	{
		nextion_onTouchEvent_callback(kernel, data[1],data[2]);
	}
}

void nextion_resetCom(void)
{
	ffCounter = 0;
	rxCounter = 0;	
}

void nextion_init(const kernel_t *kernel)
{
	uartCom.initUart(kernel->clk_16MHz, SERCOM5, 115200, uart_c::none);
	
	// Uart
	pin_enablePeripheralMux(PIN_PORT_B,16,PIN_FUNCTION_C);
	pin_enablePeripheralMux(PIN_PORT_B,17,PIN_FUNCTION_C);
	kernel->nvic.assignInterruptHandler(SERCOM5_2_IRQn,uart_onRx);
	kernel->nvic.assignInterruptHandler(SERCOM5_1_IRQn,uart_onTx);
}

void nextion_sendCommand(const kernel_t *kernel, uint8_t *command)
{
	uint8_t i = 0;
	while(command[i] != 0)
	{
		txBuffer[i] = command[i];
		i++;
		if(i >= sizeof(txBuffer)-3) break;
	}
	txBuffer[i] = 0xFF;
	i++;
	txBuffer[i] = 0xFF;
	i++;
	txBuffer[i] = 0xFF;
	i++;
	
	while(uartCom.txBusy());
	uartCom.sendData(&txBuffer[0],i);
}	
