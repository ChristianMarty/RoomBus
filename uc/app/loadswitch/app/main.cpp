//**********************************************************************************************************************
// FileName : main.cpp
// FilePath : 
// Author   : Christian Marty
// Date		: 15.06.2024
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#include "main.h"

#include "protocol/triggerSystemProtocol.h"
#include "protocol/eventSystemProtocol.h"
#include "protocol/stateSystemProtocol.h"

#include "utility/bistableRelay.h"

int main(void);
bool onReceive(uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size);
kernel_t kernel __attribute__((section(".kernelCall")));

__attribute__((section(".appHeader"))) appHead_t appHead ={
/*appCRC	 */ 0xAABBCCDD, // Will be written by Bootload tool
/*appSize	 */ 0xEEFF0000, // Will be written by Bootload tool
/*appRevMaj	 */ 0x01,
/*appRevMin	 */ 0x00,
/*appName[60]*/ "Power Distributor",
/*main		 */ main,
/*onRx		 */ onReceive
};

#define OFF_DELAY 30000
#define STATE_SEND_INTERVAL 30

uint32_t mainOffTimer;
uint32_t ampOffTimer;

uint8_t output;
uint8_t outputOld;

uint8_t amp;
uint8_t ampOld;

bsRelay_t mainRelay;
bsRelay_t ampPowerRelay;

void mainPowerOnOff(uint16_t triggerChannelNumber);
void ampPowerOnOff(uint16_t triggerChannelNumber);

void outputOnOff(uint16_t triggerChannelNumber);
void ampOnOff(uint16_t triggerChannelNumber);

void ampMakro(uint16_t triggerChannelNumber);

//**** State Configuration ********************************************************************************************

const ssp_stateSignal_t stateSystemSignal[] = {
	{0x01, "Main Input", STATE_SEND_INTERVAL},
	{0x02, "Output 1", STATE_SEND_INTERVAL},
	{0x03, "Output 2", STATE_SEND_INTERVAL},
	{0x04, "Output 3", STATE_SEND_INTERVAL},
	{0x05, "Output 4", STATE_SEND_INTERVAL},
	{0x06, "Output 5", STATE_SEND_INTERVAL},
	{0x07, "Output 6", STATE_SEND_INTERVAL},
	{0x08, "Output 7", STATE_SEND_INTERVAL},
	{0x09, "Output 8", STATE_SEND_INTERVAL},
	
	{0x20, "Amp Mains Power", STATE_SEND_INTERVAL},
	{0x21, "Amp Channel 1", STATE_SEND_INTERVAL},
	{0x22, "Amp Channel 2", STATE_SEND_INTERVAL},
	{0x23, "Amp Channel 3", STATE_SEND_INTERVAL},
	{0x24, "Amp Channel 4", STATE_SEND_INTERVAL},
	{0x25, "Amp Channel 5", STATE_SEND_INTERVAL},
	{0x26, "Amp Channel 6", STATE_SEND_INTERVAL},
	{0x27, "Amp Channel 7", STATE_SEND_INTERVAL},
	{0x28, "Amp Channel 8", STATE_SEND_INTERVAL},
	{0x29, "Amp Zone 1", STATE_SEND_INTERVAL}
};
#define stateSystemSignalListSize (sizeof(stateSystemSignal)/sizeof(ssp_stateSignal_t))

ssp_itemState_t stateSystemSignalStatusList[stateSystemSignalListSize];

const stateSystemProtocol_t stateSystem = {
	.signals = stateSystemSignal,
	.signalSize = stateSystemSignalListSize,
	.slots = nullptr,
	.slotSize = 0,
	._signalState = stateSystemSignalStatusList,
	._slotState = nullptr
};


//**** Trigger Configuration ******************************************************************************************

