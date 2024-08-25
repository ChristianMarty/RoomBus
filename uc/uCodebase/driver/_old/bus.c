/*
 * bus.c
 *
 * Created: 11.09.2018 22:34:27
 *  Author: Christian
 */ 

#include "bus.h"
#include "string.h"


#ifdef __cplusplus
extern "C" {
#endif

#define CAN_NUMBER_OF_TX_BUFFERS 32
#define CAN_NUMBER_OF_FIFO0 64
#define CAN_NUMBER_OF_EXT_FILTER 1

can_txFrame_t txBuffer[CAN_NUMBER_OF_TX_BUFFERS];

can_rxFrame_t rxFrame[CAN_NUMBER_OF_FIFO0];

can_extFilter_t extFilter[CAN_NUMBER_OF_EXT_FILTER];



uint8_t *bus_address;

void bus_transceiverEnable(bool state)
{
	if(state) PORT->Group[1].OUTCLR.reg= PORT_PB09;
	else  PORT->Group[1].OUTSET.reg = PORT_PB09;
}

void bus_init(uint8_t *selfAddress)
{
	CAN0->CCCR.bit.INIT = true;
	CAN0->CCCR.bit.CCE = true;
	
	CAN0->CCCR.bit.TXP = true;
	CAN0->CCCR.bit.FDOE = true;
	
	//CAN0->CCCR.bit.DAR = true; // disable auto retransmitt
	
	CAN0->RXESC.bit.RBDS = 0x07; // 64Byte data Size
	CAN0->RXESC.bit.F0DS = 0x07; // 64Byte data Size
	CAN0->RXESC.bit.F1DS = 0x07; // 64Byte data Size
	CAN0->RXF0C.bit.F0SA = &rxFrame[0];
	CAN0->RXF0C.bit.F0S = CAN_NUMBER_OF_FIFO0;
	
	CAN0->TXESC.bit.TBDS = 0x07; // 64Byte data Size
	//CAN0->TXBC.bit.NDTB = CAN_NUMBER_OF_TX_BUFFERS;
	CAN0->TXBC.bit.TFQS = CAN_NUMBER_OF_TX_BUFFERS;
	CAN0->TXBC.bit.TBSA = &txBuffer[0];	
	//CAN0->TXBC.bit.TFQM = true; // Tx Queue operation
	CAN0->TXBC.bit.TFQM = false; // Tx FIFO operation
	
	//CAN0->XIDFC.bit.FLESA = &extFilter[0];
	//CAN0->XIDFC.bit.LSE = CAN_NUMBER_OF_EXT_FILTER;
	
	CAN0->CCCR.bit.FDOE = true;
	CAN0->CCCR.bit.CCE = false;
	CAN0->CCCR.bit.INIT = false;
	bus_transceiverEnable(true);	
	
	
	extFilter[0].EFEC = 0x1; // Store in FIFO 0
	extFilter[0].EFT = 0x02; // Classic Filter
	extFilter[0].EFID1 = 0;// Filter
	extFilter[0].EFID2 = 0;// Mask
	
	bus_address = selfAddress;
}

bool bus_rxBufferEmpty(void)
{
	if(CAN0->RXF0S.bit.F0PI != CAN0->RXF0S.bit.F0GI) return false;
	else return true;
}

uint8_t bus_getRxFrameIndex(void)
{
	return CAN0->RXF0S.bit.F0GI;
}

uint8_t bus_getRxDataSize(uint8_t rxFrameIndex)
{
	uint8_t length = CAN_DLC_LUT[rxFrame[rxFrameIndex].DLC];
	length -= (rxFrame[rxFrameIndex].ID & 0x0F);
	return length;
}

can_rxFrame_t *bus_getRxFrame(uint8_t rxFrameIndex)
{
	return &rxFrame[rxFrameIndex];
}

void bus_freeRxFrame(uint8_t rxFrameIndex)
{
	CAN0->RXF0A.bit.F0AI = rxFrameIndex;
}

void bus_clearTxFrame(uint8_t txBufferIndex)
{
	txBuffer[txBufferIndex].DLC = 0;
	txBuffer[txBufferIndex].ID = 0;
	
	for(uint8_t i = 0; i<64; i++)
	{
		txBuffer[txBufferIndex].data[i] = 0;
	}
	
}

bool bus_getMessageSlot(bus_message_t *message)
{
	message->length = 0;
	message->bufferIndex = CAN0->TXFQS.bit.TFQPI;
	return true; 
}

uint8_t *bus_getDataAddress(bus_message_t *message)
{
	return &txBuffer[message->bufferIndex];
}

void bus_writeId(bus_message_t *message, uint32_t canId)
{
	txBuffer[message->bufferIndex].ID = canId;	
}

void bus_writeHeader(bus_message_t *message, uint8_t destAddr, busProtocol_t protocol, uint8_t command, busPriority_t priority)
{
	uint32_t temp = 0;
	destAddr &= 0x7F;
	command &= 0x07;
	protocol &= 0x3F;
	priority &= 0x03;
	
	temp |= ((uint32_t)((uint8_t)priority) << 27);	
	temp |= ((uint32_t)(*bus_address) << 20);
	temp |= ((uint32_t)destAddr << 13);
	temp |= ((uint32_t)((uint8_t)protocol) << 7);
	temp |= ((uint32_t)command << 4);
	
	txBuffer[message->bufferIndex].ID = temp;	
}

void bus_pushByte(bus_message_t *message, uint8_t data)
{
	txBuffer[message->bufferIndex].data[message->length] = data;
	message->length++;
}

void bus_pushWord16(bus_message_t *message, uint16_t data)
{
	bus_pushByte(message,(uint8_t)(data>>8));
	bus_pushByte(message,(uint8_t)(data));
}

void bus_pushWord32(bus_message_t *message, uint32_t data)
{
	bus_pushWord16(message,(uint16_t)(data>>16));
	bus_pushWord16(message,(uint16_t)(data));
}

void bus_pushString(bus_message_t *message, const char *data)
{
	uint8_t *temp = data;
	bus_pushArray(message,&temp[0],string_getLength(&data[0]));
}

void bus_pushArray(bus_message_t *message, uint8_t *data, uint8_t dataSize)
{
	for(uint8_t i= 0; i< dataSize; i++)
	{
		bus_pushByte(message,data[i]);
	}
}

bool bus_send(bus_message_t *message)
{
	if(message->bufferIndex > CAN_NUMBER_OF_TX_BUFFERS) return false;
	
	uint32_t temp = txBuffer[message->bufferIndex].ID; 
	uint8_t i = 0;
	
	for(i = 0; i< 16; i++)
	{
		if(CAN_DLC_LUT[i] >= message->length) break;
	}
	
	uint8_t stuffSize = CAN_DLC_LUT[i] - message->length;
	
	txBuffer[message->bufferIndex].DLC = i;
	temp |= (CAN_DLC_LUT[i] - message->length);
	
	for(i = 0; i< stuffSize; i++)
	{
		bus_pushByte(message, 0xAA); // Use 0xAA to fill frame
	}
	
	txBuffer[message->bufferIndex].FDF = true;
	txBuffer[message->bufferIndex].XTD = true;
	txBuffer[message->bufferIndex].ID = temp;
	
	CAN0->TXBAR.reg |= (1<<message->bufferIndex);	
	bus_onTransmitt();
	return true;
}

void bus_interrupt(void)
{
	
}

#ifdef __cplusplus
}
#endif