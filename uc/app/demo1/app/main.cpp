/*
 * app.cpp
 *
 * Created: 20.04.2019 20:05:30
 * Author : Christian
 */ 

#include "sam.h"
#include "common/kernel.h"
#include "common/io_same51jx.h"

#include "driver/SAMx5x/genericClockController.h"

#include "protocol/triggerSystemProtocol.h"
#include "protocol/eventSystemProtocol.h"
#include "protocol/stateSystemProtocol.h"
#include "protocol/valueSystemProtocol.h"

#include "utility/edgeDetect.h"

#define LED1 IO_D03
#define LED2 IO_D02
#define LED3 IO_D01
#define LED4 IO_D00

#define BUTTON1 IO_D07
#define BUTTON2 IO_D06
#define BUTTON3 IO_D05
#define BUTTON4 IO_D04

int main(void);
kernel_t kernel __attribute__((section(".kernelCall")));

__attribute__((section(".appHeader"))) applicationHeader_t appHead ={
/*appCRC	 */ 0xAABBCCDD, // Will be written by Bootload tool
/*appSize	 */ 0xEEFF0000, // Will be written by Bootload tool
/*appRevMaj	 */ 0x01,
/*appRevMin	 */ 0x00,
/*appName[60]*/ "Demo 1",
/*main		 */ main
};


//**** Trigger Configuration ******************************************************************************************

void trigger1Action(uint16_t triggerChannelNumber);

const tsp_triggerSignal_t triggerSignals[] = {
	{0x0100, "Button 1 Signal"}
};
#define triggerSignalListSize (sizeof(triggerSignals)/sizeof(tsp_triggerSignal_t))

const tsp_triggerSlot_t triggerSlots[] = {
	{0x0100, "LED 1 Slot", trigger1Action}
};
#define triggerSlotListSize (sizeof(triggerSlots)/sizeof(tsp_triggerSlot_t))

tsp_itemState_t triggerSignalStateList[triggerSignalListSize];
tsp_itemState_t triggerSlotStateList[triggerSlotListSize];

const triggerSystemProtocol_t triggerSystem = {
	.signals = triggerSignals,
	.signalSize = triggerSignalListSize,
	.slots = triggerSlots,
	.slotSize = triggerSlotListSize,
	._signalState = triggerSignalStateList,
	._slotState = triggerSlotStateList
};


//**** Event Configuration ********************************************************************************************

void led3Event(uint16_t eventChannelNumber, bool state)
{
	pin_setOutput(LED3, !state);
}

void led4Event(uint16_t eventChannelNumber, bool state)
{
	pin_setOutput(LED4, !state);
}

const esp_eventSignal_t eventSignal[] = {
	{0x0100, "Button 3 Signal", 1000},
	{0x0101, "Button 4 Signal", 80}
};
#define eventSignalListSize (sizeof(eventSignal)/sizeof(esp_eventSignal_t))

const esp_eventSlot_t eventSlot[] = {
	{0x0100, "LED 3 Slot", 2000, led3Event},
	{0x0101, "LED 4 Slot", 100,  led4Event}
};
#define eventSlotListSize (sizeof(eventSlot)/sizeof(esp_eventSlot_t))

esp_itemState_t eventSignalStatusList[eventSignalListSize];
esp_itemState_t eventSlotStatusList[eventSlotListSize];

const eventSystemProtocol_t eventSystem = {
	.signals = eventSignal,
	.signalSize = eventSignalListSize,
	.slots = eventSlot,
	.slotSize = eventSlotListSize,
	._signalState = eventSignalStatusList,
	._slotState = eventSlotStatusList
};

//**** State Configuration ********************************************************************************************

void led2Action(uint16_t stateChannelNumber, ssp_state_t state)
{
	if(state == srp_state_on)pin_setOutput(LED2, false);
	else pin_setOutput(LED2, true);
}

const ssp_stateSignal_t stateReportSignal[] = {
	{0x0100, "Button 3 Signal", 10}
};
#define stateReportSignalListSize (sizeof(stateReportSignal)/sizeof(ssp_stateSignal_t))

const ssp_stateSlot_t stateReportSlots[] = {
	{0x0100, "LED 3 Slot", 10, led2Action}
};
#define stateReportSlotListSize (sizeof(stateReportSlots)/sizeof(ssp_stateSlot_t))

ssp_itemState_t stateReportSignalStatusList[stateReportSignalListSize];
ssp_itemState_t stateReportSlotStatusList[stateReportSlotListSize];

const stateSystemProtocol_t stateReportSystem = {
	.signals = stateReportSignal,
	.signalSize = stateReportSignalListSize,
	.slots = stateReportSlots,
	.slotSize = stateReportSlotListSize,
	._signalState = stateReportSignalStatusList,
	._slotState = stateReportSlotStatusList
};

bool state1 = false;
void trigger1Action(uint16_t triggerChannelNumber)
{
	state1 = !state1;
	if(state1) ssp_setStateByIndex(&stateReportSystem, 0, srp_state_on);
	else ssp_setStateByIndex(&stateReportSystem, 0, srp_state_off);
}