const tsp_triggerSlot_t triggerSlots[] = {
	{ 0x00, "Power On", mainPowerOnOff },
	{ 0x01, "Power Off", mainPowerOnOff},
	
	{ 0x02, "Output All On", outputOnOff},
	{ 0x03, "Output All Off", outputOnOff},
		
	{ 0x04, "Output 1 On", outputOnOff},
	{ 0x05, "Output 1 Off", outputOnOff},
	{ 0x06, "Output 1 Toggle", outputOnOff},
	
	{ 0x07, "Output 2 On", outputOnOff},
	{ 0x08, "Output 2 Off", outputOnOff},
	{ 0x09, "Output 2 Toggle", outputOnOff},
			
	{ 0x0A, "Output 3 On", outputOnOff},
	{ 0x0B, "Output 3 Off", outputOnOff},
	{ 0x0C, "Output 3 Toggle", outputOnOff},
	
	{ 0x0D, "Output 4 On", outputOnOff},
	{ 0x0E, "Output 4 Off", outputOnOff},
	{ 0x0F, "Output 4 Toggle", outputOnOff},
	
	{ 0x10, "Output 5 On", outputOnOff},
	{ 0x11, "Output 5 Off", outputOnOff},
	{ 0x12, "Output 5 Toggle", outputOnOff},
	
	{ 0x13, "Output 6 On", outputOnOff},
	{ 0x14, "Output 6 Off", outputOnOff},
	{ 0x15, "Output 6 Toggle", outputOnOff},
		
	{ 0x16, "Output 7 On", outputOnOff},
	{ 0x17, "Output 7 Off", outputOnOff},
	{ 0x18, "Output 7 Toggle", outputOnOff},
		
	{ 0x19, "Output 8 On", outputOnOff},
	{ 0x1A, "Output 8 Off", outputOnOff},
	{ 0x1B, "Output 8 Toggle", outputOnOff},
		
// **** AMP *********************************************************************************************************** 		
	{ 0x40, "Amp Main On", ampPowerOnOff },
	{ 0x41, "Amp Main Off", ampPowerOnOff},
		
	{ 0x42, "Amp All On", ampOnOff},
	{ 0x43, "Amp All Off", ampOnOff},
		
	{ 0x44, "Amp Channel 1 On", ampOnOff},
	{ 0x45, "Amp Channel 1 Off", ampOnOff},
	{ 0x46, "Amp Channel 1 Toggle", ampOnOff},
	
	{ 0x47, "Amp Channel 2 On", ampOnOff},
	{ 0x48, "Amp Channel 2 Off", ampOnOff},
	{ 0x49, "Amp Channel 2 Toggle", ampOnOff},
	
	{ 0x4A, "Amp Channel 3 On", ampOnOff},
	{ 0x4B, "Amp Channel 3 Off", ampOnOff},
	{ 0x4C, "Amp Channel 3 Toggle", ampOnOff},
	
	{ 0x4D, "Amp Channel 4 On", ampOnOff},
	{ 0x4E, "Amp Channel 4 Off", ampOnOff},
	{ 0x4F, "Amp Channel 4 Toggle", ampOnOff},
	
	{ 0x50, "Amp Channel 5 On", ampOnOff},
	{ 0x51, "Amp Channel 5 Off", ampOnOff},
	{ 0x52, "Amp Channel 5 Toggle", ampOnOff},
	
	{ 0x53, "Amp Channel 6 On", ampOnOff},
	{ 0x54, "Amp Channel 6 Off", ampOnOff},
	{ 0x55, "Amp Channel 6 Toggle", ampOnOff},
	
	{ 0x56, "Amp Channel 7 On", ampOnOff},
	{ 0x57, "Amp Channel 7 Off", ampOnOff},
	{ 0x58, "Amp Channel 7 Toggle", ampOnOff},
	
	{ 0x59, "Amp Channel 8 On", ampOnOff},
	{ 0x5A, "Amp Channel 8 Off", ampOnOff},
	{ 0x5B, "Amp Channel 8 Toggle", ampOnOff},
		
	{ 0x5C, "Makro Amp On", ampMakro},
	{ 0x5D, "Makro Amp Off", ampMakro},
	{ 0x5E, "Makro Amp Toggle", ampMakro}
};
#define triggerSlotListSize (sizeof(triggerSlots)/sizeof(tsp_triggerSlot_t))

tsp_itemState_t triggerSlotStateList[triggerSlotListSize];

const triggerSystemProtocol_t triggerSystem = {
	.signals = nullptr,
	.signalSize = 0,
	.slots = triggerSlots,
	.slotSize = triggerSlotListSize,
	._signalState = nullptr,
	._slotState = triggerSlotStateList
};


