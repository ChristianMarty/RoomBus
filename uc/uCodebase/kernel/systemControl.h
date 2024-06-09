/*
 * systemControl.h
 *
 * Created: 14.07.2019 12:11:06
 *  Author: Christian
 */ 

#ifndef SYSTEMCONTROL_H_
#define SYSTEMCONTROL_H_

#include "sam.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint8_t appRunOnStartup : 1;
	uint8_t ledOnOff : 1;
} sysSavedSettingsBit_t;

typedef union {
	sysSavedSettingsBit_t bit;
	uint8_t reg;
}sysSavedSettings_t;

typedef struct {
	uint32_t appRun : 1;
	uint32_t appCheckCrc : 1;
	uint32_t appRunOnStartup : 1;
	uint32_t ledOnOff : 1;
	uint32_t identify : 1;
	uint32_t reserved0 : 1;
	uint32_t remoteDebuggerEnable : 1;
	uint32_t clearError : 1;
	
	uint32_t rebootApp : 1;
	uint32_t rebootSystem : 1;
	uint32_t reserved1 : 6;
	
	uint32_t appSpecific : 16;
} sysControlDataBit_t;

typedef union {
	sysControlDataBit_t bit;
	uint32_t reg;
}sysControlData_t;

typedef struct {
	uint32_t appRuning : 1;
	uint32_t appCrcError : 1;
	uint32_t appRunOnStartup : 1;
	uint32_t ledOnOff : 1;
	uint32_t identify : 1;
	uint32_t setupModeEn : 1;
	uint32_t remoteDebugger : 1;
	uint32_t systemError : 1;
	
	uint32_t watchdogWarning : 1;
	uint32_t watchdogError : 1;
	uint32_t reserved1 : 7;
	
	uint32_t appSpecific : 16;
} sysStatusDataBit_t;

typedef union {
	sysStatusDataBit_t bit;
	uint32_t reg;
}sysStatusData_t;

typedef struct {
	sysControlData_t *sysControl;
	sysStatusData_t *sysStatus;
	
	sysControlData_t sysControlOld;
	sysStatusData_t sysStatusOld;
}systemControl_t;

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


void systemControl_init(systemControl_t *self, sysSavedSettings_t *settings);
void systemControl_run(systemControl_t *self);

bool systemControl_hasChanged(systemControl_t *self);
sysSavedSettings_t systemControl_getSaveStting(systemControl_t *self);

void systemControl_interveneStartup(systemControl_t *self);

void systemControl_setWatchdogWarning(systemControl_t *self);
void systemControl_setWatchdogError(systemControl_t *self);
void systemControl_setAppCrcError(systemControl_t *self, bool error);
bool systemControl_getAppRun(systemControl_t *self);

void benchmark_reset(app_benchmark_t *benchmark);
void benchmark_run(app_benchmark_t *benchmark);

#ifdef __cplusplus
}
#endif


#endif /* SYSTEMCONTROL_H_ */