//**** Value Configuration ********************************************************************************************

const vsp_valueSignal_t valueReportSignal[] = {
	{0x0100, "Value Signal", 10, true, {.Number = 10.0}, {.Number = 20.0}, vsp_uom_number, nullptr},
	{0x0101, "Value Signal 2", 10, false, {.Long = 0}, {.Long = 255}, vsp_uom_long, nullptr}
};
#define valueReportSignalListSize (sizeof(valueReportSignal)/sizeof(vsp_valueSignal_t))

const vsp_valueSlot_t valueReportSlots[] = {
	{0x0100, "Value Slot", 10, nullptr}
};
#define valueReportSlotListSize (sizeof(valueReportSlots)/sizeof(vsp_valueSlot_t))

vsp_itemState_t valueReportSignalStatusList[valueReportSignalListSize];
vsp_itemState_t valueReportSlotStatusList[valueReportSlotListSize];

const valueSystemProtocol_t valueReportSystem = {
	.signals = valueReportSignal,
	.signalSize = valueReportSignalListSize,
	.slots = valueReportSlots,
	.slotSize = valueReportSlotListSize,
	._signalState = valueReportSignalStatusList,
	._slotState = valueReportSlotStatusList
};

//**** onReceive Configuration ****************************************************************************************

void handleReceive(void)
{
	bus_rxMessage_t message;
	if(!kernel.bus.getReceivedMessage(&message)) return;
	
	bool processed = false;
	switch(message.protocol){
		case busProtocol_triggerSystemProtocol:	processed = tsp_receiveHandler(&triggerSystem, &message); break;
		case busProtocol_eventSystemProtocol:	processed = esp_receiveHandler(&eventSystem, &message); break;
		case busProtocol_stateSystemProtocol:	processed = ssp_receiveHandler(&stateReportSystem, &message); break;
		case busProtocol_valueSystemProtocol:	processed = vsp_receiveHandler(&valueReportSystem, &message); break;
	}
	kernel.bus.receivedProcessed(processed);
}


edgeDetect_t button[4];
bool led3;
bool led4;

uint32_t valueTimer;
vsp_valueData_t val1;
int main(void)
{
	// App Init Code
	if(kernel.kernelSignals->initApp)
	{
		Reset_Handler();
		
		pin_setOutput(LED1, true);
		pin_setOutput(LED2, true);
		pin_setOutput(LED3, true);
		pin_setOutput(LED4, true);
					
		pin_enableOutput(LED1);
		pin_enableOutput(LED2);
		pin_enableOutput(LED3);
		pin_enableOutput(LED4);
		
		pin_enableInput(BUTTON1);
		pin_enableInput(BUTTON2);
		pin_enableInput(BUTTON3);
		pin_enableInput(BUTTON4);
		
		ed_init(&button[0], pin_getInput(BUTTON1));
		ed_init(&button[1], pin_getInput(BUTTON2));
		ed_init(&button[2], pin_getInput(BUTTON3));
		ed_init(&button[3], pin_getInput(BUTTON4));
		
		tsp_initialize(&triggerSystem);
		esp_initialize(&eventSystem);
		ssp_initialize(&stateReportSystem);
		vsp_initialize(&valueReportSystem);
		
		kernel.tickTimer.reset(&valueTimer);
		val1.Number = 0;
		
		kernel.appSignals->appReady = true;
	}

//----- Main -------------------------------------------------------------------------------------------------	
	
	if(kernel.appSignals->appReady == true)
	{	
		handleReceive();
		
		tsp_mainHandler(&triggerSystem);
		esp_mainHandler(&eventSystem);
		ssp_mainHandler(&stateReportSystem);
		vsp_mainHandler(&valueReportSystem);
		
		if(ed_onFalling(&button[0], pin_getInput(BUTTON1))){
			uint8_t triggerList[] = {0};
			tsp_sendTriggersByIndex(&triggerSystem, &triggerList[0], sizeof(triggerList));
		}
		
		if(ed_onFalling(&button[1], pin_getInput(BUTTON2))){

		}
		
		if(ed_onFalling(&button[2], pin_getInput(BUTTON3))){
			led3 = !led3;
			esp_setStateByIndex(&eventSystem, 0, led3);
			pin_setOutput(LED3, !led3);
		}
		
		if(ed_onFalling(&button[3], pin_getInput(BUTTON4))){
			led4 = !led4;
			esp_setStateByIndex(&eventSystem, 1, led4);
			pin_setOutput(LED4, !led4);
		}
		
		ssp_state_t state = ssp_getStateByIndex(&stateReportSystem, 0);
		if(state == srp_state_on)pin_setOutput(LED1, !true);
		else pin_setOutput(LED1, !false);
		
		if(kernel.tickTimer.delay1ms(&valueTimer, 1000)){
			val1.Number += 1.7;
			vsp_sendValueReportByChannel(&valueReportSystem, 0x0100, val1);
		}
	}
	
	// App deinit code
    if(kernel.kernelSignals->shutdownApp){
		kernel.appSignals->shutdownReady = true;
    }	
}