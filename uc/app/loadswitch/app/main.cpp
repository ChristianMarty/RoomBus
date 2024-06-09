/*
 * app.cpp
 *
 * Created: 20.04.2019 20:05:30
 * Author : Christian
 */ 


#include "sam.h"
#include "kernel/kernel.h"
#include "kernel/busController_IO.h"

#include "Raumsteuerung/triggerProtocol.h"
#include "Raumsteuerung/eventProtocol.h"
#include "Raumsteuerung/stateReportProtocol.h"
#include "Raumsteuerung/bistableRelay.h"

int main(const kernel_t *kernel);
void onReceive(const kernel_t *kernel, uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size);

__attribute__((section(".appHeader"))) appHead_t appHead ={
/*appCRC	 */ 0xAABBCCDD, // Will be written by Bootload tool
/*appSize	 */ 0xEEFF0000, // Will be written by Bootload tool
/*appRevMaj	 */ 0x01,
/*appRevMin	 */ 0x00,
/*appName[60]*/ "Load switch",
/*main		 */ main,
/*onRx		 */ onReceive
};

#define OFF_DELAY 30000
#define STATE_SEND_INTERVAL 60000

uint32_t offTimer;
uint32_t sendStateTimer;

uint32_t ampOffTimer;

uint8_t output;
uint8_t outputOld;

uint8_t amp;
uint8_t ampOld;

uint8_t ememData[256];

bsRelay_t mainRelay;
bsRelay_t ampPowerRelay;

srp_state_t outputStateList[8];
srp_state_t mainRelayState;
srp_state_t mainRelayStateOld;

srp_state_t ampMainState;
srp_state_t ampMainStateOld;
srp_state_t ampChannelState[8];

srp_state_t ampMakroState;

const srp_stateReportChannel_t stateReportList[] = {
	{0x00, "Main Input", &mainRelayState},
	{0x01, "Output 1", &outputStateList[0]},
	{0x02, "Output 2", &outputStateList[1]},
	{0x03, "Output 3", &outputStateList[2]},
	{0x04, "Output 4", &outputStateList[3]},
	{0x05, "Output 5", &outputStateList[4]},
	{0x06, "Output 6", &outputStateList[5]},
	{0x07, "Output 7", &outputStateList[6]},
	{0x08, "Output 8", &outputStateList[7]},
	
	{0x09, "Amp Mains Power", &ampMainState},
	{0x0A, "Amp Channel 1", &ampChannelState[0]},
	{0x0B, "Amp Channel 2", &ampChannelState[1]},
	{0x0C, "Amp Channel 3", &ampChannelState[2]},
	{0x0D, "Amp Channel 4", &ampChannelState[3]},
	{0x0E, "Amp Channel 5", &ampChannelState[4]},
	{0x0F, "Amp Channel 6", &ampChannelState[5]},
	{0x10, "Amp Channel 7", &ampChannelState[6]},
	{0x11, "Amp Channel 8", &ampChannelState[7]},
	{0x12, "Amp Makro", &ampMakroState}
};
#define stateReportListSize (sizeof(stateReportList)/sizeof(srp_stateReportChannel_t))

void mainPowerOnOff(uint8_t triggerChannelNumber);
void ampPowerOnOff(uint8_t triggerChannelNumber);

void outputOnOff(uint8_t triggerChannelNumber);
void ampOnOff(uint8_t triggerChannelNumber);

void ampMakro(uint8_t triggerChannelNumber);

