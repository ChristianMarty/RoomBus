/*
 * bus.h
 *
 * Created: 11.09.2018 22:34:27
 *  Author: Christian
 */ 

#ifndef BUS_H_
#define BUS_H_

#include "common/typedef.h"


#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

	
typedef struct  
{
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

typedef struct
{
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


typedef struct
{	
	uint32_t EFID1 : 29;
	uint32_t EFEC : 3;
	
	uint32_t EFID2 : 29;
	uint32_t reserved : 1;
	uint32_t EFT : 2;
} can_extFilter_t;


void bus_transceiverEnable(bool state);
void bus_init(uint8_t selfAddress);

bool bus_getMessageSlot(bus_message_t *message);
void bus_writeId(bus_message_t *message, uint32_t canId);
void bus_writeHeader(bus_message_t *message, uint8_t destAddr, busProtocol_t protocol, uint8_t command, busPriority_t priority);
bool bus_send(bus_message_t *message);
void bus_pushByte(bus_message_t *message, uint8_t data);
void bus_pushWord16(bus_message_t *message, uint16_t data);
void bus_pushWord24(bus_message_t *message, uint32_t data);
void bus_pushWord32(bus_message_t *message, uint32_t data);
void bus_pushArray(bus_message_t *message, const uint8_t *data, uint8_t dataSize);
void bus_pushString(bus_message_t *message, const char *data);

void bus_clearTxFrame(uint8_t txBufferIndex);

bool bus_rxBufferEmpty(void);
uint8_t bus_getRxFrameIndex(void);
can_rxFrame_t *bus_getRxFrame(uint8_t rxBufferIndex);
uint8_t bus_getRxDataSize(can_rxFrame_t *frame);
uint8_t bus_getSourceAddress(can_rxFrame_t *frame);
uint8_t bus_getDestinationAddress(can_rxFrame_t *frame);
uint8_t bus_getCommand(can_rxFrame_t *frame);
busProtocol_t bus_getProtocol(can_rxFrame_t *frame);
void bus_freeRxFrame(uint8_t rxBufferIndex);

uint32_t bus_getErrorCounter(void);
uint8_t bus_getLastErrorCode(void);

void bus_interrupt(void);
void bus_onTransmitt(void);


#ifdef __cplusplus
}
#endif

#endif /* BUS_H_ */