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

typedef struct {
	uint32_t ID  : 29;
	uint32_t RTR : 1;
	uint32_t XTD : 1;
	uint32_t ESI : 1;
	
	uint32_t RXTS :16;
	uint32_t DLC : 4;
	uint32_t BRS : 1;
	uint32_t FDF : 1;
	uint32_t reserved0 :2;
	uint32_t FIDX : 7;
	uint32_t ANMF : 1;
	
	uint8_t data[64];
} can_rxFrame_t;


#define CAN_TX_BUFFER_SIZE 32 // 32 is max buffer size
#define CAN_KERNEL_RX_BUFFER_SIZE 64 // 64 is max buffer size
#define CAN_APP_RX_BUFFER_SIZE 64 // 64 is max buffer size

can_txFrame_t txBuffer[CAN_TX_BUFFER_SIZE];
can_rxFrame_t kernelRxBuffer[CAN_KERNEL_RX_BUFFER_SIZE];
can_rxFrame_t appRxBuffer[CAN_APP_RX_BUFFER_SIZE];

#define CAN_NUMBER_OF_EXT_FILTER 18
can_extFilter_t extFilter[CAN_NUMBER_OF_EXT_FILTER];

static const uint8_t canDlcLut[16] = {0,1,2,3,4,5,6,7,8,12,16,20,24,32,48,64};

uint8_t bus_address;

void _bus_decodeMessage(bus_rxMessage_t *message, const can_rxFrame_t *frame);
uint8_t _bus_getRxDataSize(const can_rxFrame_t *frame);

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
	
	CAN0->RXF0C.bit.F0SA = (uint32_t) &kernelRxBuffer[0];
	CAN0->RXF0C.bit.F0S = CAN_KERNEL_RX_BUFFER_SIZE;
	
	CAN0->RXF1C.bit.F1SA = (uint32_t) &appRxBuffer[0];
	CAN0->RXF1C.bit.F1S = CAN_APP_RX_BUFFER_SIZE;
	
	CAN0->TXESC.bit.TBDS = 0x07; // 64Byte data Size
	//CAN0->TXBC.bit.NDTB = CAN_NUMBER_OF_TX_BUFFERS;
	CAN0->TXBC.bit.TFQS = CAN_TX_BUFFER_SIZE;
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
	
		
	// Kernel data filter
		for(uint8_t i = 0; i<8; i++){
			extFilter[i*2].EFEC = 0x1; // Store in FIFO 0
			extFilter[i*2].EFT = 0x02; // Classic Filter
			extFilter[i*2].EFID1 = (((uint32_t)bus_address)<<13) | (i<<7);// Filter
			extFilter[i*2].EFID2 = 0x000FFF80;// Mask
	
			extFilter[i*2+1].EFEC = 0x1; // Store in FIFO 0
			extFilter[i*2+1].EFT = 0x02; // Classic Filter
			extFilter[i*2+1].EFID1 = (BUS_BROADCAST_ADDRESS<<13) | (i<<7);// Filter
			extFilter[i*2+1].EFID2 = 0x000FFF80;// Mask
		}
	
	//	App data filter
		//extFilter[CAN_NUMBER_OF_EXT_FILTER-2].EFEC = 0x2; // Store in FIFO 1
		extFilter[CAN_NUMBER_OF_EXT_FILTER-2].EFT = 0x02; // Classic Filter
		extFilter[CAN_NUMBER_OF_EXT_FILTER-2].EFID1 =  (((uint32_t)bus_address)<<13)&0x000FE000;// Filter
		extFilter[CAN_NUMBER_OF_EXT_FILTER-2].EFID2 = 0x000FE000;// Mask
			
		//extFilter[CAN_NUMBER_OF_EXT_FILTER-1].EFEC = 0x2; // Store in FIFO 1
		extFilter[CAN_NUMBER_OF_EXT_FILTER-1].EFT = 0x02; // Classic Filter
		extFilter[CAN_NUMBER_OF_EXT_FILTER-1].EFID1 =  (BUS_BROADCAST_ADDRESS<<13)&0x000FE000;// Filter
		extFilter[CAN_NUMBER_OF_EXT_FILTER-1].EFID2 = 0x000FE000;// Mask	
		
		bus_enableAppMessageBuffer(false);
	}

	CAN0->CCCR.bit.FDOE = true;
	CAN0->CCCR.bit.CCE = false;
	CAN0->CCCR.bit.INIT = false;
	
	BusTransceiverInitialization();
	BusTransceiverEnable();
}

void bus_deinitialize(void)
{
	BusTransceiverDisable();
}

