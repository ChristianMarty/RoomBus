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

#ifndef HARDWARE_VERSION
	#define HARDWARE_VERSION_MAJOR 2
	#define HARDWARE_VERSION_MINOR 3
	#define HARDWARE_VERSION HARDWARE_VERSION_MAJOR,HARDWARE_VERSION_MINOR
#endif

#ifndef KERNEL_VERSION
	#define KERNEL_VERSION_MAJOR 2
	#define KERNEL_VERSION_MINOR 7
#endif
