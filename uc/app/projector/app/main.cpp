//**********************************************************************************************************************
// FileName : main.cpp
// FilePath :
// Author   : Christian Marty
// Date		: 20.04.2019
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#include "main.h"

#include "common/kernel.h"

#include "protocol/triggerSystemProtocol.h"
#include "protocol/stateSystemProtocol.h"
#include "protocol/eventSystemProtocol.h"

#include "driver/SAMx5x/pin.h"
#include "driver/SAMx5x/i2cMaster.h"

#include "utility/string.h"
#include "utility/bistableRelay.h"
#include "hdmi_switch.h"
#include "projector.h"

int main(void);
kernel_t kernel __attribute__((section(".kernelCall")));

__attribute__((section(".appHeader"))) applicationHeader_t appHead ={
/*appCRC	 */ 0xAABBCCDD, // Will be written by Bootload tool
/*appSize	 */ 0xEEFF0000, // Will be written by Bootload tool
/*appRevMaj	 */ 0x01,
/*appRevMin	 */ 0x00,
/*appName[60]*/ "Projector / HDMI-Switch Controller",
/*main		 */ main
};

#define OFF_DELAY 30000
#define STATE_SEND_INTERVAL 60000
#define POWER_DELIVERY_EVENT_TIMEOUT 25000

ssp_state_t mainPowerState;
ssp_state_t projectorBlackout;

bsRelay_t relay1;

//**** State Configuration ********************************************************************************************
#define STATE_SIGNAL_INTERVAL 10
const ssp_stateSignal_t stateSignals[] = {
	{0x80, "Projector Power State", STATE_SIGNAL_INTERVAL},
	{0x81, "Projector Blackout State", STATE_SIGNAL_INTERVAL},
	
	{0x82, "HDMI Out A - In 1", STATE_SIGNAL_INTERVAL},
	{0x83, "HDMI Out A - In 2", STATE_SIGNAL_INTERVAL},
	{0x84, "HDMI Out A - In 3", STATE_SIGNAL_INTERVAL},
	{0x85, "HDMI Out A - In 4", STATE_SIGNAL_INTERVAL},
	
	{0x86, "HDMI Out B - In 1", STATE_SIGNAL_INTERVAL},
	{0x87, "HDMI Out B - In 2", STATE_SIGNAL_INTERVAL},
	{0x88, "HDMI Out B - In 3", STATE_SIGNAL_INTERVAL},
	{0x89, "HDMI Out B - In 4", STATE_SIGNAL_INTERVAL}
};
#define stateSystemSignaListSize ARRAY_LENGTH(stateSignals)

ssp_itemState_t stateSystemSignaStatusList[stateSystemSignaListSize];

const stateSystemProtocol_t stateSystem = {
	.signals = stateSignals,
	.signalSize = stateSystemSignaListSize,
	.slots = nullptr,
	.slotSize = 0,
	._signalState = stateSystemSignaStatusList,
	._slotState = nullptr
};

//**** Trigger Configuration ******************************************************************************************
void projector_pwr(uint16_t triggerChannelNumber);
void projector_blackout(uint16_t triggerChannelNumber);
void hdmi_matrix(uint16_t triggerChannelNumber);

const tsp_triggerSlot_t triggerSlotList[] = {
	{0x80, "Projector On", projector_pwr},
	{0x81, "Projector Off", projector_pwr},
	{0x82, "Projector Toggle", projector_pwr},
		
	{0x83, "Projector Blackout On", projector_blackout},
	{0x84, "Projector Blackout Off", projector_blackout},
	{0x85, "Projector Blackout Toggle", projector_blackout},
		
	{0x86, "HDMI Out A - In 1", hdmi_matrix},
	{0x87, "HDMI Out A - In 2", hdmi_matrix},
	{0x88, "HDMI Out A - In 3", hdmi_matrix},
	{0x89, "HDMI Out A - In 4", hdmi_matrix},
	
	{0x8A, "HDMI Out B - In 1", hdmi_matrix},
	{0x8B, "HDMI Out B - In 2", hdmi_matrix},
	{0x8C, "HDMI Out B - In 3", hdmi_matrix},
	{0x8D, "HDMI Out B - In 4", hdmi_matrix},
	{0x8E, "HDMI Off", hdmi_matrix}, 
};
#define triggerSlotListSize ARRAY_LENGTH(triggerSlotList)

