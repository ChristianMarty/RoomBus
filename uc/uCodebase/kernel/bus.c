//**********************************************************************************************************************
// FileName : bus.c
// FilePath : uCodebase/kernel/
// Author   : Christian Marty
// Date		: 11.09.2018
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#ifdef __cplusplus
extern "C" {
#endif

#include "bus.h"
#include "main.h"

#include "utility/string.h"
#include "kernel/systemControl.h"

#include "driver/SAMx5x/kernel/interrupt.h"

extern systemControl_t sysControlHandler;

typedef struct{
	uint32_t EFID1 : 29;
	uint32_t EFEC : 3;
	
	uint32_t EFID2 : 29;
	uint32_t reserved : 1;
	uint32_t EFT : 2;
} can_extFilter_t;

typedef struct{
	uint32_t ID  : 29;
	uint32_t RTR : 1;
	uint32_t XTD : 1;
	uint32_t ESI : 1;
	
	uint32_t reserved0 :16;
	uint32_t DLC : 4;
	uint32_t BRS : 1;
	uint32_t FDF : 1;
	uint32_t reserved1 :1;
	uint32_t EFG : 1;
	uint32_t MM  : 7;
	
	uint8_t data[64];
} can_txFrame_t;


#define CAN_NUMBER_OF_TX_BUFFERS 32 // 32 is max buffer size
#define CAN_NUMBER_OF_FIFO0 64 // 64 is max buffer size
#define CAN_NUMBER_OF_EXT_FILTER 2

can_txFrame_t txBuffer[CAN_NUMBER_OF_TX_BUFFERS];
can_rxFrame_t rxFrame[CAN_NUMBER_OF_FIFO0];
can_extFilter_t extFilter[CAN_NUMBER_OF_EXT_FILTER];

static const uint8_t canDlcLut[16] = {0,1,2,3,4,5,6,7,8,12,16,20,24,32,48,64};

uint8_t bus_address;

void bus_initialize(uint8_t selfAddress, bool promiscuousMode)
{
	bus_address = selfAddress;
	
	CAN0->CCCR.bit.INIT = true;
	CAN0->CCCR.bit.CCE = true;
	
	CAN0->CCCR.bit.TXP = true;
	CAN0->CCCR.bit.FDOE = true;
	
	CAN0->NBTP.bit.NBRP = 1; // Set 500kbaud
	CAN0->NBTP.bit.NSJW = 3;
	CAN0->NBTP.bit.NTSEG1 = 10;
	CAN0->NBTP.bit.NTSEG2 = 3;	
	
	//CAN0->CCCR.bit.DAR = true; // disable auto retransmitt
	
	CAN0->RXESC.bit.RBDS = 0x07; // 64Byte data Size
	CAN0->RXESC.bit.F0DS = 0x07; // 64Byte data Size
	CAN0->RXESC.bit.F1DS = 0x07; // 64Byte data Size
	CAN0->RXF0C.bit.F0SA = (uint32_t) &rxFrame[0];
	CAN0->RXF0C.bit.F0S = CAN_NUMBER_OF_FIFO0;
	
	CAN0->TXESC.bit.TBDS = 0x07; // 64Byte data Size
	//CAN0->TXBC.bit.NDTB = CAN_NUMBER_OF_TX_BUFFERS;
	CAN0->TXBC.bit.TFQS = CAN_NUMBER_OF_TX_BUFFERS;
	CAN0->TXBC.bit.TBSA = (uint32_t) &txBuffer[0];	
	//CAN0->TXBC.bit.TFQM = true; // Tx Queue operation
	CAN0->TXBC.bit.TFQM = false; // Tx FIFO operation
	

	if(!promiscuousMode){
		CAN0->GFC.bit.RRFS = 1; // Reject Remote Frames Standard
		CAN0->GFC.bit.RRFE = 1; // Reject Remote Frames Extended
	
		CAN0->GFC.bit.ANFS = 0x02; // Reject Non-matching  11-Bit Ids
		CAN0->GFC.bit.ANFE = 0x02; // Reject Non-matching  29-Bit Ids
	
		CAN0->XIDFC.bit.FLESA = (uint32_t) &extFilter[0];
		CAN0->XIDFC.bit.LSE = CAN_NUMBER_OF_EXT_FILTER;
	
		extFilter[0].EFEC = 0x1; // Store in FIFO 0
		extFilter[0].EFT = 0x02; // Classic Filter
		extFilter[0].EFID1 =  (((uint32_t)bus_address)<<13)&0x000FE000;// Filter
		extFilter[0].EFID2 = 0x000FE000;// Mask
	
		extFilter[1].EFEC = 0x1; // Store in FIFO 0
		extFilter[1].EFT = 0x02; // Classic Filter
		extFilter[1].EFID1 =  (BUS_BROADCAST_ADDRESS<<13)&0x000FE000;// Filter
		extFilter[1].EFID2 = 0x000FE000;// Mask
	}

	CAN0->CCCR.bit.FDOE = true;
	CAN0->CCCR.bit.CCE = false;
	CAN0->CCCR.bit.INIT = false;
	
	BusTransceiverInitialization();
	BusTransceiverEnable();
	
	nvic_assignInterruptHandler(CAN0_IRQn, bus_interrupt);
}

void bus_deinitialize(void)
{
	BusTransceiverDisable();
}

uint32_t bus_getErrorCounter(void)
{
	return CAN0->ECR.reg;
}

uint8_t bus_getLastErrorCode(void)
{
	uint8_t temp = CAN0->PSR.bit.DLEC;
	temp = (temp<<4);
	return (temp | CAN0->PSR.bit.LEC);
}

bool bus_rxBufferEmpty(void)
{
	if(CAN0->RXF0S.bit.F0FL != 0) return false;
	else return true;
}

uint8_t bus_getRxFrameIndex(void)
{
	return CAN0->RXF0S.bit.F0GI;
}

uint8_t bus_getRxDataSize(can_rxFrame_t *frame)
{
	uint8_t length = canDlcLut[frame->DLC];
	length -= (frame->ID & 0x0F);
	return length;
}

can_rxFrame_t *bus_getRxFrame(uint8_t rxFrameIndex)
{
	return &rxFrame[rxFrameIndex];
}

uint8_t bus_getSourceAddress(can_rxFrame_t *frame)
{
	return (frame->ID>>20)&0x7F;
}

uint8_t bus_getDestinationAddress(can_rxFrame_t *frame)
{
	return (frame->ID>>13)&0x7F;
}

busCommand_t bus_getCommand(can_rxFrame_t *frame)
{
	return (frame->ID>>4)&0x07;		
}

busProtocol_t bus_getProtocol(can_rxFrame_t *frame)
{
	return (busProtocol_t)((uint8_t)(frame->ID>>7)&0x3F);
}

void bus_freeRxFrame(uint8_t rxFrameIndex)
{
	CAN0->RXF0A.bit.F0AI = rxFrameIndex;
}

void bus_clearTxFrame(uint8_t txBufferIndex)
{
	txBuffer[txBufferIndex].DLC = 0;
	txBuffer[txBufferIndex].ID = 0;
	
	for(uint8_t i = 0; i<64; i++){
		txBuffer[txBufferIndex].data[i] = 0;
	}
}

bool bus_getMessageSlot(bus_message_t *message)
{
	message->length = 0;
	message->bufferIndex = CAN0->TXFQS.bit.TFQPI;
	
	if(CAN0->TXFQS.bit.TFQF){ //TX FIFO/Queue full
		sysControlHandler.sysStatus.bit.txBufferOverrun = true;
		return false;
	}
	
	return true;
}

void bus_writeId(bus_message_t *message, uint32_t canId)
{
	txBuffer[message->bufferIndex].ID = canId;	
}

void bus_writeHeader(bus_message_t *message, uint8_t destAddr, busProtocol_t protocol, busCommand_t command, busPriority_t priority)
{
	uint32_t temp = 0;
	destAddr &= 0x7F;
	command &= 0x07;
	protocol &= 0x3F;
	priority &= 0x03;
	
	temp |= ((uint32_t)((uint8_t)priority) << 27);	
	temp |= ((uint32_t)(bus_address) << 20);
	temp |= ((uint32_t)destAddr << 13);
	temp |= ((uint32_t)((uint8_t)protocol) << 7);
	temp |= ((uint32_t)command << 4);
	
	txBuffer[message->bufferIndex].ID = temp;	
}

void bus_pushByte(bus_message_t *message, uint8_t data)
{
	if(message->length >= 64){ // Don't allow to push more than CAN-FD max size
		sysControlHandler.sysStatus.bit.txMessageOverrun = true;
		return;
	}
	
	txBuffer[message->bufferIndex].data[message->length] = data;
	message->length++;
}

void bus_pushWord16(bus_message_t *message, uint16_t data)
{
	bus_pushByte(message,(uint8_t)(data>>8));
	bus_pushByte(message,(uint8_t)(data));
}

void bus_pushWord24(bus_message_t *message, uint32_t data)
{
	bus_pushByte(message,(uint8_t)(data>>16));
	bus_pushWord16(message,(uint16_t)(data));
}

void bus_pushWord32(bus_message_t *message, uint32_t data)
{
	bus_pushWord16(message,(uint16_t)(data>>16));
	bus_pushWord16(message,(uint16_t)(data));
}

void bus_pushString(bus_message_t *message, const char *data)
{
	const uint8_t *temp = (const uint8_t*)data;
	bus_pushArray(message, &temp[0], string_getLength(&data[0]));
}

void bus_pushArray(bus_message_t *message, const uint8_t *data, uint8_t dataSize)
{
	for(uint8_t i= 0; i< dataSize; i++){
		bus_pushByte(message, data[i]);
	}
}

bool bus_send(bus_message_t *message)
{
	uint32_t temp = txBuffer[message->bufferIndex].ID; 
	uint8_t i = 0;
		
	for(i = 0; i< 16; i++){
		if(canDlcLut[i] >= message->length) break;
	}
	
	uint8_t stuffSize = canDlcLut[i] - message->length;
	
	txBuffer[message->bufferIndex].DLC = i;
	temp |= (canDlcLut[i] - message->length);
	
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