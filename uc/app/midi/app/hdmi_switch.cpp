#include "hdmi_switch.h"

uint8_t hdmi_cmd;
uint8_t hdmi_tx_counter;

uint32_t repeatTimer; 

#define NR_OF_REPETITIONS 3
#define REPETITION_DELAY 150

void hdmi_setMatrix(uint8_t out, uint8_t in)
{
	if(in < 1) in = 1;
	if(in > 4) in = 4;
	
	in--;
	uint8_t a[4] = { 0, 1, 2, 3};
	uint8_t b[4] = { 9,10,11,12};
		
		
	hdmi_tx_counter = 0;
	
	if(out == 'A') hdmi_cmd = a[in];
	else if(out == 'B') hdmi_cmd = b[in];
}

void hdmi_init(kernel_t *kernel, pin_port_t portNr, uint8_t pinNr)
{
	kernel->tickTimer_reset(&repeatTimer);
	
	necIR_init(kernel, portNr, pinNr);
}

void hdmi_handler(kernel_t *kernel)
{
	if(kernel->tickTimer_delay1ms(&repeatTimer, REPETITION_DELAY))
	{
		if(hdmi_tx_counter < NR_OF_REPETITIONS)
		{
			necIR_transmitCode(0x00,hdmi_cmd);
			hdmi_tx_counter++;
		}
	}	
}