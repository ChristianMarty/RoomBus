/*
 * app.cpp
 *
 * Created: 20.04.2019 20:05:30
 * Author : Christian
 */ 

#include "sam.h"
#include "kernel/kernel.h"
#include "kernel/busController_IO.h"

#include "drv/SAMx5x/genericClockController.h"

#include "Raumsteuerung/stateReportProtocol.h"
#include "Raumsteuerung/triggerProtocol.h"

#include "Raumsteuerung/nextion.h"

int main(const kernel_t *kernel);
void onReceive(const kernel_t *kernel, uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size);
kernel_t const *_kernel;

__attribute__((section(".appHeader"))) appHead_t appHead ={
/*appCRC	 */ 0xAABBCCDD, // Will be written by Bootload tool
/*appSize	 */ 0xEEFF0000, // Will be written by Bootload tool
/*appRevMaj	 */ 0x01,
/*appRevMin	 */ 0x00,
/*appName[60]*/ "Touch display",
/*main		 */ main,
/*onRx		 */ onReceive
};

#define LOADSWITCH_ADDRESS 0x02
#define PROJECTOR_ADDRESS 0x05
#define SCREEN_ADDRESS PROJECTOR_ADDRESS

#define DISPLAY_TIMEOUT 10000

uint32_t displayOnTimer;
uint32_t displayComTimer;
uint8_t displayPage;
uint8_t displayPageOld;
volatile bool statusChanged = false;

typedef void (*touchEventHandler_t)(const kernel_t *kernel, uint8_t index, uint8_t page, uint8_t component, srp_state_t * const state);

typedef struct  {
	uint8_t page;
	uint8_t component;
	uint8_t objName[4];
	srp_state_t * const state;
	touchEventHandler_t eventHandler;
}touchEventAction_t;

void stdEventHandler(const kernel_t *kernel, uint8_t index, uint8_t page, uint8_t component, srp_state_t * const state);
void hdmiMatrixHandler(const kernel_t *kernel, uint8_t index, uint8_t page, uint8_t component, srp_state_t * const state);

srp_state_t stateList[16];

touchEventAction_t touchEventAction[] = {
	{ 1, 1, "bt0", &stateList[0], stdEventHandler},
	{ 1, 2, "bt1", &stateList[1], stdEventHandler},
	{ 1, 3, "bt2", &stateList[2], stdEventHandler},
		
	{ 2, 1, "bt0", &stateList[3], stdEventHandler},
	{ 2, 2, "bt1", &stateList[4], stdEventHandler},
	{ 2, 3, "bt2", &stateList[5], hdmiMatrixHandler},
	{ 2, 4, "bt3", &stateList[6], hdmiMatrixHandler},
	{ 2, 5, "bt4", &stateList[7], hdmiMatrixHandler},
	{ 2, 6, "bt5", &stateList[8], hdmiMatrixHandler},
		
	{ 3, 1, "bt0", &stateList[9], stdEventHandler},
	{ 3, 2, "bt1", &stateList[10], stdEventHandler},
	{ 3, 3, "bt2", &stateList[11], stdEventHandler},
	{ 3, 4, "bt3", &stateList[12], stdEventHandler},
	{ 3, 5, "bt4", &stateList[13], stdEventHandler},
	
	{ 4, 1, "bt1", &stateList[14], stdEventHandler}
};
#define touchEventListSize (sizeof(touchEventAction)/sizeof(touchEventAction_t))

