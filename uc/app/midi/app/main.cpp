//**********************************************************************************************************************
// FileName : main.cpp
// FilePath :
// Author   : Christian Marty
// Date		: 20.04.2019
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#include "main.h"

#include "common/kernel.h"

#include "protocol/valueSystemProtocol.h"

#include "driver/SAMx5x/pin.h"

#include "addOn/midiModul.h"
#include "utility/string.h"

int main(void);
bool onReceive(uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size);
kernel_t kernel __attribute__((section(".kernelCall")));

__attribute__((section(".appHeader"))) appHead_t appHead ={
/*appCRC	 */ 0xAABBCCDD, // Will be written by Bootload tool
/*appSize	 */ 0xEEFF0000, // Will be written by Bootload tool
/*appRevMaj	 */ 0x01,
/*appRevMin	 */ 0x00,
/*appName[60]*/ "Midi Controller",
/*main		 */ main,
/*onRx		 */ onReceive
};

//**** Value Configuration ********************************************************************************************
void vrp_valueChange(uint16_t valueChannelNumber, vsp_valueData_t value)
{
	midiModul_sendControllerChange(midiModul_output_t::midiModul_output_2, 0x00,  0, (uint8_t)value.Long); // Update BCF2200
	midiModul_sendControllerChange(midiModul_output_t::midiModul_output_1, 0x00, 70, (uint8_t)value.Long); // Update X32 Volume
}

const vsp_valueSignal_t valueSignals[] = {
	{0x01, "Main Volume", 60, false, {.Long = 0}, {.Long = 127}, vsp_uom_long, vrp_valueChange}
};
#define valueSignalListSize ARRAY_LENGTH(valueSignals)

vsp_itemState_t valueSignalStateList[valueSignalListSize];

const valueSystemProtocol_t valueSystem = {
	.signals = valueSignals,
	.signalSize = valueSignalListSize,
	.slots = nullptr,
	.slotSize = 0,
	._signalState = valueSignalStateList,
	._slotState = nullptr
};

uint8_t val;
uint8_t val2;
void midi_controllerChangeChange(uint8_t index)
{
	midiModul_sendControllerChange(midiModul_output_t::midiModul_output_2, 0x00, 0, val); // Update BCF2200
	vsp_sendValueReportByChannel(&valueSystem, 0x01, {.Long = val});
}

void midi_controllerChangeChange_2(uint8_t index)
{
	midiModul_sendControllerChange(midiModul_output_t::midiModul_output_1, 0x00, 70, val2); // Update X32 Volume
}

midi_controllerChange_t midiControllerChange_1[]={
	{ 0x00, 70, &val, midi_controllerChangeChange}	
};

midi_controllerChange_t midiControllerChange_2[]={
	{ 0x00, 0, &val2, midi_controllerChangeChange_2}
};

midiModul_t midiModul={
	.controllerChange_1 = midiControllerChange_1,
	.controllerChangeSize_1 = (sizeof(midiControllerChange_1)/sizeof(midi_controllerChange_t)), 
	
	.controllerChange_2 = midiControllerChange_2,
	.controllerChangeSize_2 = (sizeof(midiControllerChange_2)/sizeof(midi_controllerChange_t))
	
};

bool onReceive(uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size)
{
	switch(protocol){
		case busProtocol_valueSystemProtocol:	return vsp_receiveHandler(&valueSystem, sourceAddress, command, data, size);
		default: return false;
	}
}


int main(void)
{
	// App init Code
	if(kernel.kernelSignals->initApp)
	{
		Reset_Handler();
		midiModul_init(&midiModul);
		vsp_initialize(&valueSystem);
		kernel.appSignals->appReady = true;
	}

	// Main code here
	if(kernel.appSignals->appReady == true)
	{
		midiModul_handler();
		vsp_mainHandler(&valueSystem);
	}
	
	// App deinit code
    if(kernel.kernelSignals->shutdownApp)
    {
		//kernel->eemem_write(&ememData[0]);
		kernel.appSignals->shutdownReady = true;
    }
}