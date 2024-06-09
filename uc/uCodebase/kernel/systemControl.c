/*
 * systemControl.cpp
 *
 * Created: 14.07.2019 12:10:54
 *  Author: Christian
 */ 

#include "systemControl.h"
#include "drv/SAMx5x/system.h"


void systemControl_init(systemControl_t *self, sysSavedSettings_t *settings)
{
	self->sysControl->bit.appRunOnStartup = settings->bit.appRunOnStartup;
	
	self->sysControl->bit.ledOnOff = settings->bit.ledOnOff;
	
	self->sysStatus->bit.appRunOnStartup = self->sysControl->bit.appRunOnStartup;
	
	self->sysStatus->bit.ledOnOff = self->sysControl->bit.ledOnOff;;
	
	self->sysStatus->bit.identify = self->sysControl->bit.identify;
	
	self->sysStatus->bit.appCrcError = true;
	
	if(self->sysStatus->bit.appRunOnStartup) self->sysControl->bit.appRun = true;
}


void systemControl_run(systemControl_t *self)
{
	self->sysStatus->bit.ledOnOff = self->sysControl->bit.ledOnOff;
	self->sysStatus->bit.identify = self->sysControl->bit.identify;
	self->sysStatus->bit.appRunOnStartup = self->sysControl->bit.appRunOnStartup;	
}

bool systemControl_hasChanged(systemControl_t *self)
{
	if( (self->sysControl->reg != self->sysControlOld.reg) || (self->sysStatus->reg != self->sysStatusOld.reg) )
	{
		self->sysControlOld.reg = self->sysControl->reg;
		self->sysStatusOld.reg = self->sysStatus->reg;
		
		return true;
	} 
	else
	{
		return false;
	}
}

sysSavedSettings_t systemControl_getSaveStting(systemControl_t *self)
{
	sysSavedSettings_t settings;
	
	settings.bit.ledOnOff = self->sysStatus->bit.ledOnOff;
	settings.bit.appRunOnStartup = self->sysStatus->bit.appRunOnStartup;
	
	return settings;
}

void systemControl_clearErrorFlags(systemControl_t *self)
{
		self->sysStatus->bit.systemError = false;
		self->sysStatus->bit.appCrcError = false;
		self->sysStatus->bit.watchdogWarning = false;
		self->sysStatus->bit.watchdogError = false;
}

void systemControl_setSysError(systemControl_t *self)
{
	self->sysStatus->bit.systemError = true;
}

void systemControl_setWatchdogWarning(systemControl_t *self)
{
	self->sysStatus->bit.watchdogWarning = true;
}

void systemControl_setWatchdogError(systemControl_t *self)
{
	self->sysStatus->bit.watchdogError = true;
}

void systemControl_interveneStartup(systemControl_t *self)
{
	self->sysStatus->bit.appRunOnStartup = false;
	self->sysControl->bit.appRun = false;
}

void systemControl_setAppCrcError(systemControl_t *self, bool error)
{
	self->sysStatus->bit.appCrcError = error;
}

bool systemControl_getAppRun(systemControl_t *self)
{
	return ((self->sysStatus->bit.appCrcError == false)&&(self->sysControl->bit.appRun == true));
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