const tsp_triggerSlot_t triggerSlotList[] = {
	{UNFILTERED, 0x00, "Power On", mainPowerOnOff },
	{UNFILTERED, 0x01, "Power Off", mainPowerOnOff},
	
	{UNFILTERED, 0x02, "Output All On", outputOnOff},
	{UNFILTERED, 0x03, "Output All Off", outputOnOff},
		
	{UNFILTERED, 0x04, "Output 1 On", outputOnOff},
	{UNFILTERED, 0x05, "Output 1 Off", outputOnOff},
	{UNFILTERED, 0x06, "Output 1 Toggle", outputOnOff},
	
	{UNFILTERED, 0x07, "Output 2 On", outputOnOff},
	{UNFILTERED, 0x08, "Output 2 Off", outputOnOff},
	{UNFILTERED, 0x09, "Output 2 Toggle", outputOnOff},
			
	{UNFILTERED, 0x0A, "Output 3 On", outputOnOff},
	{UNFILTERED, 0x0B, "Output 3 Off", outputOnOff},
	{UNFILTERED, 0x0C, "Output 3 Toggle", outputOnOff},
	
	{UNFILTERED, 0x0D, "Output 4 On", outputOnOff},
	{UNFILTERED, 0x0E, "Output 4 Off", outputOnOff},
	{UNFILTERED, 0x0F, "Output 4 Toggle", outputOnOff},
	
	{UNFILTERED, 0x10, "Output 5 On", outputOnOff},
	{UNFILTERED, 0x11, "Output 5 Off", outputOnOff},
	{UNFILTERED, 0x12, "Output 5 Toggle", outputOnOff},
	
	{UNFILTERED, 0x13, "Output 6 On", outputOnOff},
	{UNFILTERED, 0x14, "Output 6 Off", outputOnOff},
	{UNFILTERED, 0x15, "Output 6 Toggle", outputOnOff},
		
	{UNFILTERED, 0x16, "Output 7 On", outputOnOff},
	{UNFILTERED, 0x17, "Output 7 Off", outputOnOff},
	{UNFILTERED, 0x18, "Output 7 Toggle", outputOnOff},
		
	{UNFILTERED, 0x19, "Output 8 On", outputOnOff},
	{UNFILTERED, 0x1A, "Output 8 Off", outputOnOff},
	{UNFILTERED, 0x1B, "Output 8 Toggle", outputOnOff},
		
	{UNFILTERED, 0x1C, "Amp Main On", ampPowerOnOff },
	{UNFILTERED, 0x1D, "Amp Main Off", ampPowerOnOff},
		
	{UNFILTERED, 0x1E, "Amp All On", ampOnOff},
	{UNFILTERED, 0x1F, "Amp All Off", ampOnOff},
		
	{UNFILTERED, 0x20, "Amp Channel 1 On", ampOnOff},
	{UNFILTERED, 0x21, "Amp Channel 1 Off", ampOnOff},
	{UNFILTERED, 0x22, "Amp Channel 1 Toggle", ampOnOff},
	
	{UNFILTERED, 0x23, "Amp Channel 2 On", ampOnOff},
	{UNFILTERED, 0x24, "Amp Channel 2 Off", ampOnOff},
	{UNFILTERED, 0x25, "Amp Channel 2 Toggle", ampOnOff},
	
	{UNFILTERED, 0x26, "Amp Channel 3 On", ampOnOff},
	{UNFILTERED, 0x27, "Amp Channel 3 Off", ampOnOff},
	{UNFILTERED, 0x28, "Amp Channel 3 Toggle", ampOnOff},
	
	{UNFILTERED, 0x29, "Amp Channel 4 On", ampOnOff},
	{UNFILTERED, 0x2A, "Amp Channel 4 Off", ampOnOff},
	{UNFILTERED, 0x2B, "Amp Channel 4 Toggle", ampOnOff},
	
	{UNFILTERED, 0x2C, "Amp Channel 5 On", ampOnOff},
	{UNFILTERED, 0x2D, "Amp Channel 5 Off", ampOnOff},
	{UNFILTERED, 0x2E, "Amp Channel 5 Toggle", ampOnOff},
	
	{UNFILTERED, 0x2F, "Amp Channel 6 On", ampOnOff},
	{UNFILTERED, 0x30, "Amp Channel 6 Off", ampOnOff},
	{UNFILTERED, 0x31, "Amp Channel 6 Toggle", ampOnOff},
	
	{UNFILTERED, 0x32, "Amp Channel 7 On", ampOnOff},
	{UNFILTERED, 0x33, "Amp Channel 7 Off", ampOnOff},
	{UNFILTERED, 0x34, "Amp Channel 7 Toggle", ampOnOff},
	
	{UNFILTERED, 0x35, "Amp Channel 8 On", ampOnOff},
	{UNFILTERED, 0x36, "Amp Channel 8 Off", ampOnOff},
	{UNFILTERED, 0x37, "Amp Channel 8 Toggle", ampOnOff},
		
	{UNFILTERED, 0x38, "Makro Amp On", ampMakro},
	{UNFILTERED, 0x39, "Makro Amp Off", ampMakro},
	{UNFILTERED, 0x40, "Makro Amp Toggle", ampMakro}
};
#define  triggerSlotListSize (sizeof(triggerSlotList)/sizeof(tsp_triggerSlot_t))


esp_eventState_t eventState[4];

const esp_eventSlot_t eventSlots[] = {
	{UNFILTERED, 0x00, "15V Power Supply", 30000, &eventState[0], nullptr},
	{UNFILTERED, 0x01, "LED Power", 30000, &eventState[1], nullptr},	
	{UNFILTERED, 0x02, "Amp Power", 60000, &eventState[2], nullptr},	
	{UNFILTERED, 0x03, "A/V Aux Power", 30000, &eventState[3], nullptr},
};

const eventSystemProtocol_t eventSystem = {
	.signals = nullptr,
	.signalSize = 0,
	.slots = eventSlots,
	.slotSize = (sizeof(eventSlots)/sizeof(esp_eventSlot_t))
};

