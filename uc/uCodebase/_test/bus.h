#ifndef BSU_TEST_H_
#define BSU_TEST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "common/typedef.h"

bool bus_getMessageSlot(bus_message_t *message);
void bus_writeHeader(bus_message_t *message, uint8_t destAddr, busProtocol_t protocol, uint8_t command, busPriority_t priority);
bool bus_send(bus_message_t *message);
void bus_pushByte(bus_message_t *message, uint8_t data);
void bus_pushWord16(bus_message_t *message, uint16_t data);
void bus_pushWord24(bus_message_t *message, uint32_t data);
void bus_pushWord32(bus_message_t *message, uint32_t data);
void bus_pushArray(bus_message_t *message, const uint8_t *data, uint8_t dataSize);
void bus_pushString(bus_message_t *message, const char *data);
void bus_init();
bus_message_t* bus_lastSend(void);

#ifdef __cplusplus
}
#endif

#endif /* BSU_TEST_H_ */



