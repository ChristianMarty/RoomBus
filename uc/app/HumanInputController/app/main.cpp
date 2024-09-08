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
#include "protocol/valueSystemProtocol.h"
#include "protocol/stateSystemProtocol.h"

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

encoderSwitch_t encoderSwitch_0;
buttonSwitch_t buttonSwitch_0;
i2cMaster_c i2c;

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
	

//**** State Configuration ********************************************************************************************
#define SLOT_TIMEOUT 10

const ssp_stateSlot_t stateSystemSlot[] = {
	{0x02, "Button 1", SLOT_TIMEOUT, nullptr},
	{0x03, "Button 2", SLOT_TIMEOUT, nullptr},
	{0x04, "Button 3", SLOT_TIMEOUT, nullptr},
	{0x07, "Button 4", SLOT_TIMEOUT, nullptr},
	{0x06, "Button 5", SLOT_TIMEOUT, nullptr},
	{0x05, "Button 6", SLOT_TIMEOUT, nullptr},
		
	{0x29, "Rotary Button LED", SLOT_TIMEOUT, nullptr}
};
#define stateSystemSlotListSize (sizeof(stateSystemSlot)/sizeof(ssp_stateSlot_t))

ssp_itemState_t stateSystemSlotStatusList[stateSystemSlotListSize];

const stateSystemProtocol_t stateSystem = {
	.signals = nullptr,
	.signalSize = 0,
	.slots = stateSystemSlot,
	.slotSize = stateSystemSlotListSize,
	._signalState = nullptr,
	._slotState = stateSystemSlotStatusList
};


//**** Trigger Configuration ******************************************************************************************

const tsp_triggerSignal_t triggerSignals[] = {
	{ 0x06, "Button 1"},
	{ 0x09, "Button 2"},
	{ 0x0C, "Button 3"},
	{ 0x15, "Button 4"},
	{ 0x12, "Button 5"},
	{ 0x0F, "Button 6"},
		
	{ 0x5E, "Amp Button"}
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

const vsp_valueSlot_t valueSlots[] = {
	{ 0x01, "Rotary Knob 1", SLOT_TIMEOUT, nullptr}
};
#define valueSlotListSize (sizeof(valueSlots)/sizeof(vsp_valueSlot_t))

vsp_itemState_t valueSlotStateList[valueSlotListSize];

const valueSystemProtocol_t valueSystem = {
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
		case busProtocol_triggerSystemProtocol:	return tsp_receiveHandler(&triggerSystem, sourceAddress, command, data, size);
		case busProtocol_valueSystemProtocol:	return vsp_receiveHandler(&valueSystem, sourceAddress, command, data, size);
		case busProtocol_stateSystemProtocol:	return ssp_receiveHandler(&stateSystem, sourceAddress, command, data, size);
		default: return false;
	}
}

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
		
		tsp_initialize(&triggerSystem);
		vsp_initialize(&valueSystem);
		ssp_initialize(&stateSystem);
				
		kernel.appSignals->appReady = true;
	}

	// Main code here
	if(kernel.appSignals->appReady == true)
	{
		tsp_mainHandler(&triggerSystem);
		vsp_mainHandler(&valueSystem);
		ssp_mainHandler(&stateSystem);
		
		encoderSwitch_handler(&encoderSwitch_0);
		buttonSwitch_handler(&buttonSwitch_0);
		
		// Handle I2C
		i2c.handler();
		if(i2c.hasError()){
			i2c.reset();
		}
		
		// Update Button LEDs
		for(uint8_t i = 0; i < 6; i++)
		{
			bool led = ssp_getStateByIndex(&stateSystem, i) == ssp_state_t::srp_state_on;
			buttonSwitch_setLed(&buttonSwitch_0, i, led);
		}
		
		// Update Encoder LEDs
		vsp_valueData_t value = vsp_valueByIndex(&valueSystem, 0);
		encoderSwitch_setRingLed(&encoderSwitch_0, ((uint8_t)value.Long)/8);
		
		if(ssp_getStateByIndex(&stateSystem, 6) == srp_state_on){
			encoderSwitch_setPowerLed(&encoderSwitch_0, true);
		}else{
			encoderSwitch_setPowerLed(&encoderSwitch_0, false);
		}
		
		// Handle Button Change
		buttonSwitch_userInput_t buttonInput = buttonSwitch_getUserInput(&buttonSwitch_0);
		if(buttonInput.old != buttonInput.current)
		{
			uint8_t indexList[6] = {0,0,0,0,0,0};
			uint8_t index = 0;
			
			for(uint8_t i = 0; i < sizeof(indexList); i++)
			{
				if(buttonInput.current & 0x01){
					indexList[index] = i;
					index++;
				}
				buttonInput.current = (buttonInput.current>>1);
			}
			
			tsp_sendTriggersByIndex(&triggerSystem, &indexList[0], index);
		}
		
		// Handle Encoder Change
		encoderSwitch_userInput_t encoderInput = encoderSwitch_getUserInput(&encoderSwitch_0);
		
		if(encoderInput.down){
			vsp_valueData_t value = {.Long = (uint32_t)(encoderInput.down*3)};
			vsp_sendValueCommandByIndex(&valueSystem, 0, vsp_vCmd_subtractLongClamp, value);
			
		}else if(encoderInput.up){
			vsp_valueData_t value = {.Long = (uint32_t)(encoderInput.up*3)};
			vsp_sendValueCommandByIndex(&valueSystem, 0, vsp_vCmd_addLongClamp, value);
		}

		if(encoderInput.button != encoderInput.oldButton && encoderInput.button)
		{
			tsp_sendTriggerByIndex(&triggerSystem, 6);
		}
		
	}
	
	// App deinit code
    if(kernel.kernelSignals->shutdownApp)
    {
		kernel.appSignals->shutdownReady = true;
    }
}