const stateReportProtocol_t stateReport = {
	.states = nullptr,
	.stateSize = 0,
	.channels = stateReportList,
	.channelSize = stateReportListSize
};

tsp_itemStatus_t _slotsStatus[triggerSlotListSize];
const triggerSlotProtocol_t triggerSystem = {
	.signals = nullptr,
	.signalSize = 0,
	.slots = triggerSlotList,
	.slotSize = triggerSlotListSize,
	._slotsStatus = &_slotsStatus[0]
};

void onReceive(const kernel_t *kernel, uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size)
{
	switch(protocol)
	{
		case busProtocol_triggerProtocol:		tsp_receiveHandler(kernel, sourceAddress, command,data, size, &triggerSystem);	break;
		case busProtocol_stateReportProtocol:	srp_receiveHandler(kernel, sourceAddress, command,data, size, &stateReport);	break;
		case busProtocol_eventProtocol:			esp_receiveHandler(kernel, sourceAddress, command, data, size, &eventSystem);	break;
	}
}

void tsp_onTrigger_callback(const kernel_t *kernel, uint8_t sourceAddress, uint8_t triggerChannelNumber)
{
	tsp_triggerAction(kernel, triggerChannelNumber, triggerSlotList, triggerSlotListSize);
}

void tsp_onStateChange_callback(const kernel_t *kernel)
{
	
}

typedef enum  {
	onOff_idel,
	onOff_on,
	onOff_turnOff,
	onOff_off
}onOffState_t;

onOffState_t onOffState;

void mainPowerOnOff(uint8_t triggerChannelNumber)
{
	if(triggerChannelNumber == 0) onOffState = onOff_on;
	else if(triggerChannelNumber == 1) onOffState = onOff_turnOff;
}


void setMain(bool state)
{
	if(state)
	{
		bsRelay_set(&mainRelay,true);
		mainRelayState = srp_state_on;
	}
	else
	{
		bsRelay_set(&mainRelay,false);
		mainRelayState = srp_state_off;
	}
}

void outputOnOff(uint8_t triggerChannelNumber)
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

void mainOnOffControl(const kernel_t *kernel)
{
	switch(onOffState)
	{
		case onOff_idel :	kernel->tickTimer.reset(&offTimer);
							break;
		
		case onOff_on:		setMain(true);
							kernel->tickTimer.reset(&offTimer);
							onOffState = onOff_idel;
							break;
		
		case onOff_turnOff: if(kernel->tickTimer.delay1ms(&offTimer, OFF_DELAY))  onOffState = onOff_off;
							break;
		
		case onOff_off:		setMain(false);
							kernel->tickTimer.reset(&offTimer);
							onOffState = onOff_idel;
							break;
	}
}

onOffState_t ampOnOffState;

void ampPowerOnOff(uint8_t triggerChannelNumber)
{
	triggerChannelNumber -= 0x1C;
	
	if(triggerChannelNumber == 0) ampOnOffState = onOff_on;
	else if(triggerChannelNumber == 1) ampOnOffState = onOff_turnOff;
}


