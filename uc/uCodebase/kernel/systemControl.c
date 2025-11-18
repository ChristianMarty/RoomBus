//**********************************************************************************************************************
// FileName : systemControl.c
// FilePath : kernel/
// Author   : Christian Marty
// Date		: 02.08.2025
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#include "systemControl.h"
#include "driver/SAMx5x/kernel/system.h"
#include "common/kernel.h"

#include "protocol/kernel/messageLogProtocol.h"
#include "protocol/kernel/deviceManagementProtocol.h"

#include "driver/SAMx5x/kernel/eeprom.h"
#include "driver/SAMx5x/kernel/tickTimer.h"

uint32_t sysControlTaskTick;

eememData_t eememData EEPROM = {
	.hardwareRevisionMajor = HARDWARE_VERSION_MAJOR,
	.hardwareRevisionMinor = HARDWARE_VERSION_MINOR,
	.deviceAddress = 0,
	.systemSavedSettings = 0,
	.heartbeatInterval = 10,
	.extendedHeartbeatInterval = 60,
	
	.deviceName = {"New Device"},
	.deviceNameLength = 10,
	
	.administratorAccessKey  = {0x12, 0x34},
	.administratorAccessKeyLength = 2,
	
	.reserved0 = {},
	.appData = {}
};


extern applicationHeader_t *appHead;
extern kernel_t kernel;

extern kernelSignals_t kernelSignals;
extern appSignals_t appSignals;

extern systemControl_t sysControlHandler;

extern uint8_t watchdogWarningConter;
extern volatile uint8_t eememSysSavedSettings;

uint32_t systemControl_appCRC(void)
{
	PAC->WRCTRL.reg = 0x00010021; // Unlock
	
	DSU->STATUSA.bit.DONE = 1;
	
	DSU->DATA.reg = 0xFFFFFFFF;
	DSU->ADDR.reg = (APP_START_ADDR+4); //(appHead->appSize);
	DSU->LENGTH.reg = appHead->appSize;
	
	DSU->CTRL.bit.CRC = true;
	
	while(DSU->STATUSA.bit.DONE == 0);
	
	return DSU->DATA.reg;
}

bool systemControl_checkAppValid(void)
{
	if((appHead->appSize == 0xEEFF0000)&&(appHead->appCRC == 0xAABBCCDD)){
		sysControlHandler.appCrc = 0xAABBCCDD;
		return true; // For application debug
	}
	
	if((appHead->appSize != 0xFFFFFFFF)&&(appHead->appCRC != 0xFFFFFFFF)) // If App is not empty
	{
		sysControlHandler.appCrc = systemControl_appCRC();
		if(appHead->appCRC == sysControlHandler.appCrc){
			return true;
		}
	}
	
	sysControlHandler.appCrc = 0xFFFFFFFF;
	return false;
}

void systemControl_initialize(void)
{
	sysControlHandler.sysControl.reg = 0;
	sysControlHandler.sysStatus.reg = 0;
	
	sysControlHandler.sysControlOld.reg = 0;
	sysControlHandler.sysStatusOld.reg = 0;

	sysSavedSettings_t settings;
	settings.reg = eeporm_readByte(&eememData.systemSavedSettings);
	
	sysControlHandler.sysControl.bit.applicationRunOnStartup = settings.bit.applicationRunOnStartup;
	sysControlHandler.sysControl.bit.userLedEnabled = settings.bit.userLedEnabled;
	sysControlHandler.sysControl.bit.messageLogEnabled = settings.bit.messageLogEnabled;
	
	sysControlHandler.sysStatus.bit.applicationRunOnStartup = sysControlHandler.sysControl.bit.applicationRunOnStartup;
	sysControlHandler.sysStatus.bit.userLedEnabled = sysControlHandler.sysControl.bit.userLedEnabled;
	sysControlHandler.sysStatus.bit.identify = sysControlHandler.sysControl.bit.identify;
	sysControlHandler.sysStatus.bit.messageLogEnabled = sysControlHandler.sysControl.bit.messageLogEnabled;
	sysControlHandler.sysStatus.bit.applicationCrcError = true;
	
	if(sysControlHandler.sysStatus.bit.applicationRunOnStartup){
		sysControlHandler.sysControl.bit.applicationRun = true;
	}
	
	tickTimer_reset(&sysControlTaskTick);
	systemControl_setAppCrcError(!systemControl_checkAppValid());	
}

