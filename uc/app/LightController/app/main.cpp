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

#include "addOn/lightBusModul.h"
#include "Raumsteuerung/lightBus.h"

#include "Raumsteuerung/lightMacro.h"

int main(const kernel_t *kernel);
void onReceive(const kernel_t *kernel, uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size);
kernel_t const *_kernel;

__attribute__((section(".appHeader"))) appHead_t appHead ={
/*appCRC	 */ 0xAABBCCDD, // Will be written by Bootload tool
/*appSize	 */ 0xEEFF0000, // Will be written by Bootload tool
/*appRevMaj	 */ 0x01,
/*appRevMin	 */ 0x00,
/*appName[60]*/ "Light Controller",
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
	lightBusModul_send(_kernel, lightBusModul_channel_t::lightBusModul_channel_1, data, size);
	
	_kernel->tickTimer.reset(&offTimeoutTimer);
	lightPowerOn = true;
}

void lightBusModul_onReceive(lightBusModul_channel_t channel, const uint8_t *data, uint8_t size, bool error)
{
	sbp_status_t status = sbp_status_t::sbp_state_ok;
	if(error) status = sbp_status_t::sbp_state_crcError;
	
	if(channel == lightBusModul_channel_t::lightBusModul_channel_1) sbp_sendData(_kernel,0x7E,0,status,data,size);
	else if(channel == lightBusModul_channel_t::lightBusModul_channel_2) sbp_sendData(_kernel,0x7E,1,status,data,size);
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
		
	lightBusModul_set(_kernel, lightBusModul_channel_t::lightBusModul_channel_1, 0x0F,fade,dimmer_value[0],fade,dimmer_value[1],fade,dimmer_value[2],fade,dimmer_value[3]);
	
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
/*
extern uint32_t _sfixed;
extern uint32_t _efixed;
extern uint32_t _sstack;
extern uint32_t _estack;

extern uint32_t _etext;
extern uint32_t _srelocate;
extern uint32_t _erelocate;
extern uint32_t _szero;
extern uint32_t _ezero;

void app_init(void)
{
	uint32_t *pSrc, *pDest;

	// Initialize the relocate segment 
	pSrc = &_etext;
	pDest = &_srelocate;

	if (pSrc != pDest) {
		for (; pDest < &_erelocate;) {
			*pDest++ = *pSrc++;
		}
	}

	// Clear the zero segment 
	for (pDest = &_szero; pDest < &_ezero;) {
		*pDest++ = 0;
	}
}

//*/

uint8_t dmx_txBuffer[514];

lightBusModul_t lightBusModul = {
	.dmx_enable = false,
	.dmx_data = dmx_txBuffer,
	.dmx_data_size = sizeof(dmx_txBuffer),
	.lightBusModul_onReceive = lightBusModul_onReceive
};


int main(const kernel_t *kernel)
{
	_kernel = kernel;
	
	// App init Code
	if(kernel->kernelSignals->initApp)
	{
		//app_init();
		Reset_Handler();
		
		lightBusModul_init(kernel, &lightBusModul);
		
		//kernel->eemem_read(&ememData[0]);
		kernel->tickTimer.reset(&powerEventTimer);
		kernel->tickTimer.reset(&offTimeoutTimer);
		
		kernel->appSignals->appReady = true;
	}

	// Main code here
	if(kernel->appSignals->appReady == true)
	{
		lightBusModul_handler(kernel, &lightBusModul);
		
		if(lightBusModul_uart_dataAvailable(kernel, &lightBusModul))
		{
			uint8_t *uart_rxData = lightBusModul_uart_data(kernel, &lightBusModul);
			
			dmx_txBuffer[2] = uart_rxData[0];
			dmx_txBuffer[3] = uart_rxData[1];
			dmx_txBuffer[4] = uart_rxData[2];
			dmx_txBuffer[5] = uart_rxData[3];
			dmx_txBuffer[6] = uart_rxData[4];
			
			lightBusModul_uart_dataClear(kernel, &lightBusModul);
			
			lightBus_fade_t fade = {.wfe = 0, .fadetime = 1};
				
			lightBusModul_set(kernel, lightBusModul_channel_t::lightBusModul_channel_1, 0x0F, fade, uart_rxData[0]<<8, fade, uart_rxData[1]<<8, fade, uart_rxData[2]<<8, fade, 0);
			
			kernel->tickTimer.reset(&offTimeoutTimer);
			lightPowerOn = true;
			
		}
		
		lightMacro_run(kernel, &on);
		lightMacro_run(kernel, &off);
		
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