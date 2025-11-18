//**********************************************************************************************************************
// FileName : system.c
// FilePath : uCodebase/driver/SAMx5x/kernel/
// Author   : Christian Marty
// Date		: 02.08.2025
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#include "system.h"

#include "driver/SAMx5x/kernel/fuse.h"

void system_initialize(void)
{
	// Setup OSC1
	OSCCTRL->XOSCCTRL[1].reg = 0x00F0A646; // Enable OSC1 -> 16MHz
	while(!OSCCTRL->STATUS.bit.XOSCRDY1); // Wait until OSC1 is ready to use
	
	// Setup PLL0
	OSCCTRL->Dpll[0].DPLLCTRLB.reg = 0x00070060; // Setup OSC1 as 1MHz Clock for PLL
	OSCCTRL->Dpll[0].DPLLRATIO.reg = 0x00000078; // Multiply by 120
	OSCCTRL->Dpll[0].DPLLCTRLA.reg = 0x42; // Enable PLL
	while(!OSCCTRL->Dpll[0].DPLLSTATUS.bit.CLKRDY); // Wait until PLL is ready to use	

	// Sets CPU Clk (GCLK 0) to 120MHz
	GCLK[0].GENCTRL->reg |= 0x01; // Set OSC
	
	fuse_initialize();
}

void system_reboot(void)
{
	SCB->AIRCR = 0x5FA0004;
}
