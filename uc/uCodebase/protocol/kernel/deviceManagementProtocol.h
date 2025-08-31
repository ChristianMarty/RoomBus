//**********************************************************************************************************************
// FileName : deviceManagementProtocol.h
// FilePath : protocol/kernel/
// Author   : Christian Marty
// Date		: 02.08.2025
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#ifndef DEVICE_MANAGEMENT_PROTOCOL_H_
#define DEVICE_MANAGEMENT_PROTOCOL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "kernel/systemControl.h"
#include "kernel/bus.h"

#include "common/typedef.h"

typedef enum {
	dmp_cmdHeartbeat,
	dmp_cmdSystemInfo,
	dmp_cmdHardwareName,
	dmp_cmdApplicationName,
	dmp_cmdDeviceName,
	dmp_cmdHeartbeatRequest,
	dmp_cmdSystemInformationRequest,
	dmp_cmdHeartbeatSettings, //*
	
	dmp_cmdWriteControl,
	dmp_cmdSetControl,
	dmp_cmdClearControl,
	dmp_cmdEnteradministratorAccess,
	dmp_cmdExitadministratorAccess,
	dmp_cmdSetDeviceName, //*
	dmp_cmdSetAddress, //*
	dmp_cmdSetadministratorAccessKey, //*
	
	dmp_canDiagnosticsRequest = 0xF0,
	dmp_canDiagnosticsReport = 0xF1,
	
	dmp_eepromReadRequest = 0xF8, //*
	dmp_eepromReadReport = 0xF9,
	
	dmp_echo = 0xFA,
	dmp_cmdReboot = 0xFB, //*
	dmp_cmdEraseApp = 0xFC, //*
	dmp_cmdEraseAppRsp = 0xFD, //*
	dmp_cmdBtl = 0xFE, //*
	dmp_cmdBtlRsp = 0xFF //*
} dmp_command_t;

//* Administration mode commands

void dmp_initialize(uint8_t *appName, systemControl_t *sysControl);	
void dmp_handler(void);
bool dmp_receiveHandler(const bus_rxMessage_t *rxMessage);

uint8_t dmp_getDeviceAddress(void);

void dmp_sendHeartbeat(void);
void dmp_sendExtendedHeartbeat(void);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_MANAGEMENT_PROTOCOL_H_ */