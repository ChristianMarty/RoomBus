#include "bistableRelay.h"


void bsRelay_init(bsRelay_t *relay, pin_port_t onPortNr, uint8_t onPinNr, pin_port_t offPortNr, uint8_t offPinNr)
{
	relay->onPinPortNr = onPortNr; 
	relay->onPinNr = onPinNr;
	relay->offPinPortNr = offPortNr; 
	relay->offPinNr	= offPinNr;
	
	relay->relayStep = bsRelay_idle;
	relay->relayState = bsRelay_unknown;
	
	pin_setOutput(onPortNr,onPinNr,false);
	pin_enableOutput(onPortNr,onPinNr);
	
	pin_setOutput(offPortNr,offPinNr,false);
	pin_enableOutput(offPortNr,offPinNr);	
}

void bsRelay_set(bsRelay_t *relay, bool state)
{
	if(state == true) relay->relayStep = bsRelay_initOn;
	else relay->relayStep = bsRelay_initOff;
}

void bsRelay_toggle(bsRelay_t *relay)
{
	switch(relay->relayState)
	{
		case bsRelay_unknown: relay->relayStep = bsRelay_initOn; break;
		case bsRelay_transitioning: break;
		case bsRelay_on: relay->relayStep = bsRelay_initOff; break;
		case bsRelay_off: relay->relayStep = bsRelay_initOn; break; 
	}
}

bsRelay_state_t bsRelay_state(bsRelay_t *relay)
{
	return 	relay->relayState;
}

void bsRelay_handler(const kernel_t *kernel, bsRelay_t *relay)
{
	switch(relay->relayStep)
	{
		case bsRelay_idle:		break;
		case bsRelay_initOn :	relay->relayState = bsRelay_transitioning;
								relay->relayStep = bsRelay_turnOn;
								kernel->tickTimer.reset(&relay->pulsTimer);
								break;
								
		case bsRelay_turnOn :	pin_setOutput(relay->onPinPortNr,relay->onPinNr, true);
								if(kernel->tickTimer.delay1ms(&relay->pulsTimer, RELAIS_SWITCH_ON_TIME)) relay->relayStep = bsRelay_isOn;
								break;
								
		case bsRelay_isOn	:   relay->relayState = bsRelay_on;
								relay->relayStep = bsRelay_idle;
								pin_setOutput(relay->onPinPortNr,relay->onPinNr, false);
								pin_setOutput(relay->offPinPortNr,relay->offPinNr, false);
								break;
								
		case bsRelay_initOff:	relay->relayState = bsRelay_transitioning;
								relay->relayStep = bsRelay_turnOff;
								kernel->tickTimer.reset(&relay->pulsTimer);
								break;
		
				
		case bsRelay_turnOff:   pin_setOutput(relay->offPinPortNr,relay->offPinNr, true);
								if(kernel->tickTimer.delay1ms(&relay->pulsTimer, RELAIS_SWITCH_OFF_TIME)) relay->relayStep = bsRelay_isOff;
								break;
				
		case bsRelay_isOff	:	relay->relayState = bsRelay_off;
								relay->relayStep = bsRelay_idle;
								pin_setOutput(relay->onPinPortNr,relay->onPinNr, false);
								pin_setOutput(relay->offPinPortNr,relay->offPinNr, false);
								break;
	}
}