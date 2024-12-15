/*
 * app.cpp
 *
 * Created: 20.04.2019 20:05:30
 * Author : Christian
 */ 

#include "sam.h"
#include "common/kernel.h"
#include "common/io_same51jx.h"

#include "driver/SAMx5x/genericClockController.h"
#include "driver/SAMx5x/spiMaster.h"

#include "peripheral/sitronix_ST7565R.h"

#include "protocol/triggerSystemProtocol.h"
#include "protocol/eventSystemProtocol.h"
#include "protocol/stateSystemProtocol.h"
#include "protocol/valueSystemProtocol.h"

spiMaster_c spi;

int main(void);
bool onReceive(uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size);
kernel_t kernel __attribute__((section(".kernelCall")));

__attribute__((section(".appHeader"))) appHead_t appHead ={
/*appCRC	 */ 0xAABBCCDD, // Will be written by Bootload tool
/*appSize	 */ 0xEEFF0000, // Will be written by Bootload tool
/*appRevMaj	 */ 0x01,
/*appRevMin	 */ 0x00,
/*appName[60]*/ "Druckluftüberwachung",
/*main		 */ main,
/*onRx		 */ onReceive
};


//**** Trigger Configuration ******************************************************************************************

const tsp_triggerSignal_t triggerSignals[] = {
};
#define triggerSignalListSize (sizeof(triggerSignals)/sizeof(tsp_triggerSignal_t))

const tsp_triggerSlot_t triggerSlots[] = {
};
#define triggerSlotListSize (sizeof(triggerSlots)/sizeof(tsp_triggerSlot_t))

tsp_itemState_t triggerSignalStateList[triggerSignalListSize];
tsp_itemState_t triggerSlotStateList[triggerSlotListSize];

const triggerSystemProtocol_t triggerSystem = {
	.signals = triggerSignals,
	.signalSize = triggerSignalListSize,
	.slots = triggerSlots,
	.slotSize = triggerSlotListSize,
	._signalState = triggerSignalStateList,
	._slotState = triggerSlotStateList
};


//**** Event Configuration ********************************************************************************************

const esp_eventSignal_t eventSignal[] = {
};
#define eventSignalListSize (sizeof(eventSignal)/sizeof(esp_eventSignal_t))

const esp_eventSlot_t eventSlot[] = {
};
#define eventSlotListSize (sizeof(eventSlot)/sizeof(esp_eventSlot_t))

esp_itemState_t eventSignalStatusList[eventSignalListSize];
esp_itemState_t eventSlotStatusList[eventSlotListSize];

const eventSystemProtocol_t eventSystem = {
	.signals = eventSignal,
	.signalSize = eventSignalListSize,
	.slots = eventSlot,
	.slotSize = eventSlotListSize,
	._signalState = eventSignalStatusList,
	._slotState = eventSlotStatusList
};

//**** State Configuration ********************************************************************************************

const ssp_stateSignal_t stateReportSignal[] = {
};
#define stateReportSignalListSize (sizeof(stateReportSignal)/sizeof(ssp_stateSignal_t))

const ssp_stateSlot_t stateReportSlots[] = {
};
#define stateReportSlotListSize (sizeof(stateReportSlots)/sizeof(ssp_stateSlot_t))

ssp_itemState_t stateReportSignalStatusList[stateReportSignalListSize];
ssp_itemState_t stateReportSlotStatusList[stateReportSlotListSize];

const stateSystemProtocol_t stateReportSystem = {
	.signals = stateReportSignal,
	.signalSize = stateReportSignalListSize,
	.slots = stateReportSlots,
	.slotSize = stateReportSlotListSize,
	._signalState = stateReportSignalStatusList,
	._slotState = stateReportSlotStatusList
};

//**** Value Configuration ********************************************************************************************

const vsp_valueSignal_t valueReportSignal[] = {
};
#define valueReportSignalListSize (sizeof(valueReportSignal)/sizeof(vsp_valueSignal_t))

const vsp_valueSlot_t valueReportSlots[] = {
};
#define valueReportSlotListSize (sizeof(valueReportSlots)/sizeof(vsp_valueSlot_t))

vsp_itemState_t valueReportSignalStatusList[valueReportSignalListSize];
vsp_itemState_t valueReportSlotStatusList[valueReportSlotListSize];

const valueSystemProtocol_t valueReportSystem = {
	.signals = valueReportSignal,
	.signalSize = valueReportSignalListSize,
	.slots = valueReportSlots,
	.slotSize = valueReportSlotListSize,
	._signalState = valueReportSignalStatusList,
	._slotState = valueReportSlotStatusList
};

//**** onReceive Configuration ****************************************************************************************

