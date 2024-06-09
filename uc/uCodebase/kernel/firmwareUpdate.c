/*
 * firmwareUpdate.cpp
 *
 * Created: 07.10.2018 14:59:42
 *  Author: Christian
 */ 

#include "kernel/deviceManagementProtocol.h"
#include "kernel/firmwareUpdate.h"
#include "drv/SAMx5x/flash.h"
#include "kernel/systemControl.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RESPONSE_PRIORITY busPriority_low

#define APP_START_ADDRESS (NVMCTRL_BLOCK_SIZE*8)
#define APP_END_ADDRESS (NVMCTRL_BLOCK_SIZE*60)

bool erase = false;
uint8_t adr;

void fu_handler(sysControlData_t *systemControl, sysStatusData_t *systemStatus, appState_t *appStatePtr)
{
	if(erase)
	{
		if(*appStatePtr == APP_RUN)
		{
			*appStatePtr = APP_SHUTDOWN;
		}
		else if(*appStatePtr == APP_STOP)
		{
			uint8_t blocksToErase = (APP_END_ADDRESS - APP_START_ADDRESS) / flash_getBlockSize();
			
			for(uint32_t i = 0; i<blocksToErase; i++)
			{
				flash_eraseBlock(APP_START_ADDRESS+ (i*flash_getBlockSize()) );
			}
			
			bus_message_t msg;

			bus_getMessageSlot(&msg);
			bus_writeHeader(&msg,adr, busProtocol_deviceManagementProtocol, 1, RESPONSE_PRIORITY);
			bus_pushByte(&msg,dmp_cmdEraseAppRsp);
			bus_send(&msg);
			
			erase = false;
		}
	}	
}

void fu_eraseApp(uint8_t sourceAddress, uint8_t *data, uint8_t size)
{
	adr = sourceAddress;
	erase = true;
}

void fu_writeAppData(uint8_t sourceAddress, uint8_t *data, uint8_t size)
{
	bus_message_t msg;

	bus_getMessageSlot(&msg);
	bus_writeHeader(&msg,sourceAddress, busProtocol_deviceManagementProtocol, 1, RESPONSE_PRIORITY);
	
	uint32_t memAddress = APP_START_ADDRESS + (((uint32_t)data[0]<<24)|((uint32_t)data[1]<<16)|((uint32_t)data[2]<<8)|((uint32_t)data[3]));

	if(memAddress <= APP_END_ADDRESS) // Address range check
	{
		flash_writePage(memAddress, &data[4], (size-4));

		bus_pushByte(&msg,dmp_cmdBtlRsp);
		bus_pushWord32(&msg,memAddress);
		bus_pushByte(&msg,(size-4));
		
		bus_send(&msg);
	}
}

#ifdef __cplusplus
}
#endif