const srp_stateReport_t stateReports[] = {
	{LOADSWITCH_ADDRESS, 0x01, &stateList[0] },
	{LOADSWITCH_ADDRESS, 0x02, &stateList[1] },
	{LOADSWITCH_ADDRESS, 0x03, &stateList[2] },
		
	{PROJECTOR_ADDRESS, 0x00, &stateList[3] },
	{PROJECTOR_ADDRESS, 0x01, &stateList[4] },
	{PROJECTOR_ADDRESS, 0x02, &stateList[5] },
	{PROJECTOR_ADDRESS, 0x03, &stateList[6] },
	{PROJECTOR_ADDRESS, 0x04, &stateList[7] },
	{PROJECTOR_ADDRESS, 0x05, &stateList[8] },
		
	{SCREEN_ADDRESS, 0x06, &stateList[9] },
	{SCREEN_ADDRESS, 0x07, &stateList[10] },
	{SCREEN_ADDRESS, 0x08, &stateList[11] },
	{SCREEN_ADDRESS, 0x09, &stateList[12] },
	{SCREEN_ADDRESS, 0x0A, &stateList[13] },
		
	{LOADSWITCH_ADDRESS, 0x12, &stateList[14] },
};
#define stateReportListSize (sizeof(stateReports)/sizeof(srp_stateReport_t))

const tsp_triggerSignal_t triggerSignals[] = {
	{ 0, LOADSWITCH_ADDRESS, 0x06, "Button 1"},
	{ 1, LOADSWITCH_ADDRESS, 0x09, "Button 2"},
	{ 2, LOADSWITCH_ADDRESS, 0x0C, "Button 3"},
		
	{ 3, PROJECTOR_ADDRESS, 0x02, "Projector On/Off"},
	{ 4, PROJECTOR_ADDRESS, 0x05, "Projector Blackout"},
	{ 5, PROJECTOR_ADDRESS, 0x06, "HDMI Out A - In 1"},
	{ 6, PROJECTOR_ADDRESS, 0x07, "HDMI Out A - In 2"},
	{ 7, PROJECTOR_ADDRESS, 0x08, "HDMI Out A - In 3"},
	{ 8, PROJECTOR_ADDRESS, 0x09, "HDMI Out A - In 4"},
	{ 9, PROJECTOR_ADDRESS, 0x0E, "HDMI Off"},
			
	{10, SCREEN_ADDRESS, 0x10, "No Screen"},
	{11, SCREEN_ADDRESS, 0x11, "Screen 16:9"},
	{12, SCREEN_ADDRESS, 0x12, "Screen Up"},
	{13, SCREEN_ADDRESS, 0x13, "Screen Down"},
	//{14, SCREEN_ADDRESS, 0x14, "Screen Stop"},
		
	{14, LOADSWITCH_ADDRESS, 0x40, "Amp"},	
};
#define triggerSignalListSize (sizeof(triggerSignals)/sizeof(tsp_triggerSignal_t))

const triggerSlotProtocol_t triggerSystem = {
	.signals = triggerSignals,
	.signalSize = triggerSignalListSize,
	.slots = nullptr,
	.slotSize = 0,
	._slotsStatus = nullptr
};

const stateReportProtocol_t stateReport = {
	.states = stateReports,
	.stateSize = stateReportListSize,
	.channels = nullptr,
	.channelSize = 0
};

void onReceive(const kernel_t *kernel, uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size)
{
	switch(protocol)
	{
		//case busProtocol_triggerProtocol:		tsp_receiveHandler(kernel, sourceAddress, command,data, size, &triggerSystem);	break;
		case busProtocol_stateReportProtocol:	srp_receiveHandler(kernel, sourceAddress, command,data, size, &stateReport);	break;
	}
	
}

void nextion_onTouchEvent_callback(const kernel_t *kernel, uint8_t page, uint8_t component)
{	
	for(uint8_t i = 0; i< touchEventListSize; i++)
	{
		if(touchEventAction[i].page == page && touchEventAction[i].component == component)
		{
			touchEventAction[i].eventHandler(kernel, i, page, component, touchEventAction[i].state);
		}
	}
	kernel->tickTimer.reset(&displayOnTimer);
}

void stdEventHandler(const kernel_t *kernel, uint8_t index, uint8_t page, uint8_t component, srp_state_t * const state)
{
	uint8_t indexList[triggerSignalListSize];
	uint8_t indexSize = 0;
	
	//for(uint8_t i = 0; i< touchEventListSize; i++)
	//{
		if(touchEventAction[index].page == page && touchEventAction[index].component == component)
		{
			/*if(index > 4 && index < 9 && state == false)
			{
				indexList[indexSize] = 9;
			}
			else
			{
				if(index>9)indexList[indexSize] = index+1;
				else indexList[indexSize] = index;
				
			}*/
			
			indexList[indexSize] = index;
			indexSize++;
		}
//	}
	
	if(indexSize) tep_sendTriggerByGroup(kernel, &indexList[0], indexSize, triggerSignals, triggerSignalListSize);
}

