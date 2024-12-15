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
#include "Raumsteuerung/midi.h"

#include "Raumsteuerung/valueReportProtocol.h"

int main(const kernel_t *kernel);
void onReceive(const kernel_t *kernel, uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size);
kernel_t const *_kernel;

__attribute__((section(".appHeader"))) appHead_t appHead ={
/*appCRC	 */ 0xAABBCCDD, // Will be written by Bootload tool
/*appSize	 */ 0xEEFF0000, // Will be written by Bootload tool
/*appRevMaj	 */ 0x01,
/*appRevMin	 */ 0x00,
/*appName[60]*/ "Midi Test",
/*main		 */ main,
/*onRx		 */ onReceive
};


uint8_t ememData[256];

float volume;

void vrp_valueChange(uint8_t index)
{
	vrp_sendValueReport(_kernel,0,volume);
	midi_sendControllerChange(0x00,70,(uint8_t)volume);
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

void midi_controllerChangeChange(uint8_t index)
{
	float temp = (float)val;
	
	if(temp != volume) 
	{
		vrp_sendValueReport(_kernel,0,temp);
		volume = temp;
	}
}

midi_controllerChange_t midiControllerChange[]={
	{ 0x00, 70, &val,midi_controllerChangeChange}	
};
#define midiControllerChangeSize (sizeof(midiControllerChange)/sizeof(midi_controllerChange_t)) 

midi_t midi={
	.controllerChange =midiControllerChange,
	.controllerChangeSize = midiControllerChangeSize
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
		
		midi_init(kernel, SERCOM5);
		
		// Uart
		pin_enablePeripheralMux(IO_D12, PIN_FUNCTION_C);
		pin_enablePeripheralMux(IO_D13, PIN_FUNCTION_C);
		kernel->nvic.assignInterruptHandler(SERCOM5_2_IRQn,midi_onRx);
		kernel->nvic.assignInterruptHandler(SERCOM5_1_IRQn,midi_onTx);

		kernel->appSignals->appReady = true;
	}

	// Main code here
	if(kernel->appSignals->appReady == true)
	{
		midi_handler(kernel,&midi);
		
	}
	
	// App deinit code
    if(kernel->kernelSignals->shutdownApp)
    {
		//kernel->eemem_write(&ememData[0]);
		kernel->appSignals->shutdownReady = true;
    }
}