//**** Event Configuration ********************************************************************************************

const esp_eventSlot_t eventSlots[] = {
	{0x00, "12V Power Supply", 30000, nullptr},
	{0x01, "LED Power", 30000, nullptr},
	{0x02, "Amp Power", 60000, nullptr},
	{0x03, "A/V Aux Power", 30000, nullptr}
};
#define eventSlotListSize (sizeof(eventSlots)/sizeof(esp_eventSlot_t))

esp_itemState_t eventSlotStatusList[eventSlotListSize];

const eventSystemProtocol_t eventSystem = {
	.signals = nullptr,
	.signalSize = 0,
	.slots = eventSlots,
	.slotSize = eventSlotListSize,
	._signalState = nullptr,
	._slotState = eventSlotStatusList
};

//**** onReceive Configuration ****************************************************************************************

bool onReceive(uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size)
{
	switch(protocol)
	{
		case busProtocol_triggerSystemProtocol:		return tsp_receiveHandler(&triggerSystem, sourceAddress, command, data, size);
		case busProtocol_eventSystemProtocol:			return esp_receiveHandler(&eventSystem, sourceAddress, command, data, size);
		case busProtocol_stateSystemProtocol:	return ssp_receiveHandler(&stateSystem, sourceAddress, command, data, size);
	}
	return false;
}


typedef enum  {
	onOff_idel,
	onOff_on,
	onOff_turnOff,
	onOff_off
}onOffState_t;

onOffState_t onOffState;

void mainPowerOnOff(uint16_t triggerChannelNumber)
{
	if(triggerChannelNumber == 0) onOffState = onOff_on;
	else if(triggerChannelNumber == 1) onOffState = onOff_turnOff;
}

void mainOnOffControl()
{
	switch(onOffState)
	{
		case onOff_idel :	
			kernel.tickTimer.reset(&mainOffTimer);
			break;
		
		case onOff_on:		
			bsRelay_set(&mainRelay, true);
			ssp_setStateByIndex(&stateSystem, 0, srp_state_on);
			kernel.tickTimer.reset(&mainOffTimer);
			onOffState = onOff_idel;
			break;
		
		case onOff_turnOff: 
			if(kernel.tickTimer.delay1ms(&mainOffTimer, OFF_DELAY)){
				onOffState = onOff_off;
			}
			break;
		
		case onOff_off:		
			bsRelay_set(&mainRelay,false);
			ssp_setStateByIndex(&stateSystem, 0, srp_state_off);
			kernel.tickTimer.reset(&mainOffTimer);
			onOffState = onOff_idel;
			break;
	}
}


void outputOnOff(uint16_t triggerChannelNumber)
{
	if(triggerChannelNumber == 2) output = 0xFF;
	else if(triggerChannelNumber == 3) output = 0x00;
	else
	{
		triggerChannelNumber -= 4;
			
		if(triggerChannelNumber%3 == 0)
		{
			output |= (0x01<<(triggerChannelNumber/3));
		}
		else if(triggerChannelNumber%3 == 1)
		{
			output &= ~(0x01<<(triggerChannelNumber/3));
		}
		else if(triggerChannelNumber%3 == 2)
		{
			output ^= (0x01<<(triggerChannelNumber/3));
		}
	}
}


onOffState_t ampOnOffState;

void ampPowerOnOff(uint16_t triggerChannelNumber)
{
	triggerChannelNumber -= 0x1C;
	
	if(triggerChannelNumber == 0) ampOnOffState = onOff_on;
	else if(triggerChannelNumber == 1) ampOnOffState = onOff_turnOff;
}

void ampOnOff(uint16_t triggerChannelNumber)
{
	triggerChannelNumber -= 0x1C;
	
	if(triggerChannelNumber == 2){
		amp = 0xFF;
	}else if(triggerChannelNumber == 3){
		amp = 0x00;
	}else{
		triggerChannelNumber -= 4;
		
		if(triggerChannelNumber%3 == 0)
		{
			amp |= (0x01<<(triggerChannelNumber/3));
		}
		else if(triggerChannelNumber%3 == 1)
		{
			amp &= ~(0x01<<(triggerChannelNumber/3));
		}
		else if(triggerChannelNumber%3 == 2)
		{
			amp ^= (0x01<<(triggerChannelNumber/3));
		}
	}
}