bool onReceive(uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size)
{
	switch(protocol)
	{
		case busProtocol_triggerSystemProtocol:	return tsp_receiveHandler(&triggerSystem, sourceAddress, command, data, size);
		case busProtocol_eventSystemProtocol:	return esp_receiveHandler(&eventSystem, sourceAddress, command, data, size);
		case busProtocol_stateSystemProtocol:	return ssp_receiveHandler(&stateReportSystem, sourceAddress, command, data, size);
		case busProtocol_valueSystemProtocol:	return vsp_receiveHandler(&valueReportSystem, sourceAddress, command, data, size);
	}
	return false;
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



#define CS_PIN		IO_D14
#define RESET_PIN	IO_D17
#define RS_PIN		IO_D15
#define SCK_PIN		IO_D13
#define SI_PIN		IO_D12

void st7565r_init(void);

int main(void)
{
	// App Init Code
	if(kernel.kernelSignals->initApp)
	{
		Reset_Handler();
		
		pin_enableOutput(CS_PIN);
		pin_enableOutput(SCK_PIN);
		pin_enableOutput(IO_D12);
		pin_enableOutput(RESET_PIN);
		pin_enableOutput(RS_PIN); // RS -> Data 1 / Command 0
		
		pin_setOutput(RESET_PIN, false);
		
		spi.init(SERCOM5, kernel.clk_16MHz,1);
		
		kernel.nvic.assignInterruptHandler(SERCOM5_0_IRQn, spiInterrupt0);
		kernel.nvic.assignInterruptHandler(SERCOM5_1_IRQn, spiInterrupt1);
		kernel.nvic.assignInterruptHandler(SERCOM5_2_IRQn, spiInterrupt2);
		kernel.nvic.assignInterruptHandler(SERCOM5_3_IRQn, spiInterrupt3);
		
		pin_enablePeripheralMux(IO_D12, PIN_FUNCTION_C);
		pin_enablePeripheralMux(IO_D13, PIN_FUNCTION_C);
	//	pin_enablePeripheralMux(IO_D14, PIN_FUNCTION_C);
	//	pin_enablePeripheralMux(IO_D15, PIN_FUNCTION_C);
		
		
		st7565r_init();
		
		tsp_initialize(&triggerSystem);
		esp_initialize(&eventSystem);
		ssp_initialize(&stateReportSystem);
		vsp_initialize(&valueReportSystem);
		
		
		
		kernel.appSignals->appReady = true;
	}

//----- Main -------------------------------------------------------------------------------------------------	
	
	if(kernel.appSignals->appReady == true)
	{	
		tsp_mainHandler(&triggerSystem);
		esp_mainHandler(&eventSystem);
		ssp_mainHandler(&stateReportSystem);
		vsp_mainHandler(&valueReportSystem);
	}
	
	// App deinit code
    if(kernel.kernelSignals->shutdownApp)
    {
		kernel.appSignals->shutdownReady = true;
    }
	
}


void sendCommand(uint8_t data)
{
	pin_setOutput(RS_PIN, false);  // RS -> Data 1 / Command 0
	
	pin_setOutput(CS_PIN,false);
	
	spi.send(data);
	
	pin_setOutput(CS_PIN,true);
}

void sendData8(uint8_t data)
{
	pin_setOutput(RS_PIN, true);  // RS -> Data 1 / Command 0
	
	pin_setOutput(CS_PIN,false);
	
	spi.send(data);
	
	pin_setOutput(CS_PIN,true);
}

void sendData32(uint32_t data)
{
	pin_setOutput(RS_PIN, true);  // RS -> Data 1 / Command 0
	
	pin_setOutput(CS_PIN,false);
	
	spi.send((data>>24) & 0xFF);
	
	pin_setOutput(CS_PIN,true);
	pin_setOutput(CS_PIN,false);
	
	spi.send((data>>16) & 0xFF);
	
	pin_setOutput(CS_PIN,true);
	pin_setOutput(CS_PIN,false);
	
	spi.send((data>>8) & 0xFF);
	
	pin_setOutput(CS_PIN,true);
	pin_setOutput(CS_PIN,false);
	
	spi.send((data>>0) & 0xFF);
	
	pin_setOutput(CS_PIN,true);
}

void sendData24(uint32_t data)
{
	spi.send((data>>16) & 0xFF);
	spi.send((data>>8) & 0xFF);
	spi.send((data>>0) & 0xFF);
}

void st7565r_init(void)
{
	uint32_t timer;
	
	// reset
	pin_setOutput(RESET_PIN, false);
	kernel.tickTimer.reset(&timer);
	while(!kernel.tickTimer.delay1ms(&timer,10));
	pin_setOutput(RESET_PIN, true);
	
	
	sendCommand(st7565r_commands_t::ST7565R_on);

	sendCommand(st7565r_commands_t::ST7565R_lcdBias1d9);
	sendCommand(st7565r_commands_t::ST7565R_adcSelectNormal);
	sendCommand(st7565r_commands_t::ST7565R_commonOutputModeNormal);
	
	sendCommand(st7565r_commands_t::ST7565R_voltageRegulatorInternalResistorRatio +5);
	
	sendCommand(st7565r_commands_t::ST7565R_powerControl+7);
	
	sendCommand(st7565r_commands_t::ST7565R_electronicVolumeMode);
	sendCommand(0x20);
	
	
	
	kernel.tickTimer.reset(&timer);
	while(!kernel.tickTimer.delay1ms(&timer,1));
	
	sendCommand(st7565r_commands_t::ST7565R_page0);
	sendCommand(st7565r_commands_t::ST7565R_line0);
	
	for(uint32_t i = 0; i< 64; i++){
		sendData8(0xA1);
	}
	
	//sendCommand(st7565r_commands_t::ST7565R_page0+1);
	//sendCommand(st7565r_commands_t::ST7565R_line0+1);
	
	for(uint32_t i = 0; i< 63; i++){
		sendData8(0x1A);
	}
	sendData8(0xFF);

	sendCommand(st7565r_commands_t::ST7565R_on);
	
}
/*
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
}//*/