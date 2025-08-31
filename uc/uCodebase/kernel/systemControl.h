//**********************************************************************************************************************
// FileName : systemControl.h
// FilePath : kernel/
// Author   : Christian Marty
// Date		: 02.08.2025
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#ifndef SYSTEM_CONTROL_H_
#define SYSTEM_CONTROL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "common/typedef.h"

#define MAX_STRING_SIZE 60
typedef struct {
	uint8_t  deviceAddress;
	uint8_t  systemSavedSettings;
	uint16_t heartbeatInterval;
	uint16_t extendedHeartbeatInterval;
	
	uint8_t deviceName[MAX_STRING_SIZE];
	uint8_t deviceNameLength;

	uint8_t administratorAccessKey[MAX_STRING_SIZE];
	uint8_t administratorAccessKeyLength;
	
	uint8_t reserved0[128];
	
	uint8_t appData[256];
} eememData_t;


typedef struct {
	uint8_t applicationRunOnStartup : 1;
	uint8_t userLedEnabled : 1;
	uint8_t messageLogEnabled : 1;
} sysSavedSettingsBit_t;

typedef union {
	sysSavedSettingsBit_t bit;
	uint8_t reg;
}sysSavedSettings_t;

typedef struct {
    uint32_t applicationRuning : 1;
    uint32_t applicationCrcError : 1;
    uint32_t applicationRunOnStartup : 1;
    uint32_t userLedEnabled : 1;
    uint32_t identify : 1;
    uint32_t administratorAccess : 1;
    uint32_t messageLogEnabled : 1;
    uint32_t reserved0 : 1;

    uint32_t systemError : 1;
    uint32_t watchdogWarning : 1;
    uint32_t watchdogError : 1;
    uint32_t kernelTxBufferOverrun: 1;
	uint32_t txMessageOverrun: 1;
    uint32_t kernelRxBufferOverrun: 1;
    uint32_t applicationRxBufferOverrun: 1;
    uint32_t applicationError : 1;

	uint32_t applicationSpecific : 16;
} sysStatusDataBit_t;

typedef union {
	sysStatusDataBit_t bit;
	uint32_t reg;
}sysStatusData_t;

typedef struct {
    uint32_t applicationRun : 1;
    uint32_t applicationCheckCrc : 1;
    uint32_t applicationRunOnStartup : 1;
    uint32_t userLedEnabled : 1;
    uint32_t identify : 1;
    uint32_t reserved0 : 1;
    uint32_t messageLogEnabled : 1;
    uint32_t reserved1 : 1;

    uint32_t clearSystemError : 1;
    uint32_t clearWatchdogWarning : 1;
    uint32_t clearWatchdogError : 1;
    uint32_t clearkernelTxBufferOverrun : 1;
	uint32_t clearTxMessageOverrun: 1;
    uint32_t clearKernelRxBufferOverrun : 1;
    uint32_t clearApplicationRxBufferOverrun: 1;
    uint32_t clearApplicationError : 1;

    uint32_t applicationSpecific : 16;
} sysControlDataBit_t;

typedef union {
	sysControlDataBit_t bit;
	uint32_t reg;
}sysControlData_t;

typedef enum {
	APP_STOP, 
	APP_INIT, 
	APP_RUN, 
	APP_SHUTDOWN
} appState_t;

typedef struct{
	uint32_t us_max;
	uint32_t us_min;
	uint32_t us_avg;
	uint16_t us_last;
	
	uint16_t ms_last;
	uint16_t tick_last;
}app_benchmark_t;

typedef struct {
	sysControlData_t sysControl;
	sysStatusData_t sysStatus;
	
	sysControlData_t sysControlOld;
	sysStatusData_t sysStatusOld;
	
	appState_t appState;
	
	app_benchmark_t appBenchmark;
	
	uint32_t appCrc;
	
}systemControl_t;


void systemControl_initialize(void);
void systemControl_handler(void);
void systemControl_run(void);

bool systemControl_hasChanged(void);
sysSavedSettings_t systemControl_getSaveSetting(void);

void systemControl_setWatchdogWarning(void);
void systemControl_setWatchdogError(void);
void systemControl_setAppCrcError(bool error);
bool systemControl_getAppRun(void);

bool systemControl_appIsActive(void);

void benchmark_reset(app_benchmark_t *benchmark);
void benchmark_run(app_benchmark_t *benchmark);

void systemControl_flagApplicationError(void);

#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_CONTROL_H_ */