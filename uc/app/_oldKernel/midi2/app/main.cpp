/*
 * app.cpp
 *
 * Created: 20.04.2019 20:05:30
 * Author : Christian
 */ 


#include "sam.h"
#include "kernel/kernel.h"
#include "kernel/busController_IO.h"

#include "drv/SAMx5x/uart.h"
#include "drv/SAMx5x/pin.h"
#include "utility/string.h"

#include "addOn/midiModul.h"
#include "Raumsteuerung/valueReportProtocol.h"

int main(const kernel_t *kernel);
void onReceive(const kernel_t *kernel, uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size);
kernel_t const *_kernel;

__attribute__((section(".appHeader"))) appHead_t appHead ={
/*appCRC	 */ 0xAABBCCDD, // Will be written by Bootload tool
/*appSize	 */ 0xEEFF0000, // Will be written by Bootload tool
/*appRevMaj	 */ 0x01,
/*appRevMin	 */ 0x00,
/*appName[60]*/ "Midi Modul",
/*main		 */ main,
/*onRx		 */ onReceive
};

uint8_t ememData[256];

float volume;

void vrp_valueChange(uint8_t index)
{
	vrp_sendValueReport(_kernel,0,volume);
	
	midiModul_sendControllerChange(midiModul_output_t::midiModul_output_2, 0x00,  0, (uint8_t)volume); // Update BCF2200
	midiModul_sendControllerChange(midiModul_output_t::midiModul_output_1, 0x00, 70, (uint8_t)volume); // Update X32 Volume
	
}

vrp_valueItem_t valueItemList[]={
	{ 0x00, false, ((float)(0.0f)), ((float)(127.0f)), &volume, UOM_RAW_FLOAT, "Main Volume",vrp_valueChange}
};
#define valueItemListSize (sizeof(valueItemList)/sizeof(vrp_valueItem_t)) 

valueReportProtocol_t valueReportProtocol=
{
	.value = valueItemList,
	.valueSize = valueItemListSize
};

uint8_t val;
uint8_t val2;
char logMsg[20];

void midi_controllerChangeChange(uint8_t index)
{
	float temp = (float)val;
	
	if(temp != volume) 
	{
		vrp_sendValueReport(_kernel,0,temp);
		midiModul_sendControllerChange(midiModul_output_t::midiModul_output_2, 0x00, 0, val); // Update BCF2200
		
		volume = temp;
	}
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

void onReceive(const kernel_t *kernel, uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size)
{
	if(protocol == busProtocol_valueReportProtocol) vrp_receiveHandler(kernel,sourceAddress,command,data,size,&valueReportProtocol);
}


int main(const kernel_t *kernel)
{
	_kernel = kernel;
	
	// App init Code
	if(kernel->kernelSignals->initApp)
	{
		Reset_Handler();
		midiModul_init(kernel, &midiModul);
		kernel->appSignals->appReady = true;
	}

	// Main code here
	if(kernel->appSignals->appReady == true)
	{
		midiModul_handler(kernel);
	}
	
	// App deinit code
    if(kernel->kernelSignals->shutdownApp)
    {
		//kernel->eemem_write(&ememData[0]);
		kernel->appSignals->shutdownReady = true;
    }
}