tsp_itemState_t triggerSlotStateList[triggerSlotListSize];
const triggerSystemProtocol_t triggerSystem = {
	.signals = nullptr,
	.signalSize = 0,
	.slots = triggerSlotList,
	.slotSize = triggerSlotListSize,
	._signalState = nullptr,
	._slotState = triggerSlotStateList
};

//**** Event Configuration ********************************************************************************************
const esp_eventSignal_t eventSignals[] = {
	{0x03, "A/V Aux Power", POWER_DELIVERY_EVENT_TIMEOUT}
};
#define eventSignalListSize ARRAY_LENGTH(eventSignals)

esp_itemState_t _eventSignalStatus[eventSignalListSize];
const eventSystemProtocol_t eventSystem = {
	.signals = eventSignals,
	.signalSize = eventSignalListSize,
	.slots = nullptr,
	.slotSize = 0,
	._signalState = _eventSignalStatus,
	._slotState = nullptr
};

void handleReceive(void)
{
	bus_rxMessage_t message;
	if(!kernel.bus.getReceivedMessage(&message)) return;
	
	bool processed = false;
	switch(message.protocol){
		case busProtocol_triggerSystemProtocol:	processed = tsp_receiveHandler(&triggerSystem, &message); break;
		case busProtocol_eventSystemProtocol:	processed = esp_receiveHandler(&eventSystem,  &message); break;
		case busProtocol_stateSystemProtocol:	processed = ssp_receiveHandler(&stateSystem, &message); break;
	}
	kernel.bus.receivedProcessed(processed);
}

void tsp_onStateChange_callback(const kernel_t *kernel)
{
	
}

void projector_pwr(uint16_t triggerChannelNumber)
{
	switch(triggerChannelNumber)
	{
		case  0x80:	projector_onOff(true);
					break;
					
		case  0x81:	projector_onOff(false);
					break;
					
		case  0x82:	if(mainPowerState == srp_state_off)projector_onOff(true);
					else if(mainPowerState == srp_state_on)projector_onOff(false);
					break;
	}			
}

void projector_blackout(uint16_t triggerChannelNumber)
{
	switch(triggerChannelNumber)
	{
		case 0x83:	projector_blank(true);
					break;
		
		case 0x84:	projector_blank(false);
					break;
		
		case 0x85:	if(projectorBlackout == srp_state_off)projector_blank(true);
					else if(projectorBlackout == srp_state_on)projector_blank(false);
					break;
	}
}