void systemControl_handler(void)
{
	if(systemControl_hasChanged()){
		systemControl_run();
		systemControl_hasChanged(); // run again to not trigger twice after systemControl_run
		dmp_sendHeartbeat();
		
		eeprom_writeByte(systemControl_getSaveSetting().reg, &eememData.systemSavedSettings);
	}
	
	// 500ms Task
	if(tickTimer_delay1ms(&sysControlTaskTick, 500)){
		if(watchdogWarningConter > 0){
			watchdogWarningConter--;
		}
		
		systemControl_setAppCrcError(!systemControl_checkAppValid());
	}
	
	if(systemControl_getAppRun()){
		if(sysControlHandler.sysStatus.bit.applicationState == APP_STOPPED){
			mlp_sysMessage("App Start");
			
			if(systemControl_checkAppValid()){
				kernel.kernelSignals->initApp = true;
				bus_clearAppReceiveBuffer();
				bus_enableAppMessageBuffer(true);
			}else{
				sysControlHandler.sysStatus.bit.applicationState = APP_STOPPED;
				mlp_sysError("App CRC Error");
				kernel.kernelSignals->initApp = false;
			}
		
			kernel.appSignals->appReady = false;
			kernel.appSignals->shutdownReady = false;
			kernel.kernelSignals->shutdownApp = false;
		
			sysControlHandler.sysStatus.bit.applicationState = APP_STARTING;
		}
	}else{
		if(sysControlHandler.sysStatus.bit.applicationState == APP_RUNNING){
			mlp_sysMessage("App Shutdown");
			sysControlHandler.sysStatus.bit.applicationState = APP_SHUTDOWN;
		}
	}
	
	if(sysControlHandler.sysStatus.bit.applicationState == APP_RUNNING)
	{
		kernel.kernelSignals->initApp = false;
		kernel.kernelSignals->shutdownApp = false;
	}
	else if(sysControlHandler.sysStatus.bit.applicationState == APP_SHUTDOWN)
	{
		kernel.kernelSignals->initApp = false;
		kernel.kernelSignals->shutdownApp = true;
	}
	
	if(systemControl_appIsActive())
	{		
		volatile uint16_t startTick = tickTimer_getTick_us();
		volatile uint32_t startTime = tickTimer_getTickTime();
		
		appHead->appRun(); // <-------------------------------  Run app
		
		volatile uint32_t endTime = tickTimer_getTickTime();
		volatile uint16_t endTick = tickTimer_getTick_us();
		
		sysControlHandler.appBenchmark.tick_last = (uint16_t)(endTick-startTick);
		sysControlHandler.appBenchmark.ms_last   = (uint16_t)(endTime-startTime);
		
		if(sysControlHandler.appBenchmark.tick_last >= 1000)
		{
			sysControlHandler.appBenchmark.tick_last = (1000-startTick)+endTick;
			if(sysControlHandler.appBenchmark.ms_last > 0) sysControlHandler.appBenchmark.ms_last--;
		}
		
		benchmark_run(&sysControlHandler.appBenchmark);
	}	
	
	if(sysControlHandler.sysStatus.bit.applicationState == APP_STARTING && kernel.appSignals->appReady==true)
	{
		sysControlHandler.sysStatus.bit.applicationState = APP_RUNNING;
	}
	
	if(sysControlHandler.sysControl.bit.applicationFroceStop){
		mlp_sysWarning("Force App Stop");
	}
	
	if((sysControlHandler.sysStatus.bit.applicationState == APP_SHUTDOWN && kernel.appSignals->shutdownReady) || sysControlHandler.sysControl.bit.applicationFroceStop){
		sysControlHandler.sysControl.bit.applicationRun = false;
		sysControlHandler.sysControl.bit.applicationFroceStop = false;
		sysControlHandler.sysStatus.bit.applicationState = APP_STOPPED;
		bus_enableAppMessageBuffer(false);
	}
}

