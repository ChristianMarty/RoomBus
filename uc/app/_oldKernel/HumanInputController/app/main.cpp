/*
 * app.cpp
 *
 * Created: 20.04.2019 20:05:30
 * Author : Christian
 */ 


#include "sam.h"
#include "kernel/kernel.h"
#include "kernel/busController_IO.h"

#include "drv/SAMx5x/pin.h"
#include "utility/string.h"

#include "utility/edgeDetect.h"

#include "addOn/i2cModul_rev2.h"
#include "addOn/encoderSwitch.h"
#include "addOn/buttonSwitch.h"

#include "drv/SAMx5x/i2cMaster.h"

#include "protocol/valueReportProtocol.h"
#include "protocol/triggerProtocol.h"
#include "protocol/stateReportProtocol.h"

int main(const kernel_t *kernel);
bool onReceive(const kernel_t *kernel, uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size);
kernel_t const *_kernel;

__attribute__((section(".appHeader"))) appHead_t appHead ={
/*appCRC	 */ 0xAABBCCDD, // Will be written by Bootload tool
/*appSize	 */ 0xEEFF0000, // Will be written by Bootload tool
/*appRevMaj	 */ 0x01,
/*appRevMin	 */ 0x00,
/*appName[60]*/ "Human Input Controller",
/*main		 */ main,
/*onRx		 */ onReceive
};

#define LOADSWITCH_ADDRESS 0x02

uint8_t ememData[256];

float mainVolTx;
float mainVolRx;

bool update = false;
bool rxVal = false;

i2cMaster_c i2c;

edgeDetect_t buttonEdgeDetect ={.oldState = false};

void vrp_valueChange(uint8_t index)
{
	update = true;
}

void vrp_valueSlaveChange(uint8_t index)
{
	
}

vrp_valueItem_t valueItemList[]={
	{ 0x00, false, ((float)(0.0f)), ((float)(127.0f)), &mainVolTx, UOM_RAW_UINT32, "CH 1",vrp_valueChange}
};
#define valueItemListSize (sizeof(valueItemList)/sizeof(vrp_valueItem_t)) 

vrp_valueSlaveItem_t valueSlaveItemList[]={
	{ UNFILTERED, 0x00, &mainVolRx, vrp_valueSlaveChange}
};
#define valueSlaveItemListSize (sizeof(valueItemList)/sizeof(vrp_valueSlaveItem_t))

valueReportProtocol_t valueReportProtocol=
{
	.value = valueItemList,
	.valueSize = valueItemListSize,
	.slaveValue = valueSlaveItemList,
	.slaveValueSize = valueSlaveItemListSize
};


void sendVal(uint8_t triggerChannelNumber)
{
	update = true;
}

void readVal(uint8_t triggerChannelNumber)
{
	rxVal = true;
}


srp_state_t ledState[12];
srp_state_t ampState;
 
const srp_stateReport_t stateReports[] = {
	{LOADSWITCH_ADDRESS, 0x12, &ampState},
	{LOADSWITCH_ADDRESS, 0x01, &ledState[6] },
	{LOADSWITCH_ADDRESS, 0x02, &ledState[7] },
	{LOADSWITCH_ADDRESS, 0x03, &ledState[8] },
	{LOADSWITCH_ADDRESS, 0x04, &ledState[9] },
	{LOADSWITCH_ADDRESS, 0x05, &ledState[10] },
	{LOADSWITCH_ADDRESS, 0x06, &ledState[11] }
};
#define stateReportListSize (sizeof(stateReports)/sizeof(srp_stateReport_t))

const tsp_triggerSlot_t triggerSlotList[] = {
};
#define  triggerSlotListSize (sizeof(triggerSlotList)/sizeof(tsp_triggerSlot_t))

const tsp_triggerSignal_t triggerSignals[] = {
	{0x40, "Amp"},
	{0x06, "Button 1"},
	{0x09, "Button 2"},
	{0x0C, "Button 3"},
	{0x0F, "Button 4"},
	{0x12, "Button 5"},
	{0x15, "Button 6"}
};
#define triggerSignalListSize (sizeof(triggerSignals)/sizeof(tsp_triggerSignal_t))

tsp_itemStatus_t _slotsStatus[triggerSlotListSize];
tsp_itemStatus_t _signalStatus[triggerSlotListSize];
const triggerSystemProtocol_t triggerSystem = {
	.signals = triggerSignals,
	.signalSize = triggerSignalListSize,
	
	.slots = triggerSlotList,
	.slotSize = triggerSlotListSize,
	
	._signalStatus = &_signalStatus[0],
	._slotStatus = &_slotsStatus[0]
};