void bus_handler(void)
{
	if(CAN0->IR.bit.RF0L){
		sysControlHandler.sysStatus.bit.kernelRxBufferOverrun = true;
		CAN0->IR.bit.RF0L = true;
	}
	
	if(CAN0->IR.bit.RF1L){
		sysControlHandler.sysStatus.bit.applicationRxBufferOverrun = true;
		CAN0->IR.bit.RF1L = true;
	}
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

void bus_clearAppReceiveBuffer(void)
{
	for(uint8_t i = 0; i < CAN_APP_RX_BUFFER_SIZE; i++){
		if(CAN0->RXF1S.bit.F1FL == 0) break;
		CAN0->RXF1A.bit.F1AI = CAN0->RXF1S.bit.F1GI;
	}
}

void bus_enableAppMessageBuffer(bool enable)
{
	if(enable){
		extFilter[CAN_NUMBER_OF_EXT_FILTER-2].EFEC = 0x2; // Store in FIFO 1
		extFilter[CAN_NUMBER_OF_EXT_FILTER-1].EFEC = 0x2; // Store in FIFO 1
	}else{
		extFilter[CAN_NUMBER_OF_EXT_FILTER-2].EFEC = 0x3; // REJECT
		extFilter[CAN_NUMBER_OF_EXT_FILTER-1].EFEC = 0x3; // REJECT
	}
}

// --- Receive Code ----------------------------------------------------------------------------------------------------------

void _bus_decodeMessage(bus_rxMessage_t *message, const can_rxFrame_t *frame)
{
	message->sourceAddress = (frame->ID>>20)&0x7F;
	message->protocol = (busProtocol_t)((uint8_t)(frame->ID>>7)&0x3F);
	message->command = (frame->ID>>4)&0x07;
	message->length = _bus_getRxDataSize(frame);
	message->broadcast = (message->sourceAddress == BUS_BROADCAST_ADDRESS);
	
	for(uint8_t i = 0; i<message->length; i++){
		message->data[i] = frame->data[i];
	}
}

uint8_t _bus_getRxDataSize(const can_rxFrame_t *frame)
{
	uint8_t length = canDlcLut[frame->DLC];
	length -= (frame->ID & 0x0F);
	return length;
}

bool bus_getKernelMessage(bus_rxMessage_t *message)
{
	if(CAN0->RXF0S.bit.F0FL == 0) return false; // check if buffer is empty
	
	uint8_t bufferIndex = CAN0->RXF0S.bit.F0GI;
	const can_rxFrame_t *frame = &kernelRxBuffer[bufferIndex];
	
	_bus_decodeMessage(message, frame);
	
	CAN0->RXF0A.bit.F0AI = bufferIndex; // free buffer item
	
	return true;
}

bool bus_getAppMessage(bus_rxMessage_t *message)
{
	if(CAN0->RXF1S.bit.F1FL == 0) return false; // check if buffer is empty
		
	uint8_t bufferIndex = CAN0->RXF1S.bit.F1GI;
	const can_rxFrame_t *frame = &appRxBuffer[bufferIndex];
		
	_bus_decodeMessage(message, frame);
		
	CAN0->RXF1A.bit.F1AI = bufferIndex; // free buffer item
	
	return true;
}

// --- Transmit Code ---------------------------------------------------------------------------------------------------------

void bus_clearTxFrame(bus_txMessage_t *message)
{
	txBuffer[message->bufferIndex].DLC = 0;
	txBuffer[message->bufferIndex].ID = 0;
	
	for(uint8_t i = 0; i<64; i++){
		txBuffer[message->bufferIndex].data[i] = 0;
	}
}

bool bus_getMessageSlot(bus_txMessage_t *message)
{
	message->length = 0;
	message->bufferIndex = CAN0->TXFQS.bit.TFQPI;
	
	if(CAN0->TXFQS.bit.TFQF){ //TX FIFO/Queue full
		sysControlHandler.sysStatus.bit.kernelTxBufferOverrun = true;
		return false;
	}
	
	return true;
}

void bus_writeId(bus_txMessage_t *message, uint32_t canId)
{
	txBuffer[message->bufferIndex].ID = canId;	
}

void bus_writeHeader(bus_txMessage_t *message, uint8_t destAddr, busProtocol_t protocol, busCommand_t command, busPriority_t priority)
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

void bus_pushByte(bus_txMessage_t *message, uint8_t data)
{
	if(message->length >= 64){ // Don't allow to push more than CAN-FD max size
		sysControlHandler.sysStatus.bit.txMessageOverrun = true;
		return;
	}
	
	txBuffer[message->bufferIndex].data[message->length] = data;
	message->length++;
}

void bus_pushWord16(bus_txMessage_t *message, uint16_t data)
{
	bus_pushByte(message,(uint8_t)(data>>8));
	bus_pushByte(message,(uint8_t)(data));
}

void bus_pushWord24(bus_txMessage_t *message, uint32_t data)
{
	bus_pushByte(message,(uint8_t)(data>>16));
	bus_pushWord16(message,(uint16_t)(data));
}

void bus_pushWord32(bus_txMessage_t *message, uint32_t data)
{
	bus_pushWord16(message,(uint16_t)(data>>16));
	bus_pushWord16(message,(uint16_t)(data));
}

void bus_pushString(bus_txMessage_t *message, const char *data)
{
	const uint8_t *temp = (const uint8_t*)data;
	bus_pushArray(message, &temp[0], string_getLength(&data[0]));
}

void bus_pushArray(bus_txMessage_t *message, const uint8_t *data, uint8_t dataSize)
{
	for(uint8_t i= 0; i< dataSize; i++){
		bus_pushByte(message, data[i]);
	}
}

bool bus_send(bus_txMessage_t *message)
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

#ifdef __cplusplus
}
#endif