void systemControl_run(void)
{
	sysControlHandler.sysStatus.bit.userLedEnabled = sysControlHandler.sysControl.bit.userLedEnabled;
	sysControlHandler.sysStatus.bit.identify = sysControlHandler.sysControl.bit.identify;
	sysControlHandler.sysStatus.bit.applicationRunOnStartup = sysControlHandler.sysControl.bit.applicationRunOnStartup;	
	sysControlHandler.sysStatus.bit.messageLogEnabled = sysControlHandler.sysControl.bit.messageLogEnabled;	
	
	kernel.kernelSignals->ledDisabled = sysControlHandler.sysControl.bit.userLedEnabled;
	kernel.kernelSignals->administratorAccess = sysControlHandler.sysStatus.bit.administratorAccess;
}

bool systemControl_hasChanged(void)
{
	if((sysControlHandler.sysControl.reg != sysControlHandler.sysControlOld.reg) || (sysControlHandler.sysStatus.reg != sysControlHandler.sysStatusOld.reg) ){
		sysControlHandler.sysControlOld.reg = sysControlHandler.sysControl.reg;
		sysControlHandler.sysStatusOld.reg = sysControlHandler.sysStatus.reg;
		
		return true;
	}else{
		return false;
	}
}

sysSavedSettings_t systemControl_getSaveSetting(void)
{
	sysSavedSettings_t settings;
	
	settings.bit.userLedEnabled = sysControlHandler.sysStatus.bit.userLedEnabled;
	settings.bit.applicationRunOnStartup = sysControlHandler.sysStatus.bit.applicationRunOnStartup;
	settings.bit.messageLogEnabled = sysControlHandler.sysStatus.bit.messageLogEnabled;
	
	return settings;
}

void systemControl_clearErrorFlags(void)
{
	sysControlHandler.sysStatus.bit.systemError = false;
	sysControlHandler.sysStatus.bit.applicationCrcError = false;
	sysControlHandler.sysStatus.bit.watchdogWarning = false;
	sysControlHandler.sysStatus.bit.watchdogError = false;
}

void systemControl_setSysError(void)
{
	sysControlHandler.sysStatus.bit.systemError = true;
}

void systemControl_setWatchdogWarning(void)
{
	sysControlHandler.sysStatus.bit.watchdogWarning = true;
}

void systemControl_setWatchdogError(void)
{
	sysControlHandler.sysStatus.bit.watchdogError = true;
	
	// turn of auto-start
	sysControlHandler.sysStatus.bit.applicationRunOnStartup = false;
	sysControlHandler.sysControl.bit.applicationRun = false;
	
	dmp_sendHeartbeat();

	eeprom_writeByte(systemControl_getSaveSetting().reg , &eememData.systemSavedSettings);
}

void systemControl_setAppCrcError(bool error)
{
	sysControlHandler.sysStatus.bit.applicationCrcError = error;
}

bool systemControl_getAppRun(void)
{
	return ((sysControlHandler.sysStatus.bit.applicationCrcError == false)&&(sysControlHandler.sysControl.bit.applicationRun == true));
}

bool systemControl_appIsActive(void)
{
	applicationState_t appState = sysControlHandler.sysStatus.bit.applicationState;
	return ( (appState == APP_STARTING)||(appState == APP_RUNNING)||(appState == APP_SHUTDOWN) );
}

void benchmark_reset(app_benchmark_t *benchmark)
{
	benchmark->us_max = 0;
	benchmark->us_min = 0xFFFFFFFF;
	benchmark->us_avg = 0;
};

void benchmark_run(app_benchmark_t *benchmark)
{
	uint32_t time_us = ((uint32_t)benchmark->ms_last*1000) + benchmark->tick_last;
	
	benchmark->us_last = time_us;
	
	if(benchmark->us_max < time_us) benchmark->us_max = time_us;
	if(benchmark->us_min > time_us) benchmark->us_min = time_us;
	
	benchmark->us_avg += time_us;
	
	benchmark->us_avg = (benchmark->us_avg >>1);
}

void systemControl_flagApplicationError(void)
{
	sysControlHandler.sysStatus.bit.applicationError = true;
}