//**********************************************************************************************************************
// FileName : messageLogProtocol.c
// FilePath : protocol/
// Author   : Christian Marty
// Date		: 09.06.2024
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#include "messageLogProtocol.h"
#include "kernel/bus.h"
#include "driver/SAMx5x/tickTimer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	mlp_cmdSysMessage,
	mlp_cmdSysWarning,
	mlp_cmdSysError,
	mlp_cmdSysReserved,
	
	mlp_cmdAppMessage,
	mlp_cmdAppWarning,
	mlp_cmdAppError,
	mlp_cmdAppReserved
} mlp_command_t;

void mlp_sysMessage(const char *message)
{
	bus_message_t msg;
	if(!bus_getMessageSlot(&msg)) return;

	bus_writeHeader(&msg, BUS_BROADCAST_ADDRESS, busProtocol_logging, mlp_cmdSysMessage, busPriority_low);
	bus_pushWord32(&msg, tickTimer_getTickTime());
	bus_pushWord32(&msg, 0);
	bus_pushString(&msg, &message[0]);
	bus_send(&msg);
}

void mlp_sysWarning(const char *message)
{
	bus_message_t msg;
	if(!bus_getMessageSlot(&msg)) return;

	bus_writeHeader(&msg, BUS_BROADCAST_ADDRESS, busProtocol_logging, mlp_cmdSysWarning, busPriority_low);
	bus_pushWord32(&msg, tickTimer_getTickTime());
	bus_pushWord32(&msg, 0);
	bus_pushString(&msg, &message[0]);
	bus_send(&msg);
}

void mlp_sysError(const char *message)
{
	bus_message_t msg;
	if(!bus_getMessageSlot(&msg)) return;

	bus_writeHeader(&msg, BUS_BROADCAST_ADDRESS, busProtocol_logging, mlp_cmdSysError, busPriority_low);
	bus_pushWord32(&msg, tickTimer_getTickTime());
	bus_pushWord32(&msg, 0);
	bus_pushString(&msg, &message[0]);
	bus_send(&msg);	
}

void mlp_appMessage(const char *message)
{
	bus_message_t msg;
	if(!bus_getMessageSlot(&msg)) return;

	bus_writeHeader(&msg, BUS_BROADCAST_ADDRESS, busProtocol_logging, mlp_cmdAppMessage, busPriority_low);
	bus_pushWord32(&msg, tickTimer_getTickTime());
	bus_pushWord32(&msg, 0);
	bus_pushString(&msg, &message[0]);
	bus_send(&msg);
}

void mlp_appWarning(const char *message)
{
	bus_message_t msg;
	if(!bus_getMessageSlot(&msg)) return;

	bus_writeHeader(&msg, BUS_BROADCAST_ADDRESS, busProtocol_logging, mlp_cmdAppWarning, busPriority_low);
	bus_pushWord32(&msg, tickTimer_getTickTime());
	bus_pushWord32(&msg, 0);
	bus_pushString(&msg, &message[0]);
	bus_send(&msg);
}

void mlp_appError(const char *message)
{
	bus_message_t msg;
	if(!bus_getMessageSlot(&msg)) return;

	bus_writeHeader(&msg, BUS_BROADCAST_ADDRESS, busProtocol_logging, mlp_cmdAppError, busPriority_low);
	bus_pushWord32(&msg, tickTimer_getTickTime());
	bus_pushWord32(&msg, 0);
	bus_pushString(&msg, &message[0]);
	bus_send(&msg);
}

#ifdef __cplusplus
}
#endif