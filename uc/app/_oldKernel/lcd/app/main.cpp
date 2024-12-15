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


#include "drv/SAMx5x/spiMaster.h"

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
/*appName[60]*/ "LCD",
/*main		 */ main,
/*onRx		 */ onReceive
};


#define LOADSWITCH_ADDRESS 0x02

uint8_t ememData[256];

float mainVolTx;
float mainVolRx;

bool update = false;
bool rxVal = false;


spiMaster_c spi;

uint32_t sendTimer;

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

const srp_stateReport_t stateReports[] = {

};
#define stateReportListSize (sizeof(stateReports)/sizeof(srp_stateReport_t))

const tsp_triggerSlot_t triggerSlotList[] = {
	{UNFILTERED, 0x00, "Write", sendVal },
	{UNFILTERED, 0x01, "Read", readVal }
};
#define  triggerSlotListSize (sizeof(triggerSlotList)/sizeof(tsp_triggerSlot_t))

const tsp_triggerSignal_t triggerSignals[] = {

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



void spiInterrupt0(void)
{
	spi.interruptHandler_0();
}

void spiInterrupt1(void)
{
	spi.interruptHandler_1();
}

void spiInterrupt2(void)
{
	spi.interruptHandler_2();
}

void spiInterrupt3(void)
{
	spi.interruptHandler_3();
}


void fill(const kernel_t *kernel);
void init(const kernel_t *kernel);

int main(const kernel_t *kernel)
{
	_kernel = kernel;
	
	// App init Code
	if(kernel->kernelSignals->initApp)
	{
		Reset_Handler();
		
		pin_enableOutput(IO_D11); // RS -> Data 1 / Command 0
		pin_enableOutput(IO_D10); // Reset
		
		pin_enableOutput(IO_D14); 
		
		pin_setOutput(IO_D10, true); // Reset Low
		
		spi.init(kernel,SERCOM5,kernel->clk_16MHz,1);
		
		kernel->nvic.assignInterruptHandler(SERCOM5_0_IRQn, spiInterrupt0);
		kernel->nvic.assignInterruptHandler(SERCOM5_1_IRQn, spiInterrupt1);
		kernel->nvic.assignInterruptHandler(SERCOM5_2_IRQn, spiInterrupt2);
		kernel->nvic.assignInterruptHandler(SERCOM5_3_IRQn, spiInterrupt3); //*/
		
		pin_enablePeripheralMux(IO_D12, PIN_FUNCTION_C);
		pin_enablePeripheralMux(IO_D13, PIN_FUNCTION_C);
	//	pin_enablePeripheralMux(IO_D14, PIN_FUNCTION_C);
	//	pin_enablePeripheralMux(IO_D15, PIN_FUNCTION_C);
		
		
		pin_setOutput(IO_D10, true); // Reset High
		
		kernel->appSignals->appReady = true;
		
		kernel->tickTimer.reset(&sendTimer);
		sendTimer += 5000;
		
		init(kernel);
	}

	// Main code here
	if(kernel->appSignals->appReady == true)
	{
		tsp_mainHandler(kernel, &triggerSystem);
		
	//	spi.handler();
		
		if(kernel->tickTimer.delay1ms(&sendTimer, 5000))
		{
			
			fill(kernel);
			
		}
		
		//if(!spi.busy())pin_setOutput(IO_D14,true);
	}
	
	// App deinit code
    if(kernel->kernelSignals->shutdownApp)
    {
		kernel->appSignals->shutdownReady = true;
    }
}

void sendCommand(uint8_t data)
{
	pin_setOutput(IO_D11, false);  // RS -> Data 1 / Command 0
	
	pin_setOutput(IO_D14,false);
	
	spi.send(data);
	
	pin_setOutput(IO_D14,true);
}

void sendData8(uint8_t data)
{	
	pin_setOutput(IO_D11, true);  // RS -> Data 1 / Command 0
	
	pin_setOutput(IO_D14,false);
	
	spi.send(data);
	
	pin_setOutput(IO_D14,true);
}

void sendData32(uint32_t data)
{	
	pin_setOutput(IO_D11, true);  // RS -> Data 1 / Command 0
	
	pin_setOutput(IO_D14,false);
	
	spi.send((data>>24) & 0xFF);
	
	pin_setOutput(IO_D14,true);
	pin_setOutput(IO_D14,false);
	
	spi.send((data>>16) & 0xFF);
		
	pin_setOutput(IO_D14,true);
	pin_setOutput(IO_D14,false);
	
	spi.send((data>>8) & 0xFF);
		
	pin_setOutput(IO_D14,true);
	pin_setOutput(IO_D14,false);
	
	spi.send((data>>0) & 0xFF);
	
	pin_setOutput(IO_D14,true);
}

void sendData24(uint32_t data)
{
	spi.send((data>>16) & 0xFF);
	spi.send((data>>8) & 0xFF);
	spi.send((data>>0) & 0xFF);
}

void init(const kernel_t *kernel)
{
	uint32_t timer;
	sendCommand(lcd_commands_t::LCD_SWRESET);
	kernel->tickTimer.reset(&timer);
	while(!kernel->tickTimer.delay1ms(&timer,150));
	
	sendCommand(lcd_commands_t::LCD_SLPOUT);
	kernel->tickTimer.reset(&timer);
	while(!kernel->tickTimer.delay1ms(&timer,200));
	
	// Set 18 Bit mode
	//sendCommand(lcd_commands_t::LCD_COLMOD);
	//sendData8(0x06);
	
	sendCommand(lcd_commands_t::LCD_MADCTL);
	sendData8(0x40);
	
	sendCommand(lcd_commands_t::LCD_DISPON );
	sendData8(0x06);
	kernel->tickTimer.reset(&timer);
	while(!kernel->tickTimer.delay1ms(&timer,200));
	
}

void fill(const kernel_t *kernel)
{
	lfs_file_t file;
    uint32_t boot_count = 0;
    int err = kernel->file.open(&file, "pic", LFS_O_RDONLY);
	
	if(err != LFS_ERR_OK) return;
	
	kernel->file.seek(&file,0x0A,LFS_SEEK_CUR);
	
	uint32_t dataOffset = 0;
	kernel->file.read(&file,&dataOffset,4);
	
	kernel->file.rewind(&file);
	kernel->file.seek(&file,dataOffset,LFS_SEEK_CUR);
	
	sendCommand(lcd_commands_t::LCD_CASET);
	sendData32(127);
	
	sendCommand(lcd_commands_t::LCD_RASET);
	sendData32(159);
	
	sendCommand(lcd_commands_t::LCD_RAMWR);
	
	pin_setOutput(IO_D11, true);  // RS -> Data 1 / Command 0
	
	pin_setOutput(IO_D14,false);
	
	for(uint32_t i = 0; i< 128*160; i++)
	{
		uint32_t data = 0;
		
		uint8_t b = (data)&0xFF;
		uint8_t g = (data>>8)&0xFF;
		uint8_t r = (data>>16)&0xFF;
		
		kernel->file.read(&file, &b, 1);
		kernel->file.read(&file, &g, 1);
		kernel->file.read(&file, &r, 1);
		
		spi.send(r);
		spi.send(g);
		spi.send(b);
		
		kernel->wdt.clear();
	}
	pin_setOutput(IO_D14,true);
	
	kernel->file.close(&file);
}