void hdmiMatrixHandler(const kernel_t *kernel, uint8_t index, uint8_t page, uint8_t component, srp_state_t * const state)
{
	uint8_t indexList[1];
	
	if(*state == srp_state_on) indexList[0] = 9;
	else indexList[0] = index;
	
	tep_sendTriggerByGroup(kernel, &indexList[0], 1, triggerSignals, triggerSignalListSize);
}

void tsp_onStateChange_callback(const kernel_t *kernel)
{
	statusChanged = true;
}

void display_on(void)
{
	pin_setOutput(IO_D17,true);	
}

void display_off(void)
{
	pin_setOutput(IO_D17,false);	
}

uint16_t i = 0;

int main(const kernel_t *kernel)
{
	//_kernel = kernel;
	
	// App Init Code
	if(kernel->kernelSignals->initApp)
	{
		Reset_Handler();
		
		kernel->tickTimer.reset(&displayOnTimer);
		kernel->tickTimer.reset(&displayComTimer);
		
		pin_enableInput(IO_D04);
		pin_enableInput(IO_D05);
		pin_enableInput(IO_D06);
		pin_enableInput(IO_D07);	
		
		pin_enableOutput(IO_D17);
		pin_setOutput(IO_D17,true);	
		
		nextion_init(kernel);
		
		srp_stateReportRequestAll(kernel, &stateReport);
		
		display_off();
		
		kernel->appSignals->appReady = true;		
		
	}

//----- Main -------------------------------------------------------------------------------------------------	
	
	if(kernel->appSignals->appReady == true)
	{	
		if(!pin_getInput(IO_D06)) displayPage = 1;
		else if(!pin_getInput(IO_D07)) displayPage = 2;
		else if(!pin_getInput(IO_D04)) displayPage = 3;
		else if(!pin_getInput(IO_D05)) displayPage = 4;
				
		if(kernel->tickTimer.delay1ms(&displayOnTimer,DISPLAY_TIMEOUT))
		{
			if(displayPage != 0)
			{
				displayPage = 0;
				displayPageOld = 0;
				i = 0;
				display_off();
			}
		}
		
		if(displayPageOld != displayPage)
		{
			kernel->tickTimer.reset(&displayOnTimer);
			statusChanged = true;
		}
		
		nextion_handler(kernel);
		
//----- Update Display ---------------------------------------------------------------------------------------		
 		if(statusChanged && displayPage != 0)
		{
			
			if(kernel->tickTimer.delay1ms(&displayComTimer,4))
			{
				if(i == 0)
				{
					display_on();
				}
				else if(i <49)
				{
					// Wait for turn on
				}
				else if(i == 49)
				{
					nextion_resetCom();
					
					if(displayPageOld != displayPage)
					{	
						uint8_t temp[] = "page 0";
						temp[5] = displayPage +0x30;
						nextion_sendCommand(kernel, temp);
						displayPageOld = displayPage;
					}		
				}
				else if(touchEventAction[i-50].page == displayPage)
				{
					
					uint8_t temp[] = "bt0.val=1";
					temp[2] = touchEventAction[i-50].objName[2];
					if(*(touchEventAction[i-50].state) == srp_state_on)temp[8] = '1'; 
					else if(*(touchEventAction[i-50].state) == srp_state_off)temp[8] = '0';
					nextion_sendCommand(kernel, temp);
				}
					
				i++;

				if(i >= (touchEventListSize+50))
				{
					i = 0;
					statusChanged = false;
				}
			}
 		} 
	}
	
	// App Deinit code
    if(kernel->kernelSignals->shutdownApp)
    {
		//kernel->eemem_write(&ememData[0]);
		kernel->appSignals->shutdownReady = true;
    }
	
}