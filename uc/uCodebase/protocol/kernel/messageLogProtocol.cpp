//**********************************************************************************************************************
// FileName : messageLogProtocol.c
// FilePath : protocol/kernel/
// Author   : Christian Marty
// Date		: 02.08.2025
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#ifdef __cplusplus
extern "C" {
#endif

#include "messageLogProtocol.h"

#include "driver/SAMx5x/kernel/tickTimer.h"
#include "kernel/bus.h"
#include "kernel/systemControl.h"

extern systemControl_t sysControlHandler;

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

inline void mlp_pushLogMessage(bus_txMessage_t *busMessage, busCommand_t command, const char *message)
{
	bus_writeHeader(busMessage, BUS_BROADCAST_ADDRESS, busProtocol_logging, command, busPriority_low);
	bus_pushWord32(busMessage, tickTimer_getTickTime());
	bus_pushWord32(busMessage, 0); //reserved
	bus_pushString(busMessage, message);
}

void mlp_sysMessage(const char *message)
{
	if(!sysControlHandler.sysStatus.bit.messageLogEnabled) return;
	
	bus_txMessage_t msg;
	if(!bus_getMessageSlot(&msg)) return;
	mlp_pushLogMessage(&msg, mlp_cmdSysMessage, message);
	bus_send(&msg);
}

void mlp_sysWarning(const char *message)
{
	if(!sysControlHandler.sysStatus.bit.messageLogEnabled) return;
	
	bus_txMessage_t msg;
	if(!bus_getMessageSlot(&msg)) return;
	mlp_pushLogMessage(&msg, mlp_cmdSysWarning, message);
	bus_send(&msg);
}

void mlp_sysError(const char *message)
{
	if(!sysControlHandler.sysStatus.bit.messageLogEnabled) return;
	
	bus_txMessage_t msg;
	if(!bus_getMessageSlot(&msg)) return;
	mlp_pushLogMessage(&msg, mlp_cmdSysError, message);
	bus_send(&msg);	
}

void mlp_appMessage(const char *message)
{
	if(!sysControlHandler.sysStatus.bit.messageLogEnabled) return;
	
	bus_txMessage_t msg;
	if(!bus_getMessageSlot(&msg)) return;
	mlp_pushLogMessage(&msg, mlp_cmdAppMessage, message);
	bus_send(&msg);
}

void mlp_appWarning(const char *message)
{
	if(!sysControlHandler.sysStatus.bit.messageLogEnabled) return;
	
	bus_txMessage_t msg;
	if(!bus_getMessageSlot(&msg)) return;
	mlp_pushLogMessage(&msg, mlp_cmdAppWarning, message);
	bus_send(&msg);
}

void mlp_appError(const char *message)
{
	if(!sysControlHandler.sysStatus.bit.messageLogEnabled) return;
	
	bus_txMessage_t msg;
	if(!bus_getMessageSlot(&msg)) return;
	mlp_pushLogMessage(&msg, mlp_cmdAppError, message);
	bus_send(&msg);
}

#ifdef __cplusplus
}
#endif