/*
 * app.cpp
 *
 * Created: 20.04.2019 20:05:30
 * Author : Christian
 */ 


#include "sam.h"
#include "kernel/kernel.h"
#include "kernel/busController_IO.h"

#include "Raumsteuerung/stateReportProtocol.h"
#include "Raumsteuerung/triggerProtocol.h"
#include "Raumsteuerung/eventProtocol.h"
#include "Raumsteuerung/serialBridgProtocol.h"

#include "LightBus.h"

#include "Raumsteuerung/lightMacro.h"

int main(const kernel_t *kernel);
void onReceive(const kernel_t *kernel, uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size);
kernel_t const *_kernel;

__attribute__((section(".appHeader"))) appHead_t appHead ={
/*appCRC	 */ 0xAABBCCDD, // Will be written by Bootload tool
/*appSize	 */ 0xEEFF0000, // Will be written by Bootload tool
/*appRevMaj	 */ 0x01,
/*appRevMin	 */ 0x00,
/*appName[60]*/ "LightBus Controller",
/*main		 */ main,
/*onRx		 */ onReceive
};

#define POWER_DELIVERY_EVENT_TIMEOUT 12000
#define OFF_TIMEOUT 30000

bool lightPowerOn = false;
bool lightPowerOnOld = false;
srp_state_t lightPowerState;

uint32_t powerEventTimer;
uint32_t offTimeoutTimer;

uint8_t ememData[256];

const srp_stateReportChannel_t stateReportList[] = {
	{0x00, "Power On", &lightPowerState}
};

void light_scene(uint8_t triggerChannelNumber);

const tsp_triggerSlot_t triggerSlotList[] = {
	{UNFILTERED, 0x00, "Off", light_scene},
	{UNFILTERED, 0x01, "Day", light_scene}

};
#define  triggerSlotListSize (sizeof(triggerSlotList)/sizeof(tsp_triggerSlot_t))

esp_eventState_t eventState[1];
const esp_eventSignal_t eventSignals[] = {
	{0x02, 0x01, "12V Power Supply"}
};

const eventSystemProtocol_t eventSystem = {
	.signals = eventSignals,
	.signalSize = (sizeof(eventSignals)/sizeof(esp_eventSignal_t)),
	.slots = nullptr,
	.slotSize = 0
};

const stateReportProtocol_t stateReport = {
	.states = nullptr,
	.stateSize = 0,
	.channels = stateReportList,
	.channelSize = (sizeof(stateReportList)/sizeof(srp_stateReportChannel_t))
};

tsp_itemStatus_t _slotsStatus[triggerSlotListSize];
const triggerSlotProtocol_t triggerSystem = {
	.signals = nullptr,
	.signalSize = 0,
	.slots = triggerSlotList,
	.slotSize = triggerSlotListSize,
	._slotsStatus = &_slotsStatus[0]
};

void sbp_transmit_port0(const uint8_t *data, uint8_t size);

const sbp_port_t serialPortList[] = {
	{sbp_type_uart, sbp_transmit_port0}
};
#define serialPortListSize (sizeof(serialPortList)/sizeof(sbp_port_t))

void sbp_transmit_port0(const uint8_t *data, uint8_t size)
{
	lightBus_send(_kernel,data,size);
	
	_kernel->tickTimer.reset(&offTimeoutTimer);
	lightPowerOn = true;
}

const serialBridgeProtocol_t serialBridge = {
	.ports = serialPortList,
	.portSize = serialPortListSize
};

void onReceive(const kernel_t *kernel, uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size)
{	
	switch(protocol)
	{
		case busProtocol_triggerProtocol:		tsp_receiveHandler(kernel, sourceAddress, command,data, size, &triggerSystem); break;
		case busProtocol_stateReportProtocol:	srp_receiveHandler(kernel,sourceAddress,command,data,size, &stateReport); break;
		case busProtocol_eventProtocol:			esp_receiveHandler(kernel, sourceAddress, command, data, size, &eventSystem); break;
		case busProtocol_serialBridgeProtocol:	sbp_receiveHandler(kernel, sourceAddress, command, data, size, &serialBridge); break;
	}
}

