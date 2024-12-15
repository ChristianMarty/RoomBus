/*
 * app.cpp
 *
 * Created: 20.04.2019 20:05:30
 * Author : Christian
 */ 


#include "sam.h"
#include "kernel/kernel.h"
#include "kernel/busController_IO.h"

#include "Raumsteuerung/bistableRelay.h"

#include "protocol/stateReportProtocol.h"
#include "protocol/triggerProtocol.h"
#include "protocol/eventProtocol.h"

#include "hdmi_switch.h"
#include "projector.h"

int main(const kernel_t *kernel);
bool onReceive(const kernel_t *kernel, uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size);
kernel_t const *_kernel;

__attribute__((section(".appHeader"))) appHead_t appHead ={
/*appCRC	 */ 0xAABBCCDD, // Will be written by Bootload tool
/*appSize	 */ 0xEEFF0000, // Will be written by Bootload tool
/*appRevMaj	 */ 0x01,
/*appRevMin	 */ 0x00,
/*appName[60]*/ "Projector / HDMI-Switch Controller",
/*main		 */ main,
/*onRx		 */ onReceive
};

#define OFF_DELAY 30000
#define STATE_SEND_INTERVAL 60000
#define POWER_DELIVERY_EVENT_TIMEOUT 25000

uint32_t offTimer;
uint32_t sendStateTimer;
uint32_t hdmiPowerEventTimer;

uint8_t ememData[256];

srp_state_t mainPowerState;
srp_state_t mainPowerStateOld;
srp_state_t projectorBlackout;
srp_state_t projectorBlackoutOld;
srp_state_t hdmiSwitchA[4];
srp_state_t hdmiSwitchB[4];

bsRelay_t relay1;

bool sendStateUpdate = true;

const srp_stateReportChannel_t stateReportList[] = {
	{0x80, "Projector Power State", &mainPowerState},
	{0x81, "Projector Blackout State", &projectorBlackout},
			
	{0x82, "HDMI Out A - In 1", &hdmiSwitchA[0]},
	{0x83, "HDMI Out A - In 2", &hdmiSwitchA[1]},
	{0x84, "HDMI Out A - In 3", &hdmiSwitchA[2]},
	{0x85, "HDMI Out A - In 4", &hdmiSwitchA[3]},
		
	{0x86, "HDMI Out B - In 1", &hdmiSwitchB[0]},
	{0x87, "HDMI Out B - In 2", &hdmiSwitchB[1]},
	{0x88, "HDMI Out B - In 3", &hdmiSwitchB[2]},
	{0x89, "HDMI Out B - In 4", &hdmiSwitchB[3]}
};
#define stateReportListSize (sizeof(stateReportList)/sizeof(srp_stateReportChannel_t))


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
		
	{0x06, "HDMI Out A - In 1", hdmi_matrix},
	{0x87, "HDMI Out A - In 2", hdmi_matrix},
	{0x88, "HDMI Out A - In 3", hdmi_matrix},
	{0x89, "HDMI Out A - In 4", hdmi_matrix},
	
	{0x8A, "HDMI Out B - In 1", hdmi_matrix},
	{0x8B, "HDMI Out B - In 2", hdmi_matrix},
	{0x8C, "HDMI Out B - In 3", hdmi_matrix},
	{0x8D, "HDMI Out B - In 4", hdmi_matrix},
	{0x8E, "HDMI Off", hdmi_matrix}, 
};
#define triggerSlotListSize (sizeof(triggerSlotList)/sizeof(tsp_triggerSlot_t))


const esp_eventSignal_t eventSignals[] = {
	{0x82, "A/V Aux Power", POWER_DELIVERY_EVENT_TIMEOUT}
};
#define eventSignalListSize (sizeof(eventSignals)/sizeof(esp_eventSignal_t))

esp_itemState_t _eventSignalStatus[eventSignalListSize];
const eventSystemProtocol_t eventSystem = {
	.signals = eventSignals,
	.signalSize = (sizeof(eventSignals)/sizeof(esp_eventSignal_t)),
	
	.slots = nullptr,
	.slotSize = 0,
	
	._signalState = &_eventSignalStatus[0],
	._slotState = nullptr
};

