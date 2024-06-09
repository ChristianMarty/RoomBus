                      

#include "messageProtocol.h"
#include "bus.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SYS_MESSAGE_CMD 0x00
#define SYS_WARNING_CMD 0x01
#define SYS_ERROR_CMD 0x02

#define APP_MESSAGE_CMD 0x04
#define APP_WARNING_CMD 0x05
#define APP_ERROR_CMD 0x06

void mlp_sysMessage(const char *message)
{
	bus_message_t msg;
	
	bus_getMessageSlot(&msg);
	bus_writeHeader(&msg,BUS_BROADCAST_ADDRESS, busProtocol_logging, SYS_MESSAGE_CMD, busPriority_low);
	bus_pushByte(&msg,0);
	bus_pushString(&msg, &message[0]);
	bus_send(&msg);
}

void mlp_sysWarning(const char *message)
{
	bus_message_t msg;
	
	bus_getMessageSlot(&msg);
	bus_writeHeader(&msg,BUS_BROADCAST_ADDRESS, busProtocol_logging, SYS_WARNING_CMD, busPriority_low);
	bus_pushByte(&msg,0);
	bus_pushString(&msg, &message[0]);
	bus_send(&msg);
}

void mlp_sysError(const char *message)
{
	bus_message_t msg;

	bus_getMessageSlot(&msg);
	bus_writeHeader(&msg,BUS_BROADCAST_ADDRESS, busProtocol_logging, SYS_ERROR_CMD, busPriority_low);
	bus_pushByte(&msg,0);
	bus_pushString(&msg, &message[0]);
	bus_send(&msg);	
}

void mlp_appMessage(const char *message)
{
	bus_message_t msg;
	
	bus_getMessageSlot(&msg);
	bus_writeHeader(&msg,BUS_BROADCAST_ADDRESS, busProtocol_logging, APP_MESSAGE_CMD, busPriority_low);
	bus_pushByte(&msg,0);
	bus_pushString(&msg, &message[0]);
	bus_send(&msg);
}

void mlp_appWarning(const char *message)
{
	bus_message_t msg;
	
	bus_getMessageSlot(&msg);
	bus_writeHeader(&msg,BUS_BROADCAST_ADDRESS, busProtocol_logging, APP_WARNING_CMD, busPriority_low);
	bus_pushByte(&msg,0);
	bus_pushString(&msg, &message[0]);
	bus_send(&msg);
}

void mlp_appError(const char *message)
{
	bus_message_t msg;
	
	bus_getMessageSlot(&msg);
	bus_writeHeader(&msg,BUS_BROADCAST_ADDRESS, busProtocol_logging, APP_ERROR_CMD, busPriority_low);
	bus_pushByte(&msg,0);
	bus_pushString(&msg, &message[0]);
	bus_send(&msg);
}

#ifdef __cplusplus
}
#endif