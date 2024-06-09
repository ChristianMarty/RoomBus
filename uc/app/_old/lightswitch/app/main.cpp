/*
 * app.cpp
 *
 * Created: 20.04.2019 20:05:30
 * Author : Christian
 */ 

#include "sam.h"
#include "kernel/kernel.h"

#include "drv/SAMx5x/genericClockController.h"
#include "kernel/busController_IO.h"

#include "Raumsteuerung/lichtschalter.h"

#include "Raumsteuerung/triggerProtocol.h"
#include "Raumsteuerung/stateReportProtocol.h"

int main(const kernel_t *kernel);
void onReceive(const kernel_t *kernel, uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size);

__attribute__((section(".appHeader"))) appHead_t appHead ={
/*appCRC	 */ 0xAABBCCDD, // Will be written by Bootload tool
/*appSize	 */ 0xEEFF0000, // Will be written by Bootload tool
/*appRevMaj	 */ 0x01,
/*appRevMin	 */ 0x00,
/*appName[60]*/ "Light switch",
/*main		 */ main,
/*onRx		 */ onReceive
};

#define OUTPUT_ADDRESS 0x02

uint32_t tickTimer2;	
uint32_t buttonReadTimer;
uint32_t ledToggleTimer;

bool ledToggleState;

uint8_t i2cInit = 0;

uint8_t i2cStep = 0;
uint32_t i2cStepTimer = 0;	

i2cMaster_c i2c;
lightswitch_c lightSwitch1;
lightswitch_c lightSwitch2;

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

srp_state_t ledState[12];

const srp_stateReport_t stateReports[] = {
	{OUTPUT_ADDRESS, 0x01, &ledState[6] },
	{OUTPUT_ADDRESS, 0x02, &ledState[7] },
	{OUTPUT_ADDRESS, 0x03, &ledState[8] },
	{OUTPUT_ADDRESS, 0x04, &ledState[9] },
	{OUTPUT_ADDRESS, 0x05, &ledState[10] },
	{OUTPUT_ADDRESS, 0x06, &ledState[11] }
};
#define stateReportListSize (sizeof(stateReports)/sizeof(srp_stateReport_t))

const tsp_triggerSignal_t triggerSignals[] = {
	{5, OUTPUT_ADDRESS, 0x04, "Group 1"},
	{5, OUTPUT_ADDRESS, 0x07, "Group 1"},
	{5, OUTPUT_ADDRESS, 0x0A, "Group 1"},
	{3, OUTPUT_ADDRESS, 0x05, "Off"},
	{3, OUTPUT_ADDRESS, 0x08, "Off"},
	{3, OUTPUT_ADDRESS, 0x0B, "Off"},
	{6, OUTPUT_ADDRESS, 0x06, "Button 1"},
	{7, OUTPUT_ADDRESS, 0x09, "Button 2"},
	{8, OUTPUT_ADDRESS, 0x0C, "Button 3"},
	{9, OUTPUT_ADDRESS, 0x0F, "Button 4"},
	{10, OUTPUT_ADDRESS, 0x12, "Button 5"},
	{11, OUTPUT_ADDRESS, 0x15, "Button 6"}
};
#define triggerSignalListSize (sizeof(triggerSignals)/sizeof(tsp_triggerSignal_t))

const triggerSlotProtocol_t triggerSystem = {
	.signals = triggerSignals,
	.signalSize = triggerSignalListSize,
	.slots = nullptr,
	.slotSize = 0,
	._slotsStatus = nullptr
};

const stateReportProtocol_t stateReport = {
	.states = stateReports,
	.stateSize = stateReportListSize,
	.channels = nullptr,
	.channelSize = 0
};

void onReceive(const kernel_t *kernel, uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size)
{
	switch(protocol)
	{
		//case busProtocol_triggerProtocol:		tsp_receiveHandler(kernel, sourceAddress, command,data, size, &triggerSystem);	break;
		case busProtocol_stateReportProtocol:	srp_receiveHandler(kernel, sourceAddress, command,data, size, &stateReport);	break;
	}
}

void tsp_onStateChange_callback(const kernel_t *kernel)
{
	
}

void sendTriggers(const kernel_t *kernel, uint16_t buttoneState)
{
	static uint16_t buttoneStateOld = 0xFF;
	
	if(buttoneState == buttoneStateOld) return;
	
	uint16_t temp =  buttoneStateOld;
	buttoneStateOld = buttoneState;
	
	uint8_t indexList[12];
	uint8_t indexSize = 0;
	
	for(uint8_t i = 0; i<12; i++)
	{
		if((buttoneState&0x01)&&((buttoneState^temp)&0x01))
		{
			for(uint8_t j = 0; j < triggerSignalListSize; j++)
			{
				if(triggerSignals[j].groupIndex == i)
				{
					indexList[indexSize] = triggerSignals[j].groupIndex;
					indexSize ++;
					break;
				}
			}
		}
		
		buttoneState = (buttoneState>>1);
		temp = (temp>>1);
	}
	
	if(indexSize) tep_sendTriggerByGroup(kernel, &indexList[0], indexSize, triggerSignals, triggerSignalListSize);
}