void ampOnOffControl()
{
	switch(ampOnOffState)
	{
		case onOff_idel :	
			kernel.tickTimer.reset(&ampOffTimer);
			break;
		
		case onOff_on:		
			bsRelay_set(&ampPowerRelay, true);
			ssp_setStateByIndex(&stateSystem, 9, srp_state_on);
			kernel.tickTimer.reset(&ampOffTimer);
			ampOnOffState = onOff_idel;
			break;
		
		case onOff_turnOff: 
			if(kernel.tickTimer.delay1ms(&ampOffTimer, OFF_DELAY)) {
				ampOnOffState = onOff_off;
			}
			break;
		
		case onOff_off:		
			bsRelay_set(&ampPowerRelay, false);
			ssp_setStateByIndex(&stateSystem, 9, srp_state_off);
			kernel.tickTimer.reset(&ampOffTimer);
			ampOnOffState = onOff_idel;
			break;
	}
}

void ampMakro(uint16_t triggerChannelNumber)
{
	if(triggerChannelNumber == 0x5C)
	{
		amp = 0xF0;
		ssp_setStateByIndex(&stateSystem, 0x12, srp_state_on);
	}
	else if(triggerChannelNumber == 0x5D) 
	{
		amp = 0x00;
		ssp_setStateByIndex(&stateSystem, 0x12, srp_state_off);
	}
	else if(triggerChannelNumber == 0x5E)
	{
		if(stateSystemSignalStatusList[0x12].state != srp_state_on)
		{
			amp = 0xF0;
			ssp_setStateByIndex(&stateSystem, 0x12, srp_state_on);
		}
		else
		{
			amp = 0x00;
			ssp_setStateByIndex(&stateSystem, 0x12, srp_state_off);
		}
	}	
}

void setOutput(uint8_t state)
{
	pin_setOutput(IO_D00, state&0x01);
	ssp_setStateByIndex(&stateSystem, 0x01, (ssp_state_t)((state&0x01)!=0));
	
	pin_setOutput(IO_D01, state&0x02);
	ssp_setStateByIndex(&stateSystem, 0x02, (ssp_state_t)((state&0x02)!=0));
	
	pin_setOutput(IO_D02, state&0x04);
	ssp_setStateByIndex(&stateSystem, 0x03, (ssp_state_t)((state&0x04)!=0));
	
	pin_setOutput(IO_D03, state&0x08);
	ssp_setStateByIndex(&stateSystem, 0x04, (ssp_state_t)((state&0x08)!=0));
	
	pin_setOutput(IO_D04, state&0x10);
	ssp_setStateByIndex(&stateSystem, 0x05, (ssp_state_t)((state&0x10)!=0));
	
	pin_setOutput(IO_D05, state&0x20);
	ssp_setStateByIndex(&stateSystem, 0x06, (ssp_state_t)((state&0x20)!=0));
	
	pin_setOutput(IO_D06, state&0x40);
	ssp_setStateByIndex(&stateSystem, 0x07, (ssp_state_t)((state&0x40)!=0));
	
	pin_setOutput(IO_D07, state&0x80);
	ssp_setStateByIndex(&stateSystem, 0x08, (ssp_state_t)((state&0x80)!=0));
}

void setAmpStandby(uint8_t state)
{
	pin_setOutput(IO_A00,(state&0x01));
	ssp_setStateByIndex(&stateSystem, 0x0A, (ssp_state_t)((state&0x01)!=0));
	
	pin_setOutput(IO_A01,(state&0x02));
	ssp_setStateByIndex(&stateSystem, 0x0B, (ssp_state_t)((state&0x02)!=0));
	
	pin_setOutput(IO_A02,(state&0x04));
	ssp_setStateByIndex(&stateSystem, 0x0C, (ssp_state_t)((state&0x04)!=0));
	
	pin_setOutput(IO_A03,(state&0x08));
	ssp_setStateByIndex(&stateSystem, 0x0D, (ssp_state_t)((state&0x08)!=0));
	
	pin_setOutput(IO_A04,(state&0x10));
	ssp_setStateByIndex(&stateSystem, 0x0E, (ssp_state_t)((state&0x10)!=0));
	
	pin_setOutput(IO_A05,(state&0x20));
	ssp_setStateByIndex(&stateSystem, 0x0F, (ssp_state_t)((state&0x20)!=0));
	
	pin_setOutput(IO_A06,(state&0x40));
	ssp_setStateByIndex(&stateSystem, 0x10, (ssp_state_t)((state&0x40)!=0));
	
	pin_setOutput(IO_A07,(state&0x80));
	ssp_setStateByIndex(&stateSystem, 0x11, (ssp_state_t)((state&0x80)!=0));
}