const stateReportProtocol_t stateReport = {
	.states = nullptr,
	.stateSize = 0,
	.channels = stateReportList,
	.channelSize = stateReportListSize
};

tsp_itemStatus_t _slotsStatus[triggerSlotListSize];
const triggerSystemProtocol_t triggerSystem = {
	.signals = nullptr,
	.signalSize = 0,
	
	.slots = triggerSlotList,
	.slotSize = triggerSlotListSize,
	
	._signalStatus = nullptr,
	._slotStatus = &_slotsStatus[0]
};

bool onReceive(const kernel_t *kernel, uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size)
{	
	switch(protocol)
	{
		case busProtocol_triggerProtocol:		return tsp_receiveHandler(kernel, &triggerSystem, sourceAddress, command,data, size);
		case busProtocol_stateReportProtocol:	return srp_receiveHandler(kernel, &stateReport, sourceAddress, command,data, size);
		case busProtocol_eventProtocol:			return esp_receiveHandler(kernel, &eventSystem, sourceAddress, command, data, size);
	}
	return false;
}

void tsp_onStateChange_callback(const kernel_t *kernel)
{
	
}

void projector_pwr(uint16_t triggerChannelNumber)
{
	switch(triggerChannelNumber)
	{
		case  0:	projector_onOff(true);
					break;
					
		case  1:	projector_onOff(false);
					break;
					
		case  2:	if(mainPowerState == srp_state_off)projector_onOff(true);
					else if(mainPowerState == srp_state_on)projector_onOff(false);
					break;
	}
	
	sendStateUpdate = true;
					
}

void projector_blackout(uint16_t triggerChannelNumber)
{
	switch(triggerChannelNumber)
	{
		case  3:	projector_blank(true);
					break;
		
		case  4:	projector_blank(false);
					break;
		
		case  5:	if(projectorBlackout == srp_state_off)projector_blank(true);
					else if(projectorBlackout == srp_state_on)projector_blank(false);
					break;
	}
	
	sendStateUpdate = true;
}

void hdmi_matrix(uint16_t triggerChannelNumber)
{
	switch(triggerChannelNumber)
	{
		case  6:	hdmi_setMatrix('A',1); 
					hdmiSwitchA[0] = srp_state_on;
					hdmiSwitchA[1] = srp_state_off;
					hdmiSwitchA[2] = srp_state_off;
					hdmiSwitchA[3] = srp_state_off;
					break;
					
		case  7:	hdmi_setMatrix('A',2); 
					hdmiSwitchA[0] = srp_state_off;
					hdmiSwitchA[1] = srp_state_on;
					hdmiSwitchA[2] = srp_state_off;
					hdmiSwitchA[3] = srp_state_off;
					break;
					
		case  8:	hdmi_setMatrix('A',3); 
					hdmiSwitchA[0] = srp_state_off;
					hdmiSwitchA[1] = srp_state_off;
					hdmiSwitchA[2] = srp_state_on;
					hdmiSwitchA[3] = srp_state_off;
					break;
					
		case  9:	hdmi_setMatrix('A',4);
					hdmiSwitchA[0] = srp_state_off;
					hdmiSwitchA[1] = srp_state_off;
					hdmiSwitchA[2] = srp_state_off;
					hdmiSwitchA[3] = srp_state_on;
					break;
		
		case  10:	hdmi_setMatrix('B',1); 
					hdmiSwitchB[0] = srp_state_on;
					hdmiSwitchB[1] = srp_state_off;
					hdmiSwitchB[2] = srp_state_off;
					hdmiSwitchB[3] = srp_state_off;
					break;
					
		case  11:	hdmi_setMatrix('B',2); 
					hdmiSwitchB[0] = srp_state_off;
					hdmiSwitchB[1] = srp_state_on;
					hdmiSwitchB[2] = srp_state_off;
					hdmiSwitchB[3] = srp_state_off;
					break;
					
		case 12:	hdmi_setMatrix('B',3); 
					hdmiSwitchB[0] = srp_state_off;
					hdmiSwitchB[1] = srp_state_off;
					hdmiSwitchB[2] = srp_state_on;
					hdmiSwitchB[3] = srp_state_off;
					break;
					
		case 13:	hdmi_setMatrix('B',4); 
					hdmiSwitchB[0] = srp_state_off;
					hdmiSwitchB[1] = srp_state_off;
					hdmiSwitchB[2] = srp_state_off;
					hdmiSwitchB[3] = srp_state_on;
					break;
					
		case 14:	hdmi_turnOff(_kernel);
		
					hdmiSwitchA[0] = srp_state_off;
					hdmiSwitchA[1] = srp_state_off;
					hdmiSwitchA[2] = srp_state_off;
					hdmiSwitchA[3] = srp_state_off;
					
					hdmiSwitchB[0] = srp_state_off;
					hdmiSwitchB[1] = srp_state_off;
					hdmiSwitchB[2] = srp_state_off;
					hdmiSwitchB[3] = srp_state_off;
					break;
	}
	
	sendStateUpdate = true;
}



