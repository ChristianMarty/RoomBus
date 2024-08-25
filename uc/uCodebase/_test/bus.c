#include "bus.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TX_BUFFER_SIZE 32
bus_message_t txBuffer[TX_BUFFER_SIZE];
uint8_t  txIndex = 0;

void bus_init()
{
    txIndex = 0;
}

bus_message_t* bus_lastSend(void)
{
    return &txBuffer[txIndex];
}

bool bus_getMessageSlot(bus_message_t *message)
{
    return true;
}

void bus_writeHeader(bus_message_t *message, uint8_t destAddr, busProtocol_t protocol, uint8_t command, busPriority_t priority)
{
    message->destinationAddress = destAddr,
    message->protocol = protocol;
    message->command = command;
    message->priority = priority;
    message->length = 0;
    for(uint8_t i = 0; i<64; i++){
        message->data[i] = 0;
    }
}

bool bus_send(bus_message_t *message)
{
    txBuffer[txIndex] = *message;
    txIndex++;
    return true;
}

void bus_pushByte(bus_message_t *message, uint8_t data)
{
    if(message->length >=64) return; // Don't allow to push more than CAN-FD max size. TODO:  Add error handling

    message->data[message->length] = data;
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

void bus_pushArray(bus_message_t *message, const uint8_t *data, uint8_t dataSize)
{
    for(uint8_t i= 0; i< dataSize; i++)
    {
        bus_pushByte(message,data[i]);
    }
}

void bus_pushString(bus_message_t *message, const char *data)
{
    for(uint8_t i= 0; i< 64; i++)
    {
        uint8_t character = data[i];
        if(character == NULL) break;
        bus_pushByte(message,character);
    }
}

#ifdef __cplusplus
}
#endif
