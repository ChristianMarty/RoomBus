//**********************************************************************************************************************
// FileName : deviceManagementProtocol.h
// FilePath : protocol/
// Author   : Christian Marty
// Date		: 09.06.2024
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#ifndef DEVICE_MANAGEMENT_PROTOCOL_H_
#define DEVICE_MANAGEMENT_PROTOCOL_H_

#include "sam.h"
#include "common/kernel.h"

#include "kernel/systemControl.h"
#include "kernel/bus.h"


#include "driver/SAMx5x/system.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	dmp_cmdHeartbeat,
	dmp_cmdSystemInfo,
	dmp_cmdHardwareName,
	dmp_cmdApplicationName,
	dmp_cmdDeviceName,
	dmp_cmdHeartbeatRequest,
	dmp_cmdSystemInfoRequest,
	dmp_cmdHeartbeatSettings,
	dmp_cmdWriteControl,
	dmp_cmdSetControl,
	dmp_cmdClearControl,
	dmp_cmdEnterRootMode,
	dmp_cmdSetDeviceName,
	dmp_cmdSetAddress,
		
	dmp_canDiagnosticsRequest = 0xF0,
	dmp_canDiagnosticsReport = 0xF1,
	
	dmp_echo = 0xFA,
	dmp_cmdReboot = 0xFB,
		
	dmp_cmdEraseApp = 0xFC,
	dmp_cmdEraseAppRsp = 0xFD,
	dmp_cmdBtl = 0xFE,
	dmp_cmdBtlRsp = 0xFF
}dmp_command_t;



void dmp_init(uint8_t *appName, sysControlData_t *sysControl, sysStatusData_t *sysStatus, appState_t *appState, app_benchmark_t *appBenchmark);	
uint8_t dmp_getDeviceAddress(void);
	
void dmp_handler(void);
void dmp_receiveHandler(uint8_t sourceAddress, uint8_t command, uint8_t *data, uint8_t size);
	
void dmp_sendHeartbeat(void);
void dmp_sendExtendedHeartbeat(void);
void dmp_sendSystemInfo(void);
bool dmp_setupMode(void);
	
void dmp_setAppCrc(uint32_t crc);
uint32_t dmp_getAppCrc(void);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_MANAGEMENT_PROTOCOL_H_ */