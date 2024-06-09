
#include "LightBus.h"
#include "drv/SAMx5x/uart.h"
#include "drv/SAMx5x/pin.h"
#include "utility/cobs.h"
#include "utility/softCRC.h"

uart_c uartCom;

void uart_onRx(void);
void uart_onTx(void);

uint32_t com_timer;

uint8_t rxBuffer[100];
uint8_t rxCount = 0;
uint8_t rxByte = 0;


void uart_onRx(void)
{
	rxByte = uartCom.RxInterrupt();
	
	rxBuffer[rxCount] = rxByte;
	rxCount ++;
	
}

void uart_onTx(void)
{
	uartCom.TxInterrupt();
}


void lightBus_init(const kernel_t *kernel)
{
	
	uartCom.initUart(kernel->clk_16MHz,SERCOM5, 14400, uart_c::odd);
	
	// Uart
	pin_enablePeripheralMux(IO_D12, PIN_FUNCTION_C);
	pin_enablePeripheralMux(IO_D13, PIN_FUNCTION_C);
	kernel->nvic.assignInterruptHandler(SERCOM5_2_IRQn,uart_onRx);
	kernel->nvic.assignInterruptHandler(SERCOM5_1_IRQn,uart_onTx);
	
	kernel->tickTimer.reset(&com_timer);
	
}

uint8_t txBuffer[100];
uint8_t txCounter= 0; 




void lightBus_handler(const kernel_t *kernel)
{
	if(kernel->tickTimer.delay1ms(&com_timer, 1000))
	{
		if(rxByte == 0x55)
		{
			rxCount = 0;
		}
	}

}

void lightBus_send(const kernel_t *kernel, const uint8_t *data, uint8_t size)
{
	uint8_t temp[50];
	for(uint8_t i = 0; i< size; i++)
	{
		temp[i] = data[i];
	}
	
	uint16_t crc = crc16(&temp[0],size);
	
	temp[size] = (crc>>8)&0xFF;
	temp[size+1] = crc&0xFF;
	
	txBuffer[0] = 0;
	size = cobsEncode(&txBuffer[1],&temp[0],size+1);
 	size++;
	
	for(uint8_t i = 0; i< size; i++)
	{
		if(txBuffer[i] == 0x00) txBuffer[i] = 0x55;
		else if(txBuffer[i] == 0x55) txBuffer[i] = 0x00;
	}
	
	uartCom.sendData(&txBuffer[0],size);
}

#define LIGHTBUS_CMD_SET_ALL 0x04
#define LIGHTBUS_CMD_EXECUTE 0x05

void lightBus_set(const kernel_t *kernel, uint8_t address, lightBus_fade_t fade0, uint16_t ch0, lightBus_fade_t fade1, uint16_t ch1, lightBus_fade_t fade2, uint16_t ch2, lightBus_fade_t fade3, uint16_t ch3)
{
	uint8_t data[13];
	data[0] = (address<<4) | LIGHTBUS_CMD_SET_ALL;
	data[1] = *((uint8_t*)(&fade0));
	data[2] = (ch0>>8)&0xFF;
	data[3] = ch0&0xFF;
	
	data[4] = *((uint8_t*)(&fade1));
	data[5] = (ch1>>8)&0xFF;
	data[6] = ch1&0xFF;
	
	data[7] = *((uint8_t*)(&fade2));
	data[8] = (ch2>>8)&0xFF;
	data[9] = ch2&0xFF;
	
	data[10] = *((uint8_t*)(&fade3));
	data[11] = (ch3>>8)&0xFF;
	data[12] = ch3&0xFF;
	
	lightBus_send(kernel,data,sizeof(data));
}

void lightBus_execute(const kernel_t *kernel, uint8_t address)
{
	uint8_t data[1];
	data[0] = (address<<4) | LIGHTBUS_CMD_EXECUTE;
	
	lightBus_send(kernel,data,sizeof(data));
}