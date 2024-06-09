
#include "lightBus.h"
#include "drv/SAMx5x/pin.h"
#include "utility/softCRC.h"


uint8_t rxBuffer[100];
volatile uint8_t rxSize = 0;

uint8_t txBuffer[100];
uint8_t txCounter= 0;


void lightBus_onRx(lightBus_t *lightBus)
{
	uint8_t rxByte = lightBus->uartCom.RxInterrupt();
	
	//if(lightBus->state != lightBus_state_transmit)
	//{
		if(rxByte == 0x00) rxByte = 0x55;
		else if(rxByte== 0x55) rxByte = 0x00;
		
		rxSize = cobs_decodeStream(&lightBus->cobsDecodeStream, rxByte, &rxBuffer[0]);// , sizeof(rxBuffer)
	//}
}

void lightBus_onTx(lightBus_t *lightBus)
{
	lightBus->uartCom.TxInterrupt();
}


void lightBus_init(const kernel_t *kernel, lightBus_t *lightBus, Sercom *sercom_p)
{
	lightBus->uartCom.initUart(kernel->clk_16MHz, sercom_p, 14400, uart_c::odd);	
	kernel->tickTimer.reset(&lightBus->com_timer);	
	
	cobs_decodeStreamStart(&lightBus->cobsDecodeStream);
}

void lightBus_handler(const kernel_t *kernel, lightBus_t *lightBus)
{
	if(rxSize > 0)
	{
		uint16_t crc = crc16(&rxBuffer[0],rxSize);
		
		if(crc == 0) lightBus->onReceive(&rxBuffer[0],rxSize-2, false);
		else lightBus->onReceive(&rxBuffer[0],rxSize-2, true);
		
		rxSize = 0;
	}
	
	//if(lightBus->state == lightBus_state_transmit && !lightBus->uartCom.txBusy()) lightBus->state = lightBus_state_receive;
	
	if(kernel->tickTimer.delay1ms(&lightBus->com_timer, 100))
	{
		
	}

}

bool lightBus_send(const kernel_t *kernel, lightBus_t *lightBus, const uint8_t *data, uint8_t size)
{
	//lightBus->state = lightBus_state_transmit;
	if(lightBus->uartCom.txBusy()) return 0; 
	
	uint8_t temp[50];
	for(uint8_t i = 0; i< size; i++)
	{
		temp[i] = data[i];
	}
	
	uint16_t crc = crc16(&temp[0],size);
	
	temp[size] = (crc>>8)&0xFF;
	size ++;
	temp[size] = crc&0xFF;
	size ++;
	
	txBuffer[0] = 0;
	size = cobs_encode(&txBuffer[1],&temp[0],size);
 	size ++;
	
	for(uint8_t i = 0; i< size; i++)
	{
		if(txBuffer[i] == 0x00) txBuffer[i] = 0x55;
		else if(txBuffer[i] == 0x55) txBuffer[i] = 0x00;
	}
	
	lightBus->uartCom.sendData(&txBuffer[0],size);
	
	return 1;
}

#define LIGHTBUS_CMD_SET_ALL 0x04
#define LIGHTBUS_CMD_EXECUTE 0x05

void lightBus_set(const kernel_t *kernel, lightBus_t *lightBus, uint8_t address, lightBus_fade_t fade0, uint16_t ch0, lightBus_fade_t fade1, uint16_t ch1, lightBus_fade_t fade2, uint16_t ch2, lightBus_fade_t fade3, uint16_t ch3)
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
	
	lightBus_send(kernel,lightBus,data,sizeof(data));
}

void lightBus_execute(const kernel_t *kernel, lightBus_t *lightBus, uint8_t address)
{
	uint8_t data[1];
	data[0] = (address<<4) | LIGHTBUS_CMD_EXECUTE;

	lightBus_send(kernel,lightBus,data,sizeof(data));
}