void tsp_onTrigger_callback(const kernel_t *kernel, uint8_t sourceAddress, uint8_t triggerChannelNumber)
{
	tsp_triggerAction(kernel, triggerChannelNumber, triggerSlotList, triggerSlotListSize);
}

void tsp_onStateChange_callback(const kernel_t *kernel)
{
	
}


uint16_t dimmer_value[4] = {0x0000, 0x0000, 0x0000, 0x0000};
void lightMacroParser_execute(uint8_t *registers, uint16_t registersSize)
{
	lightBus_fade_t fade ={
		.wfe = 0, 
		.fadetime = 0
	};
		
	lightBus_set(_kernel,0x0F,fade,dimmer_value[0],fade,dimmer_value[1],fade,dimmer_value[2],fade,dimmer_value[3]);
	
	_kernel->tickTimer.reset(&offTimeoutTimer);
	lightPowerOn = true;
}

uint8_t light_off[] = {0x11,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00};
uint8_t light_on[] =  {
	0x11,0x00,0x08,0xFF,0x08,0xFF,0x08,0xFF,0x08,0xFF,0x08,
	0x03,
	0x01,0x04,0x00,
	0x11,0x00,0x08,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0x03,
	0x00
};
	
	
lightMacroParser_t on = {
	.script = &light_on[0],
	.programCounter = 0,
	.registers = (uint8_t*)&dimmer_value[0],
	.registersSize = sizeof(dimmer_value),
	.sleepTime = 0,
	.sleepTimer = 0,
	.sleep = false,
	.loop = false,
	.run = false,
	.execute = lightMacroParser_execute,
	.error = LMPE_NONE
};

lightMacroParser_t off = {
	.script = &light_off[0],
	.programCounter = 0,
	.registers = (uint8_t*)&dimmer_value[0],
	.registersSize = sizeof(dimmer_value),
	.sleepTime = 0,
	.sleepTimer = 0,
	.sleep = false,
	.loop = false,
	.run = false,
	.execute = lightMacroParser_execute,
	.error = LMPE_NONE
};

void light_scene(uint8_t triggerChannelNumber)
{
	switch(triggerChannelNumber)
	{
		case 0: off.run = true;
				break;
				
		case 1: on.run = true;
				break;
	}
	
}

int main(const kernel_t *kernel)
{
	_kernel = kernel;
	
	// App init Code
	if(kernel->kernelSignals->initApp)
	{
		Reset_Handler();
		
		lightBus_init(kernel);
	
		//kernel->eemem_read(&ememData[0]);
		kernel->tickTimer.reset(&powerEventTimer);
		kernel->tickTimer.reset(&offTimeoutTimer);
		
		kernel->appSignals->appReady = true;
		
	}

	// Main code here
	if(kernel->appSignals->appReady == true)
	{
		lightMacro_run(kernel, &on);
		lightMacro_run(kernel, &off);
		
		lightBus_handler(kernel);
		
		esp_mainHandler(kernel, &eventSystem);
		tsp_mainHandler(kernel, &triggerSystem);
		
		if(kernel->tickTimer.delay1ms(&offTimeoutTimer, OFF_TIMEOUT)) lightPowerOn = false;
		
		if(kernel->tickTimer.delay1ms(&powerEventTimer, POWER_DELIVERY_EVENT_TIMEOUT) && lightPowerOn) esp_sendEventSignal(kernel, &eventSignals[0]);
		
		if(lightPowerOnOld == false && lightPowerOn == true) esp_sendEventSignal(kernel, &eventSignals[0]);
		
		lightPowerOnOld = lightPowerOn;
		
	}
	
	// App deinit code
    if(kernel->kernelSignals->shutdownApp)
    {
		//kernel->eemem_write(&ememData[0]);
		kernel->appSignals->shutdownReady = true;
    }
}