void hdmi_matrix(uint16_t triggerChannelNumber)
{
	switch(triggerChannelNumber)
	{
		case  0x86:	hdmi_setMatrix('A',1); 
					ssp_setStateByIndex(&stateSystem, 2, srp_state_on);
					ssp_setStateByIndex(&stateSystem, 3, srp_state_off);
					ssp_setStateByIndex(&stateSystem, 4, srp_state_off);
					ssp_setStateByIndex(&stateSystem, 5, srp_state_off);
					break;
					
		case  0x87:	hdmi_setMatrix('A',2); 
					ssp_setStateByIndex(&stateSystem, 2, srp_state_off);
					ssp_setStateByIndex(&stateSystem, 3, srp_state_on);
					ssp_setStateByIndex(&stateSystem, 4, srp_state_off);
					ssp_setStateByIndex(&stateSystem, 5, srp_state_off);
					break;
					
		case  0x88:	hdmi_setMatrix('A',3); 
					ssp_setStateByIndex(&stateSystem, 2, srp_state_off);
					ssp_setStateByIndex(&stateSystem, 3, srp_state_off);
					ssp_setStateByIndex(&stateSystem, 4, srp_state_on);
					ssp_setStateByIndex(&stateSystem, 5, srp_state_off);
					break;
					
		case  0x89:	hdmi_setMatrix('A',4);
					ssp_setStateByIndex(&stateSystem, 2, srp_state_off);
					ssp_setStateByIndex(&stateSystem, 3, srp_state_off);
					ssp_setStateByIndex(&stateSystem, 4, srp_state_off);
					ssp_setStateByIndex(&stateSystem, 5, srp_state_on);
					break;
		
		case  0x8A:	hdmi_setMatrix('B',1); 
					ssp_setStateByIndex(&stateSystem, 6, srp_state_on);
					ssp_setStateByIndex(&stateSystem, 7, srp_state_off);
					ssp_setStateByIndex(&stateSystem, 8, srp_state_off);
					ssp_setStateByIndex(&stateSystem, 9, srp_state_off);
					break;
					
		case  0x8B:	hdmi_setMatrix('B',2); 
					ssp_setStateByIndex(&stateSystem, 6, srp_state_off);
					ssp_setStateByIndex(&stateSystem, 7, srp_state_on);
					ssp_setStateByIndex(&stateSystem, 8, srp_state_off);
					ssp_setStateByIndex(&stateSystem, 9, srp_state_off);
					break;
					
		case 0x8C:	hdmi_setMatrix('B',3); 
					ssp_setStateByIndex(&stateSystem, 6, srp_state_off);
					ssp_setStateByIndex(&stateSystem, 7, srp_state_off);
					ssp_setStateByIndex(&stateSystem, 8, srp_state_on);
					ssp_setStateByIndex(&stateSystem, 9, srp_state_off);
					break;
					
		case 0x8D:	hdmi_setMatrix('B',4); 
					ssp_setStateByIndex(&stateSystem, 6, srp_state_off);
					ssp_setStateByIndex(&stateSystem, 7, srp_state_off);
					ssp_setStateByIndex(&stateSystem, 8, srp_state_off);
					ssp_setStateByIndex(&stateSystem, 9, srp_state_on);
					break;
					
		case 0x8E:	hdmi_turnOff();
		
					ssp_setStateByIndex(&stateSystem, 2, srp_state_off);
					ssp_setStateByIndex(&stateSystem, 3, srp_state_off);
					ssp_setStateByIndex(&stateSystem, 4, srp_state_off);
					ssp_setStateByIndex(&stateSystem, 5, srp_state_off);
					
					ssp_setStateByIndex(&stateSystem, 6, srp_state_off);
					ssp_setStateByIndex(&stateSystem, 7, srp_state_off);
					ssp_setStateByIndex(&stateSystem, 8, srp_state_off);
					ssp_setStateByIndex(&stateSystem, 9, srp_state_off);
					break;
	}
}


int main()
{
	// App init Code
	if(kernel.kernelSignals->initApp)
	{
		Reset_Handler();
		
		bsRelay_init(&relay1, IO_D06, IO_D07);
		hdmi_init(IO_D00);
		projector_init();
		
		pin_enableOutput(IO_D15);
		pin_setOutput(IO_D15, true);
		
		hdmi_matrix(0x8E);
	
		pin_enableOutput(IO_D01);
		pin_setOutput(IO_D01, false);
		
		tsp_initialize(&triggerSystem);
		esp_initialize(&eventSystem);
		ssp_initialize(&stateSystem);
		
		kernel.appSignals->appReady = true;	
	}

	// Main code here
	if(kernel.appSignals->appReady == true)
	{
		handleReceive();
		
		tsp_mainHandler(&triggerSystem);
		esp_mainHandler(&eventSystem);
		ssp_mainHandler(&stateSystem);
		
		bsRelay_handler(&relay1);
		
		hdmi_handler();
		projector_handler();
		
		if(projector_getSwitchOn()) 
		{
			kernel.log.message("Power On");
			bsRelay_set(&relay1,true);
		}
		if(projector_getSwitchOff())
		{
			kernel.log.message("Power Off");
			bsRelay_set(&relay1,false);
		}
		
		if(projector_getOnOff() == projector_on) mainPowerState = srp_state_on;
		else if(projector_getOnOff() == projector_off) mainPowerState = srp_state_off;
		else mainPowerState = ssp_state_transitioning;
		
		if(projector_getBlank()) projectorBlackout = srp_state_on;
		else projectorBlackout = srp_state_off;
		
		if(hdmi_getSwitchOn()) 
		{
			esp_setStateByIndex(&eventSystem, 0, true);
			pin_setOutput(IO_D01, true);
		}
		else if(hdmi_getSwitchOff()) 
		{
			esp_setStateByIndex(&eventSystem, 0, false);
			pin_setOutput(IO_D01, false);
		}
	}
	
	// App deinit code
    if(kernel.kernelSignals->shutdownApp)
    {
		kernel.appSignals->shutdownReady = true;
    }
}