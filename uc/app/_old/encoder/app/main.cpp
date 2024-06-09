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
#include "drv/SAMx5x/i2cMaster.h"

#include "Raumsteuerung/valueReportProtocol.h"
#include "Raumsteuerung/triggerProtocol.h"
#include "Raumsteuerung/stateReportProtocol.h"

int main(const kernel_t *kernel);
void onReceive(const kernel_t *kernel, uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size);
kernel_t const *_kernel;

__attribute__((section(".appHeader"))) appHead_t appHead ={
/*appCRC	 */ 0xAABBCCDD, // Will be written by Bootload tool
/*appSize	 */ 0xEEFF0000, // Will be written by Bootload tool
/*appRevMaj	 */ 0x01,
/*appRevMin	 */ 0x00,
/*appName[60]*/ "I2C Encoder",
/*main		 */ main,
/*onRx		 */ onReceive
};


#define LOADSWITCH_ADDRESS 0x02

uint8_t ememData[256];

float mainVolTx;
float mainVolRx;

uint8_t out[4] = {0xAA, 0xCC, 0xFF, 0xFF};
bool update = false;
bool rxVal = false;


uint32_t encoder_readTimer;

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


srp_state_t ampState; 


edgeDetect_t buttonEdgeDetect ={.oldState = false};

const srp_stateReport_t stateReports[] = {
	{LOADSWITCH_ADDRESS, 0x12, &ampState}
};
#define stateReportListSize (sizeof(stateReports)/sizeof(srp_stateReport_t))

const tsp_triggerSlot_t triggerSlotList[] = {
	{UNFILTERED, 0x00, "Write", sendVal },
	{UNFILTERED, 0x01, "Read", readVal }
};
#define  triggerSlotListSize (sizeof(triggerSlotList)/sizeof(tsp_triggerSlot_t))


const tsp_triggerSignal_t triggerSignals[] = {
	{14, LOADSWITCH_ADDRESS, 0x40, "Amp"},
};
#define triggerSignalListSize (sizeof(triggerSignals)/sizeof(tsp_triggerSignal_t))

tsp_itemStatus_t _slotsStatus[triggerSlotListSize];

const triggerSlotProtocol_t triggerSystem = {
	.signals = triggerSignals,
	.signalSize = triggerSignalListSize,
	.slots = triggerSlotList,
	.slotSize = triggerSlotListSize,
	._slotsStatus = &_slotsStatus[0]
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
		case busProtocol_triggerProtocol:		tsp_receiveHandler(kernel, sourceAddress, command,data, size, &triggerSystem);	break;
		case busProtocol_valueReportProtocol:   vrp_receiveHandler(kernel,sourceAddress,command,data,size,&valueReportProtocol); break;
		case busProtocol_stateReportProtocol:	srp_receiveHandler(kernel, sourceAddress, command,data, size, &stateReport);	break;
	}
}

void tsp_onTrigger_callback(const kernel_t *kernel, uint8_t sourceAddress, uint8_t triggerChannelNumber)
{
	tsp_triggerAction(kernel, triggerChannelNumber, triggerSlotList, triggerSlotListSize);
}

void tsp_onStateChange_callback(const kernel_t *kernel)
{
	update = true;
}


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
	
	
uint8_t readData[7];

int main(const kernel_t *kernel)
{
	_kernel = kernel;
	
	// App init Code
	if(kernel->kernelSignals->initApp)
	{
		Reset_Handler();
		
		// Interrupt inputs
		pin_enableInput(IO_D04);
		pin_enableInput(IO_D05);
		pin_enableInput(IO_D06);
		pin_enableInput(IO_D07);
		
		MCLK->APBAMASK.bit.SERCOM1_ = true;
		gclk_peripheralEnable(GCLK_SERCOM1_CORE,kernel->clk_1MHz);
		i2c.init(kernel,SERCOM1,kernel->clk_1MHz,1,false);
		
		kernel->nvic.assignInterruptHandler(SERCOM1_0_IRQn,i2cMbInterrupt);
		kernel->nvic.assignInterruptHandler(SERCOM1_1_IRQn,i2cSbInterrupt);
		kernel->nvic.assignInterruptHandler(SERCOM1_3_IRQn,i2cError);
		
		pin_enablePeripheralMux(IO_D08, PIN_FUNCTION_C);
		pin_enablePeripheralMux(IO_D09, PIN_FUNCTION_C);
		
		kernel->tickTimer.reset(&encoder_readTimer);
		
		kernel->appSignals->appReady = true;
		
	}

	// Main code here
	if(kernel->appSignals->appReady == true)
	{
		tsp_mainHandler(kernel, &triggerSystem);
		
		if(update == true)
		{
			uint8_t step = (uint8_t)(mainVolRx /8);
			
			uint16_t led = 0x00;
			for(uint8_t i; i< step; i++)
			{
				led = led << 1;
				led |= 1;
			}
			led = led << 1; // last led is outside of bar graph
			
			if(ampState == srp_state_on) led  |=  0x01;
			
			out[0] = led & 0xFF;
			out[1] = (led >> 8) & 0xFF;
			
			i2c.transaction(0x02,0x00,&out[0],4,0,0, 0);
			update = false;
		}
		
		if(kernel->tickTimer.delay1ms(&encoder_readTimer, 100) && !i2c.busy())
		{
			i2c.transaction(0x02,0x00,0,0,&readData[0],7, 1);
		}
		
		if(!i2c.busy() && i2c.getCurrentTransactionNumber() == 1)
		{
			i2c.closeTransaction(1);
			
			uint8_t up = readData[4];
			uint8_t down = readData[5];
			
			float value = 0;
			if(up > down) value += up;
			else value -= down;
			
			value *=3;
			
			if(value != 0)
			{
				vrp_sendValueCommmand(kernel,0,vrp_addValue, value);
			}
			
			if(ed_onRising(&buttonEdgeDetect, readData[6]))
			{
				uint8_t indexList[1] = {14};
				tep_sendTriggerByGroup(kernel, &indexList[0], 1, triggerSignals, triggerSignalListSize);
			}
		}
		
	}
	
	// App deinit code
    if(kernel->kernelSignals->shutdownApp)
    {
		kernel->appSignals->shutdownReady = true;
    }
}