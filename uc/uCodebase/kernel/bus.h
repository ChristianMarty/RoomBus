//**********************************************************************************************************************
// FileName : bus.h
// FilePath : uCodebase/kernel/
// Author   : Christian Marty
// Date		: 11.09.2018
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#ifndef BUS_H_
#define BUS_H_

#ifdef __cplusplus
extern "C" {
#endif
	
#include "common/typedef.h"

void bus_initialize(uint8_t selfAddress, bool promiscuousMode);
void bus_deinitialize(void);

void bus_handler(void);

bool bus_getMessageSlot(bus_txMessage_t *message);
void bus_writeId(bus_txMessage_t *message, uint32_t canId);
void bus_writeHeader(bus_txMessage_t *message, uint8_t destAddr, busProtocol_t protocol, busCommand_t command, busPriority_t priority);
bool bus_send(bus_txMessage_t *message);
void bus_pushByte(bus_txMessage_t *message, uint8_t data);
void bus_pushWord16(bus_txMessage_t *message, uint16_t data);
void bus_pushWord24(bus_txMessage_t *message, uint32_t data);
void bus_pushWord32(bus_txMessage_t *message, uint32_t data);
void bus_pushArray(bus_txMessage_t  *message, const uint8_t *data, uint8_t dataSize);
void bus_pushString(bus_txMessage_t *message, const char *data);

void bus_clearTxFrame(bus_txMessage_t *message);

void bus_clearAppReceiveBuffer(void);
void bus_enableAppMessageBuffer(bool enable);

bool bus_getKernelMessage(bus_rxMessage_t *message);
bool bus_getAppMessage(bus_rxMessage_t *message);

uint32_t bus_getErrorCounter(void);
uint8_t bus_getLastErrorCode(void);

void bus_onTransmitt(void);

#ifdef __cplusplus
}
#endif

#endif /* BUS_H_ */