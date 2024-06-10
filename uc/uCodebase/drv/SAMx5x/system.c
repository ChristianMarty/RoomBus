/*
 * system.c
 *
 * Created: 12.01.2019 21:38:44
 *  Author: Christian
 */ 


#include "drv/SAMx5x/system.h"
#include "drv/SAMx5x/fuse.h"
#include "protocol/messageLogProtocol.h"

void system_init(void)
{
	// Setup OSC1
	OSCCTRL->XOSCCTRL[1].reg = 0x00F0A646; // Enable OSC1
	while(!OSCCTRL->STATUS.bit.XOSCRDY1); // Wait until OSC1 is ready to use
	
	// Setup PLL0
	OSCCTRL->Dpll[0].DPLLCTRLB.reg = 0x00070060; // Setup OSC1 as 1MHz Clock for PLL
	OSCCTRL->Dpll[0].DPLLRATIO.reg = 0x00000078; // Multiply by 120
	OSCCTRL->Dpll[0].DPLLCTRLA.reg = 0x42; // Enable PLL
	while(!OSCCTRL->Dpll[0].DPLLSTATUS.bit.CLKRDY); // Wait until PLL is ready to use	

	// Sets CPU Clk (GCLK 0) to 120MHz
	GCLK[0].GENCTRL->reg |= 0x01; // Set OSC
}

void system_reboot(void)
{
	SCB->AIRCR = 0x5FA0004;
}



void system_configure(void)
{
	fuse_init();
	//fuse_setDefault();
	return;
	
	/*fuseUserPage_t userPage = fuse_getUserPage();
	bool write = false;
	
	// If smartEEPROM is not enabled -> enable smartEEPRM
	if((userPage.SEEPSZ != EEMEM_SEEPSZ)&&(userPage.SEESBLK != EEMEM_SEESBLK))
	{
		// SmartEEPROM Virtual Size: 512 Byte
		userPage.SEEPSZ = EEMEM_SEEPSZ;
		userPage.SEESBLK = EEMEM_SEESBLK;
		write = true;
		
		//mlp_appMessage("Configure: EEPROM -> 512 Byte");
	}
	
	if(userPage.BOD33Disable == 0x01)
	{
		userPage.BOD33Disable = 0x0;
		
		//mlp_appMessage("Configure: Brown Out Detection");
		
		write = true;
	}
	
	if(write)
	{
		//mlp_appMessage("Write device configuration to NVM User Page");
		fuse_setUserPage(userPage);
	}*/
	
}