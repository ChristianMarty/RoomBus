#include "hdmi_switch.h"
#include "interface/necIr.h"



#define NR_OF_REPETITIONS 3
#define REPETITION_DELAY 150
#define TURN_ON_TIME   2500

typedef enum{
	hdmi_state_on,
	hdmi_state_off,
	hdmi_state_turnOn,
	hdmi_state_turningOn,
	hdmi_state_switchOn,
	hdmi_state_switchOff
	
} hdmi_state_t;

hdmi_state_t hdmi_state;

uint8_t hdmi_cmd;
uint8_t hdmi_tx_counter;

uint32_t repeatTimer;
uint32_t hdmi_turnOnTimer;

void hdmi_setMatrix(uint8_t out, uint8_t in)
{
	const uint8_t a[4] = { 0, 1, 2, 3};
	const uint8_t b[4] = { 9,10,11,12};
		
	if(hdmi_state == hdmi_state_switchOff) hdmi_state = hdmi_state_on;
	else if(hdmi_state == hdmi_state_off) hdmi_state = hdmi_state_switchOn;
	
	while(necIR_busy());
	
	if(in < 1) in = 1;
	if(in > 4) in = 4;
	
	in--;
		
	hdmi_tx_counter = 0;
	
	if(out == 'A') hdmi_cmd = a[in];
	else if(out == 'B') hdmi_cmd = b[in];
}

void hdmi_turnOff(void)
{
	hdmi_state = hdmi_state_switchOff;
}

bool hdmi_getSwitchOn(void)
{
	if(hdmi_state == hdmi_state_switchOn)
	{
		kernel.tickTimer.reset(&hdmi_turnOnTimer);
		hdmi_state = hdmi_state_turnOn;
		return true;
	}
		
	return false;
}

bool hdmi_getSwitchOff(void)
{
	if(hdmi_state == hdmi_state_switchOff)
	{
		hdmi_state = hdmi_state_off;
		return true;
	}
		
	return false;
}

void hdmi_init(pin_port_t portNr, uint8_t pinNr)
{
	kernel.tickTimer.reset(&repeatTimer);
	kernel.tickTimer.reset(&hdmi_turnOnTimer);
	hdmi_state = hdmi_state_off;
	
	necIR_init(portNr, pinNr);
}

void hdmi_handler(void)
{
	if(hdmi_state == hdmi_state_turnOn)
	{
		if(kernel.tickTimer.delay1ms(&hdmi_turnOnTimer, TURN_ON_TIME))
		{
			hdmi_state = hdmi_state_on;
		}
	}
	
	if(hdmi_state == hdmi_state_on)
	{
		if(hdmi_tx_counter == 0)
		{
			hdmi_tx_counter ++;
			kernel.tickTimer.reset(&repeatTimer);
		}
		else if(hdmi_tx_counter < NR_OF_REPETITIONS+1)
		{
			if(kernel.tickTimer.delay1ms(&repeatTimer, REPETITION_DELAY))
			{
				necIR_transmitCode(0x00, hdmi_cmd);
				hdmi_tx_counter++;
			}
		}	
	}
}