void ampOnOff(uint8_t triggerChannelNumber)
{
	triggerChannelNumber -= 0x1C;
	
	if(triggerChannelNumber == 2) amp = 0xFF;
	else if(triggerChannelNumber == 3) amp = 0x00;
	else
	{
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

void setAmpMain(bool state)
{
	if(state)
	{
		bsRelay_set(&ampPowerRelay,true);
		ampMainState = srp_state_on;
	}
	else
	{
		bsRelay_set(&ampPowerRelay,false);
		ampMainState = srp_state_off;
	}
}

void ampOnOffControl(const kernel_t *kernel)
{
	switch(ampOnOffState)
	{
		case onOff_idel :	kernel->tickTimer.reset(&ampOffTimer);
							break;
		
		case onOff_on:		setAmpMain(true);
							kernel->tickTimer.reset(&ampOffTimer);
							ampOnOffState = onOff_idel;
							break;
		
		case onOff_turnOff: if(kernel->tickTimer.delay1ms(&ampOffTimer, OFF_DELAY))  ampOnOffState = onOff_off;
							break;
		
		case onOff_off:		setAmpMain(false);
							kernel->tickTimer.reset(&ampOffTimer);
							ampOnOffState = onOff_idel;
							break;
	}
}

void ampMakro(uint8_t triggerChannelNumber)
{
	if(triggerChannelNumber == 0x38)
	{
		amp = 0xF0;
		ampMakroState = srp_state_on;
	}
	else if(triggerChannelNumber == 0x39) 
	{
		amp = 0x00;
		ampMakroState = srp_state_off;
	}
	else if(triggerChannelNumber == 0x40)
	{
		if(ampMakroState == srp_state_off)
		{
			amp = 0xF0;
			ampMakroState = srp_state_on;
		}
		else
		{
			amp = 0x00;
			ampMakroState = srp_state_off;
		}
	}	
}


void setOutput(uint8_t state)
{
	pin_setOutput(IO_D00,(state&0x01));
	pin_setOutput(IO_D01,(state&0x02));
	pin_setOutput(IO_D02,(state&0x04));
	pin_setOutput(IO_D03,(state&0x08));
	pin_setOutput(IO_D04,(state&0x10));
	pin_setOutput(IO_D05,(state&0x20));
	pin_setOutput(IO_D06,(state&0x40));
	pin_setOutput(IO_D07,(state&0x80));
}

void setAmpStandby(uint8_t state)
{
	pin_setOutput(IO_A00,(state&0x01));
	pin_setOutput(IO_A01,(state&0x02));
	pin_setOutput(IO_A02,(state&0x04));
	pin_setOutput(IO_A03,(state&0x08));
	pin_setOutput(IO_A04,(state&0x10));
	pin_setOutput(IO_A05,(state&0x20));
	pin_setOutput(IO_A06,(state&0x40));
	pin_setOutput(IO_A07,(state&0x80));
}


bool mainOutEventOld;
bool ampPowerEventOld;

void sendState(const kernel_t *kernel)
{
	
	uint8_t temp = output;
	 
	for(uint8_t i = 0; i<8; i++)
	{
		if(temp & 0x01) outputStateList[i] = srp_state_on;
		else outputStateList[i] = srp_state_off;
		
		temp = (temp>>1);
	}
	
	temp = amp;
	 
	for(uint8_t i = 0; i<8; i++)
	{
		if(temp & 0x01) ampChannelState[i] = srp_state_on;
		else ampChannelState[i] = srp_state_off;
		
		temp = (temp>>1);
	}
	
	srp_sendGroupReport(kernel, &stateReport);
	

	kernel->tickTimer.reset(&sendStateTimer);
}


int main(const kernel_t *kernel)
{
	// App init Code
	if(kernel->kernelSignals->initApp)
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
	
		//kernel->eemem.read(&ememData[0]);
	
		kernel->tickTimer.reset(&offTimer);
		kernel->tickTimer.reset(&ampOffTimer);
		kernel->tickTimer.reset(&sendStateTimer);
		
		esp_init(kernel, &eventSystem);

		onOffState = onOff_idel;
		ampOnOffState = onOff_idel;
		
		mainOutEventOld = eventState[0].state;
		ampPowerEventOld = eventState[2].state;
		
		kernel->appSignals->appReady = true;
		
	}

	// Main code here
	if(kernel->appSignals->appReady == true)
	{
		
		esp_mainHandler(kernel, &eventSystem);
		tsp_mainHandler(kernel, &triggerSystem);
		
		if(kernel->tickTimer.delay1ms(&sendStateTimer, STATE_SEND_INTERVAL)) sendState(kernel);
		
		mainOnOffControl(kernel);
		ampOnOffControl(kernel);
		
		bsRelay_handler(kernel, &mainRelay);
		bsRelay_handler(kernel, &ampPowerRelay);
		
		if(eventState[1].state == true) output |= 0x80;
		else output &= 0x7F;
		
		if(eventState[3].state == true) output |= 0x40;
		else output &= 0xBF;
			
		// Output Power Control	
		if((outputOld != output) || (mainOutEventOld != eventState[0].state))
		{
			setOutput(output);
			
			sendState(kernel);
			outputOld = output;
			mainOutEventOld = eventState[0].state;
			
			if((output > 0)||(eventState[0].state == true)) onOffState = onOff_on;
			else onOffState = onOff_turnOff;
		}
		
		// Amp control
		if((ampOld != amp) || (ampPowerEventOld != eventState[2].state))
		{
			setAmpStandby(amp);
			
			sendState(kernel);
			ampOld = amp;
			ampPowerEventOld = eventState[2].state;
			
			if((amp > 0)||(eventState[2].state == true)) ampOnOffState = onOff_on;
			else ampOnOffState = onOff_turnOff;
		}
		
		
		if(mainRelayState != mainRelayStateOld || ampMainStateOld != ampMainState)
		{
			sendState(kernel);
			mainRelayStateOld = mainRelayState;
			ampMainStateOld = ampMainState;
		}
		
	}
	
	// App deinit code
    if(kernel->kernelSignals->shutdownApp)
    {
		setOutput(0);
		//kernel->eemem.write(&ememData[0]);
		kernel->appSignals->shutdownReady = true;
    }
	
}