const stateReportProtocol_t stateReport = {
	.states = stateReports,
	.stateSize = stateReportListSize,
	.channels = nullptr,
	.channelSize = 0
};

bool onReceive(const kernel_t *kernel, uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size)
{
	switch(protocol)
	{
		case busProtocol_triggerProtocol:		return tsp_receiveHandler(kernel, &triggerSystem, sourceAddress, command, data, size);
		case busProtocol_valueReportProtocol:	return vrp_receiveHandler(kernel, &valueReportProtocol, sourceAddress, command, data, size);
		case busProtocol_stateReportProtocol:	return srp_receiveHandler(kernel, &stateReport, sourceAddress, command, data, size);
	}
	return false;
}

void tsp_onStateChange_callback(const kernel_t *kernel)
{
	update = true;
}

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
	
encoderSwitch_t encoderSwitch_0;
buttonSwitch_t buttonSwitch_0;

uint8_t button_old;
uint8_t ring_old;

uint8_t leds_old;

int main(const kernel_t *kernel)
{
	_kernel = kernel;
	
	// App init Code
	if(kernel->kernelSignals->initApp)
	{
		Reset_Handler();
		
		pin_enableOutput(IO_D03); // Reset
		pin_setOutput(IO_D03, true);
		
		// Interrupt inputs
		pin_enableInput(IO_D04);
		pin_enableInput(IO_D05);
		pin_enableInput(IO_D06);
		pin_enableInput(IO_D07);
		
		i2c.init(kernel,SERCOM1,kernel->clk_1MHz,1,false);
		
		kernel->nvic.assignInterruptHandler(SERCOM1_0_IRQn,i2cMbInterrupt);
		kernel->nvic.assignInterruptHandler(SERCOM1_1_IRQn,i2cSbInterrupt);
		kernel->nvic.assignInterruptHandler(SERCOM1_3_IRQn,i2cError); //*/
		
		pin_enablePeripheralMux(IO_D08, PIN_FUNCTION_C);
		pin_enablePeripheralMux(IO_D09, PIN_FUNCTION_C);
		
		pin_setOutput(IO_D03, false); // Disable reset 
		
		encoderSwitch_0.address = 0x02;
		encoderSwitch_0.i2c = &i2c;
		encoderSwitch_init(&encoderSwitch_0, kernel);
		encoderSwitch_setLedBrightness(&encoderSwitch_0, 0xFFFF);
		
		buttonSwitch_0.address = 0x77;
		buttonSwitch_0.i2c = &i2c;
		buttonSwitch_init(&buttonSwitch_0, kernel);
		
		button_old = buttonSwitch_0.button;
		
		kernel->appSignals->appReady = true;
	}

	// Main code here
	if(kernel->appSignals->appReady == true)
	{
		tsp_mainHandler(kernel, &triggerSystem);
		
		i2c.handler();
		
		encoderSwitch_handler(&encoderSwitch_0, kernel);
	 	buttonSwitch_handler(&buttonSwitch_0, kernel);
		
		uint8_t leds = 0;
		for(uint8_t i = 6; i < 12; i++)
		{
			leds = (leds>>1);
			if(ledState[i] == srp_state_t::srp_state_on) leds |= 0x20;
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
			
			tsp_sendTriggerByIndex(kernel, &triggerSystem, &indexList[0], sizeof(indexList));
			
			button_old = buttonSwitch_0.button;
		}
		
		if(update == true)
		{
			encoderSwitch_setRingLed(&encoderSwitch_0,(uint8_t)(mainVolRx/8));
			
			if(ampState == srp_state_on) encoderSwitch_setPowerLed(&encoderSwitch_0, true);
			else encoderSwitch_setPowerLed(&encoderSwitch_0, false);
			
			update = false;
		}
		
		if(encoderSwitch_0.up || encoderSwitch_0.down)
		{
			float value = 0;
			if(encoderSwitch_0.up > encoderSwitch_0.down) value += encoderSwitch_0.up;
			else value -= encoderSwitch_0.down;
			
			value *=3;
			
			encoderSwitch_0.up = 0;
			encoderSwitch_0.down = 0;
			
			vrp_sendValueCommmand(kernel,0,vrp_addValue, value);
		}

		if(ed_onRising(&buttonEdgeDetect, encoderSwitch_0.button))
		{
			uint8_t indexList[1] = {14};
			tsp_sendTriggerByIndex(kernel, &triggerSystem, &indexList[0], sizeof(indexList));
		}
		
		if(i2c.hasError()) i2c.reset();
	}
	
	// App deinit code
    if(kernel->kernelSignals->shutdownApp)
    {
		kernel->appSignals->shutdownReady = true;
    }
}