int main(const kernel_t *kernel)
{
	_kernel = kernel;
	
	// App init Code
	if(kernel->kernelSignals->initApp)
	{
		Reset_Handler();
		
		bsRelay_init(&relay1, IO_D06, IO_D07);
		hdmi_init(kernel, IO_D00);
		projector_init(kernel);
		
		pin_enableOutput(IO_D15);
		pin_setOutput(IO_D15,true);
		
		//kernel->eemem.read(&ememData[0]);
		kernel->tickTimer.reset(&offTimer);
		kernel->tickTimer.reset(&sendStateTimer);
		kernel->tickTimer.reset(&hdmiPowerEventTimer);
		
		mainPowerStateOld = mainPowerState;
		projectorBlackoutOld = projectorBlackout;
		
		hdmi_matrix(14);
	
		pin_enableOutput(IO_D01);
		pin_setOutput(IO_D01,false);
		
		kernel->appSignals->appReady = true;	
	}

	// Main code here
	if(kernel->appSignals->appReady == true)
	{
		tsp_mainHandler(kernel, &triggerSystem);
		
		bsRelay_handler(kernel, &relay1);
		
		hdmi_handler(kernel);
		
		projector_handler(kernel);
		
		if(projector_getSwitchOn()) 
		{
			kernel->log.message("Power On");
			bsRelay_set(&relay1,true);
		}
		if(projector_getSwitchOff())
		{
			kernel->log.message("Power Off");
			bsRelay_set(&relay1,false);
		}
		
		if(projector_getOnOff() == projector_on) mainPowerState = srp_state_on;
		else if(projector_getOnOff() == projector_off) mainPowerState = srp_state_off;
		else mainPowerState = srp_state_transitioning;
		
		if(projector_getBlank()) projectorBlackout = srp_state_on;
		else projectorBlackout = srp_state_off;
		
		if(mainPowerStateOld != mainPowerState)
		{
			sendStateUpdate = true;
			mainPowerStateOld = mainPowerState;
		}
		
		if(projectorBlackoutOld != projectorBlackout)
		{
			sendStateUpdate = true;
			projectorBlackoutOld = projectorBlackout;
		}
		
		if(kernel->tickTimer.delay1ms(&sendStateTimer, STATE_SEND_INTERVAL)) sendStateUpdate = true;
		
		if(sendStateUpdate)
		{
			srp_sendGroupReport(kernel, &stateReport);
			sendStateUpdate = false;
			kernel->tickTimer.reset(&sendStateTimer);
		}
		
		if(hdmi_getSwitchOn(kernel)) 
		{
			esp_setStateByIndex(&eventSystem, 0, true);
			esp_sendEventByIndex(kernel, &eventSystem, 0);
			pin_setOutput(IO_D01,true);
		}
		else if(hdmi_getSwitchOff(kernel)) 
		{
			esp_setStateByIndex(&eventSystem, 0, false);
			pin_setOutput(IO_D01,false);
		}
	}
	
	// App deinit code
    if(kernel->kernelSignals->shutdownApp)
    {
		//kernel->eemem.write(&ememData[0]);
		kernel->appSignals->shutdownReady = true;
    }
}