bool mainOutEventOld;
bool ampPowerEventOld;


int main(void)
{
	// App init Code
	if(kernel.kernelSignals->initApp)
	{
		Reset_Handler();

		output = 0;
		outputOld = 0xFF;
		setOutput(output);
		
		amp = 0;
		ampOld = 0xFF;
		setAmpStandby(amp);
		
		pin_enableOutput(IO_D00);
		pin_enableOutput(IO_D01);
		pin_enableOutput(IO_D02);
		pin_enableOutput(IO_D03);
		pin_enableOutput(IO_D04);
		pin_enableOutput(IO_D05);
		pin_enableOutput(IO_D06);
		pin_enableOutput(IO_D07);
		
		pin_enableOutput(IO_A00);
		pin_enableOutput(IO_A01);
		pin_enableOutput(IO_A02);
		pin_enableOutput(IO_A03);
		pin_enableOutput(IO_A04);
		pin_enableOutput(IO_A05);
		pin_enableOutput(IO_A06);
		pin_enableOutput(IO_A07);
		
		bsRelay_init(&mainRelay, IO_D15, IO_D14);
		bsRelay_init(&ampPowerRelay, IO_A08, IO_A09);
		
		tsp_initialize(&triggerSystem);
		esp_initialize(&eventSystem);
		ssp_initialize(&stateSystem);

		kernel.tickTimer.reset(&mainOffTimer);
		kernel.tickTimer.reset(&ampOffTimer);
		
		onOffState = onOff_idel;
		ampOnOffState = onOff_idel;
		
		mainOutEventOld = esp_getStateByIndex(&eventSystem, 0);
		ampPowerEventOld = esp_getStateByIndex(&eventSystem, 2);
		
		kernel.appSignals->appReady = true;
	}

	// Main code here
	if(kernel.appSignals->appReady == true)
	{
		tsp_mainHandler(&triggerSystem);
		esp_mainHandler(&eventSystem);
		ssp_mainHandler(&stateSystem);

		bsRelay_handler(&mainRelay);
		bsRelay_handler(&ampPowerRelay);
		
		mainOnOffControl();
		ampOnOffControl();
		
		if(esp_getStateByIndex(&eventSystem, 1) == true) output |= 0x80;
		else output &= 0x7F;
		
		if(esp_getStateByIndex(&eventSystem, 3) == true) output |= 0x40;
		else output &= 0xBF;
			
		// Output Power Control	
		if((outputOld != output) || (mainOutEventOld != esp_getStateByIndex(&eventSystem, 0)))
		{
			setOutput(output);
			
			outputOld = output;
			mainOutEventOld = esp_getStateByIndex(&eventSystem, 0);
			
			if((output > 0) || mainOutEventOld) onOffState = onOff_on;
			else onOffState = onOff_turnOff;
		}

		// Amp control
		if((ampOld != amp) || (ampPowerEventOld != esp_getStateByIndex(&eventSystem, 2)))
		{
			setAmpStandby(amp);
			
			ampOld = amp;
			ampPowerEventOld = esp_getStateByIndex(&eventSystem, 2);
			
			if((amp > 0)||(esp_getStateByIndex(&eventSystem, 2) == true)) ampOnOffState = onOff_on;
			else ampOnOffState = onOff_turnOff;
		}
		
	}
	
	// App deinit code
    if(kernel.kernelSignals->shutdownApp)
    {
		setOutput(0);
		kernel.appSignals->shutdownReady = true;
    }
	
}