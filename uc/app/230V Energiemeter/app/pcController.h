/*
 * pcController.h
 *
 * Created: 02.01.2024 23:45:52
 *  Author: Christian
 */ 


#ifndef PCCONTROLLER_H_
#define PCCONTROLLER_H_
#include "kernel/kernel.h"
#include "utility/edgeDetect.h"
#include "Raumsteuerung/stateReportProtocol.h"
#include "Raumsteuerung/bistableRelay.h"


#define  PC_BUTTON_LIGHT IO_A05
#define  PC_POWER_SWITCH IO_A06

struct {
	bool initTurnOn;
	bool initTurnOff;

	srp_state_t reportedState;
	
	srp_state_t peripheralState;
	srp_state_t computerState;
	
	srp_state_t _peripheralStateOld;
	srp_state_t _computerStateOld;

	bsRelay_t peripheralRelay;
	bsRelay_t computerRelay;
}pcController;


typedef enum {
	pc_unknown,
	pc_standby,
	pc_trunOn,
	pc_computerTurnOn,
	pc_peripheralTurnOn,
	pc_startupDelay,
	pc_computerTriggerPowerSwitch,
	pc_computerPowerSwitchDelay,
	pc_trunOnCompleted,
	pc_on,
	pc_trunOff,
	pc_off
} pcState_t;

uint32_t buttonBlinkTimer;
uint32_t pcStepTimer;

uint32_t pcIsOnTimoutTimer;

pcState_t pcState;

edgeDetect_t pcButtonEdgeDetect;

void pcController_init(const kernel_t *kernel)
{
	pcController.initTurnOn = false;
	pcController.initTurnOff = false;
	
	pcButtonEdgeDetect.oldState = true; // prevent turn on  on power up
	
	pcState = pc_unknown;
	
	pcController.reportedState = srp_state_undefined;
	
	pcController.peripheralState = srp_state_undefined;
	pcController.computerState = srp_state_undefined;
	
	pcController._peripheralStateOld = srp_state_transitioning;
	pcController._computerStateOld = srp_state_transitioning;
	
	kernel->tickTimer.reset(&buttonBlinkTimer);
	kernel->tickTimer.reset(&pcIsOnTimoutTimer);
}

bool pcController_handler(const kernel_t *kernel)
{
	bool powerLed = !pin_getInput(IO_A07);
	bool button =   !pin_getInput(IO_A04);
	
	bool reportedStateChanged = false;
	
	bsRelay_handler(kernel, &pcController.peripheralRelay);
	bsRelay_handler(kernel, &pcController.computerRelay);
	
	switch( bsRelay_state(&pcController.peripheralRelay)){
		case bsRelay_unknown: pcController.peripheralState = srp_state_undefined;  break;
		case bsRelay_on: pcController.peripheralState = srp_state_on;  break;
		case bsRelay_off: pcController.peripheralState = srp_state_off;  break;
		case bsRelay_transitioning: break;		
	}
	if(pcController.peripheralState != pcController._peripheralStateOld){
		pcController._peripheralStateOld = pcController.peripheralState;
		reportedStateChanged = true;
	}
			
	switch( bsRelay_state(&pcController.computerRelay)){
		case bsRelay_unknown: pcController.computerState = srp_state_undefined;  break;
		case bsRelay_on: pcController.computerState = srp_state_on;  break;
		case bsRelay_off: pcController.computerState = srp_state_off;  break;
		case bsRelay_transitioning: break;			
	}
	if(pcController.computerState != pcController._computerStateOld){
		pcController._computerStateOld = pcController.computerState;
		reportedStateChanged = true;
	}
	
	if(ed_onRising(&pcButtonEdgeDetect, button)){
		pcController.initTurnOn = true;
	}
	
	// turn off PC power after shutdown; avoid power off in standby
	if(powerLed == true) kernel->tickTimer.reset(&pcIsOnTimoutTimer); // reset PC power watchdog

	
	// handle state machine
	switch(pcState)
	{
		case pc_standby :
			break;
		
		case pc_trunOn :
			pcController.reportedState = srp_state_transitioning;
			reportedStateChanged = true;
			pcState = pc_peripheralTurnOn;
			break;
		
		case pc_peripheralTurnOn :
			bsRelay_set(&pcController.peripheralRelay, true);
			kernel->tickTimer.reset(&pcStepTimer);
			pcState = pc_computerTurnOn;
			break;
		
		case pc_computerTurnOn :
			if(kernel->tickTimer.delay1ms(&pcStepTimer,2000))
			{
				bsRelay_set(&pcController.computerRelay, true);
				kernel->tickTimer.reset(&pcStepTimer);
				pcState = pc_startupDelay;
			}
			break;
		
		case pc_startupDelay:
			if(kernel->tickTimer.delay1ms(&pcStepTimer,3000))  // wait 3 seconds for PC PSU 
			{
				pcState = pc_computerTriggerPowerSwitch;
			}
			break;

			
		case pc_computerTriggerPowerSwitch:
			if(powerLed == false){ // don't trigger power switch if PC is already running
				pin_setOutput(PC_POWER_SWITCH, true);
			}
			kernel->tickTimer.reset(&pcStepTimer);
			pcState = pc_computerPowerSwitchDelay;
			break;
		
		case pc_computerPowerSwitchDelay:
			if(kernel->tickTimer.delay1ms(&pcStepTimer,500))
			{
				pin_setOutput(PC_POWER_SWITCH, false);
				pcState = pc_trunOnCompleted;
			}
			break;
		
		case pc_trunOnCompleted :
			kernel->tickTimer.reset(&pcIsOnTimoutTimer); // reset PC power watchdog
			pcController.reportedState = srp_state_on;
			reportedStateChanged = true;
			pcState = pc_on;
			break;
			
		case pc_on :
			if(kernel->tickTimer.delay1ms(&pcIsOnTimoutTimer, 5000)){ // PC power watchdog
				pcController.initTurnOff = true;
			}
			if(pcController.initTurnOff){
				pcState = pc_trunOff;
				pcController.initTurnOff = false;
			}
			break;
		
		case pc_trunOff :
			bsRelay_set(&pcController.peripheralRelay, false);
			bsRelay_set(&pcController.computerRelay, false);
			pcController.reportedState = srp_state_off;
			reportedStateChanged = true;
			pcState = pc_off;
			break;
		
		case pc_off:
			if(pcController.initTurnOn){
				pcState = pc_trunOn;
				pcController.initTurnOn = false;
			}
			break;
		
		case pc_unknown:
		default:
			pcController.reportedState = srp_state_undefined;
			
			if(pcController.initTurnOn){
				pcState = pc_trunOn;
				pcController.initTurnOn = false;
			}
			break;
	}
	
			
	if(pcController.reportedState == srp_state_on) pin_setOutput(PC_BUTTON_LIGHT, true);
	else if(pcController.reportedState == srp_state_transitioning)
	{
		 if(kernel->tickTimer.delay1ms(&buttonBlinkTimer, 500)){
			  pin_tglOutput(PC_BUTTON_LIGHT);
		 }
	}
	else pin_setOutput(PC_BUTTON_LIGHT, false);
	
	return reportedStateChanged;
}



#endif /* PCCONTROLLER_H_ */