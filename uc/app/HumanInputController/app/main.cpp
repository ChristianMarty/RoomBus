//**********************************************************************************************************************
// FileName : main.cpp
// FilePath :
// Author   : Christian Marty
// Date		: 20.04.2019
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#include "main.h"

#include "common/kernel.h"

#include "protocol/triggerProtocol.h"
#include "protocol/valueReportProtocol.h"
#include "protocol/stateReportProtocol.h"

#include "driver/SAMx5x/pin.h"
#include "driver/SAMx5x/i2cMaster.h"

#include "utility/string.h"
#include "utility/edgeDetect.h"

#include "addOn/i2cModul_rev2.h"
#include "addOn/encoderSwitch.h"
#include "addOn/buttonSwitch.h"

int main(void);
bool onReceive(uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size);
kernel_t kernel __attribute__((section(".kernelCall")));

__attribute__((section(".appHeader"))) appHead_t appHead ={
/*appCRC	 */ 0xAABBCCDD, // Will be written by Bootload tool
/*appSize	 */ 0xEEFF0000, // Will be written by Bootload tool
/*appRevMaj	 */ 0x01,
/*appRevMin	 */ 0x00,
/*appName[60]*/ "Human Input Controller",
/*main		 */ main,
/*onRx		 */ onReceive
};


bool update = false;
bool rxVal = false;

i2cMaster_c i2c;

edgeDetect_t buttonEdgeDetect ={.oldState = false};


void i2cMbInterrupt(void)
{
	i2c.MbInterrupt();
}

void i2cSbInterrupt(void)
{
	i2c.SbInterrupt();
}

void i2cError(void)
{
	i2c.ErrorInterrupt();
}
	
	
#define SLOT_TIMEOUT 10
	
//**** State Configuration ********************************************************************************************
void srp_stateChange(uint16_t valueChastateChannelNumbernnelNumber, srp_state_t state)
{
	update = true;
}

const srp_stateSlot_t stateReportSlot[] = {
	{0x01, "Button 1", SLOT_TIMEOUT, srp_stateChange},
	{0x02, "Button 2", SLOT_TIMEOUT, srp_stateChange},
	{0x03, "Button 3", SLOT_TIMEOUT, srp_stateChange},
	{0x04, "Button 4", SLOT_TIMEOUT, srp_stateChange},
	{0x05, "Button 5", SLOT_TIMEOUT, srp_stateChange},
	{0x06, "Button 6", SLOT_TIMEOUT, srp_stateChange},
	{0x12, "Rotary Button", SLOT_TIMEOUT, srp_stateChange}
};
#define stateReportSlotListSize (sizeof(stateReportSlot)/sizeof(srp_stateSlot_t))

srp_itemState_t stateReportSlotStatusList[stateReportSlotListSize];

const stateReportProtocol_t stateSystem = {
	.signals = nullptr,
	.signalSize = 0,
	.slots = stateReportSlot,
	.slotSize = stateReportSlotListSize,
	._signalState = nullptr,
	._slotState = stateReportSlotStatusList
};


//**** Trigger Configuration ******************************************************************************************

const tsp_triggerSignal_t triggerSignals[] = {
	{ 0x09, "Button 2"},
	{ 0x0C, "Button 3"},
	{ 0x0F, "Button 4"},
	{ 0x12, "Button 5"},
	{ 0x15, "Button 6"},
	{ 0x40, "Amp Button"}
};
#define triggerSignalListSize (sizeof(triggerSignals)/sizeof(tsp_triggerSignal_t))

tsp_itemState_t triggerSignalStateList[triggerSignalListSize];

const triggerSystemProtocol_t triggerSystem = {
	.signals = triggerSignals,
	.signalSize = triggerSignalListSize,
	.slots = nullptr,
	.slotSize = 0,
	._signalState = triggerSignalStateList,
	._slotState = nullptr
};

//**** Value Configuration ********************************************************************************************
void vrp_valueChange(uint16_t valueChannelNumber, vrp_valueData_t value)
{
	update = true;
}

const vrp_valueSlot_t valueSlots[] = {
	{ 0x00, "Rotary Knob 1", SLOT_TIMEOUT, vrp_valueChange}
};
#define valueSlotListSize (sizeof(valueSlots)/sizeof(vrp_valueSlot_t))

vrp_itemState_t valueSlotStateList[valueSlotListSize];

const valueReportProtocol_t valueSystem = {
	.signals = nullptr,
	.signalSize = 0,
	.slots = valueSlots,
	.slotSize = valueSlotListSize,
	._signalState = nullptr,
	._slotState = valueSlotStateList
};

//*********************************************************************************************************************

bool onReceive(uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size)
{
	switch(protocol){
		case busProtocol_triggerProtocol:		return tsp_receiveHandler(&triggerSystem, sourceAddress, command, data, size);
		case busProtocol_valueReportProtocol:	return vrp_receiveHandler(&valueSystem, sourceAddress, command, data, size);
		case busProtocol_stateReportProtocol:	return srp_receiveHandler(&stateSystem, sourceAddress, command, data, size);
		default: return false;
	}
}

