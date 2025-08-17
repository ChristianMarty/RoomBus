//**********************************************************************************************************************
// FileName : main.h
// FilePath :
// Author   : Christian Marty
// Date		: 08.09.2024
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#include "sam.h"

#ifndef KERNEL_HARDWARE_NAME
	#define KERNEL_HARDWARE_NAME "BUS-Controller"
#endif

#ifndef APP_START_ADDR
	#define APP_START_ADDR   0x10000
#endif

#ifndef HARDWARE_VERSION_MAJOR
	#define HARDWARE_VERSION_MAJOR 2
	#define HARDWARE_VERSION_MINOR 3
#endif

#ifndef KERNEL_VERSION_MAJOR
	#define KERNEL_VERSION_MAJOR 3
	#define KERNEL_VERSION_MINOR 0
#endif


/******************************************************************************
Administration Mode Button (AMB) settings
******************************************************************************/
#define AmbInitialization() PORT->Group[0].PINCFG[31].bit.INEN = true
#define AmbRead() (PORT->Group[0].IN.reg & PORT_PA31)


/******************************************************************************
LED settings
******************************************************************************/
#define LedInitialization() PORT->Group[0].DIRSET.reg = (PORT_PA27|PORT_PA01|PORT_PA00)

#define LedRedEnable()  PORT->Group[0].OUTCLR.reg= PORT_PA00
#define LedRedDisable() PORT->Group[0].OUTSET.reg = PORT_PA00

#define LedGreenEnable()  PORT->Group[0].OUTCLR.reg= PORT_PA27
#define LedGreenDisable() PORT->Group[0].OUTSET.reg = PORT_PA27

#define LedBlueEnable()  PORT->Group[0].OUTCLR.reg= PORT_PA01
#define LedBlueDisable() PORT->Group[0].OUTSET.reg = PORT_PA01


/******************************************************************************
CAN-BUS settings
******************************************************************************/
#define BusTransceiverInitialization() PORT->Group[1].DIRSET.reg = PORT_PB09 // set pin as output
#define BusTransceiverEnable()  PORT->Group[1].OUTCLR.reg = PORT_PB09
#define BusTransceiverDisable() PORT->Group[1].OUTSET.reg = PORT_PB09