gclk_generator_t clk_1MHz;


int main(const kernel_t *kernel)
{
	// App Init Code
	if(kernel->kernelSignals->initApp)
	{
		Reset_Handler();

		kernel->tickTimer.reset(&tickTimer2);
		kernel->tickTimer.reset(&buttonReadTimer);
		kernel->tickTimer.reset(&i2cStepTimer);
		kernel->tickTimer.reset(&ledToggleTimer);
		
		pin_enableInput(IO_D04);
		pin_enableInput(IO_D05);
		pin_enableInput(IO_D06);
		pin_enableInput(IO_D07);	
			
		pin_enableOutput(IO_D03);
		pin_setOutput(IO_D03,true); // Enable reset 
		
		MCLK->APBAMASK.bit.SERCOM1_ = true;
		gclk_peripheralEnable(GCLK_SERCOM1_CORE,kernel->clk_1MHz);
		i2c.init(kernel, SERCOM1,kernel->clk_1MHz, 10, false);
		 
		kernel->nvic.assignInterruptHandler(SERCOM1_0_IRQn,i2cMbInterrupt);
		kernel->nvic.assignInterruptHandler(SERCOM1_1_IRQn,i2cSbInterrupt);
		kernel->nvic.assignInterruptHandler(SERCOM1_3_IRQn,i2cError);
		
		pin_enablePeripheralMux(IO_D08, PIN_FUNCTION_C);
		pin_enablePeripheralMux(IO_D09, PIN_FUNCTION_C);
		 
		kernel->appSignals->appReady = true;	
	}
	
	if(i2cInit != 0xFF) 
	{
		if(kernel->tickTimer.delay1ms(&tickTimer2,100))
		{
			if(i2cInit == 0) 
			{
				pin_setOutput(IO_D03,false); // Disable reset 
				i2cInit++;
			}
			else if(i2cInit == 1)
			{
				lightSwitch1.init(&i2c,0x77);
				i2cInit++;
			}
			else if(i2cInit == 2)
			{
				lightSwitch2.init(&i2c,0x75);
				i2cInit++;
			}
			else if(i2cInit == 3)
			{
				i2cInit = 0xFF;
			}
		}
	}

//----- Main -------------------------------------------------------------------------------------------------	
	
	if((kernel->appSignals->appReady == true) && i2cInit == 0xFF)
	{	
		if(kernel->tickTimer.delay1ms(&i2cStepTimer,15))
		{
			if(i2cStep == 0) lightSwitch1.updateButton();
			else if(i2cStep == 1) lightSwitch2.updateButton();	
			else if(i2cStep == 2) lightSwitch1.updateLed();	
			else if(i2cStep == 3) lightSwitch2.updateLed();	
			
			i2cStep ++;
			if(i2cStep > 3) 
			{
				uint16_t buttoneState = 0;
				buttoneState = (lightSwitch1.getButtons() &0x3F);
				buttoneState |= ((uint16_t)lightSwitch2.getButtons()<<6)&0x0FC0;
				sendTriggers(kernel, buttoneState);
				
				for(uint8_t i = 0; i < sizeof(ledState); i++)
				{
					if(i<6)
					{
						if(ledState[i] == srp_state_on) lightSwitch1.setLed(i,true);
						else if(ledState[i] == srp_state_off) lightSwitch1.setLed(i,false);
					}
					else
					{
						if(ledState[i] == srp_state_on) lightSwitch2.setLed(i-6,true);
						else if(ledState[i] == srp_state_off) lightSwitch2.setLed(i-6,false);
					}
				}
				
				if(kernel->tickTimer.delay1ms(&ledToggleTimer,1000))
				{
					for(uint8_t i = 0; i < sizeof(ledState); i++)
					{
						if(i<6) 
						{
							if(ledState[i] == srp_state_transitioning) lightSwitch1.setLed(i,ledToggleState);
						}
						else
						{
							if(ledState[i] == srp_state_transitioning) lightSwitch2.setLed(i-6,ledToggleState);
						}
					}
					ledToggleState = !ledToggleState;
				}
				
				i2cStep = 0;
			}
		}
	}
	
	// App Deinit code
    if(kernel->kernelSignals->shutdownApp)
    {
		//kernel->eemem_write(&ememData[0]);
		kernel->appSignals->shutdownReady = true;
    }
}