encoderSwitch_t encoderSwitch_0;
buttonSwitch_t buttonSwitch_0;

uint8_t button_old;
uint8_t ring_old;

uint8_t leds_old;

int main(void)
{
	// App init Code
	if(kernel.kernelSignals->initApp)
	{
		Reset_Handler();
		
		pin_enableOutput(IO_D03); // Reset
		pin_setOutput(IO_D03, true);
		
		// Interrupt inputs
		pin_enableInput(IO_D04);
		pin_enableInput(IO_D05);
		pin_enableInput(IO_D06);
		pin_enableInput(IO_D07);
		
		i2c.init(SERCOM1, kernel.clk_1MHz, 1, false);
		
		kernel.nvic.assignInterruptHandler(SERCOM1_0_IRQn,i2cMbInterrupt);
		kernel.nvic.assignInterruptHandler(SERCOM1_1_IRQn,i2cSbInterrupt);
		kernel.nvic.assignInterruptHandler(SERCOM1_3_IRQn,i2cError); 
		
		pin_enablePeripheralMux(IO_D08, PIN_FUNCTION_C);
		pin_enablePeripheralMux(IO_D09, PIN_FUNCTION_C);
		
		pin_setOutput(IO_D03, false); // Disable reset 
		
		encoderSwitch_0.address = 0x02;
		encoderSwitch_0.i2c = &i2c;
		encoderSwitch_init(&encoderSwitch_0);
		encoderSwitch_setLedBrightness(&encoderSwitch_0, 0xFFFF);
		
		buttonSwitch_0.address = 0x77;
		buttonSwitch_0.i2c = &i2c;
		buttonSwitch_init(&buttonSwitch_0);
		
		button_old = buttonSwitch_0.button;
		
		tsp_initialize(&triggerSystem);
		vrp_initialize(&valueSystem);
		srp_initialize(&stateSystem);
				
		kernel.appSignals->appReady = true;
	}

	// Main code here
	if(kernel.appSignals->appReady == true)
	{
		tsp_mainHandler(&triggerSystem);
		vrp_mainHandler(&valueSystem);
		srp_mainHandler(&stateSystem);
		
		i2c.handler();
		
		encoderSwitch_handler(&encoderSwitch_0);
	 	buttonSwitch_handler(&buttonSwitch_0);
		
		uint8_t leds = 0;
		for(uint8_t i = 0; i < 6; i++)
		{
			leds = (leds>>1);
			if(srp_getStateByIndex(&stateSystem, i)== srp_state_t::srp_state_on) leds |= 0x20;
		}
		
		if(leds_old != leds)
		{
			buttonSwitch_setLeds(&buttonSwitch_0,leds);
			leds_old = leds;
		}
		
		if(buttonSwitch_0.button != button_old && buttonSwitch_0.state == buttonSwitch_state_run)
		{
			uint8_t indexList[6] = {0,0,0,0,0,0};
			uint8_t button = buttonSwitch_0.button;
			
			for(uint8_t i = 0; i < 6; i++)
			{
				if(button & 0x01) indexList[i] = 6 + i;
				button = (button>>1);
			}
			
			tsp_sendTriggersByIndex(&triggerSystem, &indexList[0], sizeof(indexList));
			
			button_old = buttonSwitch_0.button;
		}
		
		if(update == true)
		{
			vrp_valueData_t value = vrp_valueByIndex(&valueSystem, 0);
			encoderSwitch_setRingLed(&encoderSwitch_0,(uint8_t)(value.Long/8));
			
			if(srp_getStateByIndex(&stateSystem, 6) == srp_state_on){
				encoderSwitch_setPowerLed(&encoderSwitch_0, true);
			}else{
				encoderSwitch_setPowerLed(&encoderSwitch_0, false);
			}
			
			update = false;
		}
		
		if(encoderSwitch_0.down){
			vrp_valueData_t value;
			value.Long = encoderSwitch_0.down*3;
			
			encoderSwitch_0.up = 0;
			encoderSwitch_0.down = 0;
			
			vrp_sendValueCommandByIndex(&valueSystem, 0, vrp_vCmd_subtractLongClamp, value);
		}else if(encoderSwitch_0.up){
			vrp_valueData_t value;
			value.Long = encoderSwitch_0.up*3;
			
			encoderSwitch_0.up = 0;
			encoderSwitch_0.down = 0;
			
			vrp_sendValueCommandByIndex(&valueSystem, 0, vrp_vCmd_addLongClamp, value);
		}
		

		if(ed_onRising(&buttonEdgeDetect, encoderSwitch_0.button))
		{
			tsp_sendTriggerByIndex(&triggerSystem, 5);
		}
		
		if(i2c.hasError()){
			i2c.reset();
		}
	}
	
	// App deinit code
    if(kernel.kernelSignals->shutdownApp)
    {
		kernel.appSignals->shutdownReady = true;
    }
}