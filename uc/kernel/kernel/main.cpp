//**********************************************************************************************************************
// FileName : main.cpp
// FilePath : 
// Author   : Christian Marty
// Date		: 12.01.2019
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#define KERNEL_H_

#include "main.h"
#include "common/kernel.h"

#include "protocol/kernel/deviceManagementProtocol.h"
#include "protocol/kernel/messageLogProtocol.h"
#include "protocol/kernel/fileTransferProtocol.h"

#include "kernel/bus.h"
#include "kernel/userIo.h"
#include "kernel/systemControl.h"
#include "kernel/dataSystem.h"

#include "driver/SAMx5x/kernel/tickTimer.h"
#include "driver/SAMx5x/kernel/watchDogTimer.h"
#include "driver/SAMx5x/kernel/interrupt.h"
#include "driver/SAMx5x/kernel/random.h"
#include "driver/SAMx5x/kernel/system.h"
#include "driver/SAMx5x/kernel/eeprom.h"

#include "driver/SAMx5x/pin.h"
#include "driver/SAMx5x/uart.h"
#include "driver/SAMx5x/genericClockController.h"

#include "driver/SAMx5x/dma.h"

#include "utility/string.h"
#include "utility/softCRC.h"
#include "utility/cobs.hpp"


kernel_t kernel __attribute__((section(".kernelCall")));
applicationHeader_t *appHead = (applicationHeader_t*)(APP_START_ADDR);

extern eememData_t eememData;

systemControl_t sysControlHandler;

kernelSignals_t kernelSignals;

appSignals_t appSignals;

lfs_t lfs;

int file_open(lfs_file_t *file, const char *path, int flags){
	return lfs_file_open(&lfs, file, path, flags);
}
int file_close(lfs_file_t *file){
	return lfs_file_close(&lfs, file);
}
lfs_ssize_t file_read(lfs_file_t *file, void *buffer, lfs_size_t size){
	return lfs_file_read(&lfs, file, buffer, size);
}
lfs_ssize_t file_write(lfs_file_t *file, const void *buffer, lfs_size_t size){
	return lfs_file_write(&lfs, file, buffer, size);
}
lfs_soff_t file_seek(lfs_file_t *file, lfs_soff_t off, int whence){
	return lfs_file_seek(&lfs, file, off, whence);
}
int file_rewind(lfs_file_t *file){
	return lfs_file_rewind(&lfs, file);
}
lfs_soff_t file_size(lfs_file_t *file){
	return lfs_file_size(&lfs, file);
}

void bus_onTransmitt(void){
	userIo_txLed();
}

void emem_write(uint8_t *data){
	eeprom_writeArray(data, &eememData.appData[0], sizeof(eememData.appData));
}
void emem_read(uint8_t *data){
	eeprom_readArray(data, &eememData.appData[0], sizeof(eememData.appData));
}

uint8_t watchdogWarningConter = 0;
#define WATCHDOG_MAX 10

void wdt_earlyWarningHandler(void)
{
	watchdogWarningConter ++;
	
	systemControl_setWatchdogWarning();
	
	char msg[] = "Watchdog Warning:    ";
	string_fromInt32(watchdogWarningConter, 0, &msg[18]);
	mlp_sysWarning(msg);
	
	WDT->INTFLAG.bit.EW = true;
	
	watchDogTimer_clear(); // TODO: Makes sense?
	
	if(watchdogWarningConter >= WATCHDOG_MAX) {
		systemControl_setWatchdogError(); // Turn off app auto start
		system_reboot();
	}
}

void reportBootReason(void)
{
	if(RSTC->RCAUSE.reg & 0x20) // WDT
	{
		systemControl_setWatchdogError();
		mlp_sysError("System restart after watchdog timeout");
		mlp_appWarning("Run on startup disabled");
	}
		
	if(RSTC->RCAUSE.bit.BODCORE) mlp_sysError("System restart after Core Brown Out");
	if(RSTC->RCAUSE.bit.BODVDD) mlp_sysError("System restart after VDD Brown Out");
	if(RSTC->RCAUSE.bit.NVM) mlp_sysError("NVM Reset");
	if(RSTC->RCAUSE.bit.EXT) mlp_sysMessage("External Reset");
	if(RSTC->RCAUSE.bit.POR) mlp_sysMessage("System restart after power on");
	if(RSTC->RCAUSE.bit.SYST) mlp_sysMessage("System Reset Request");
	if(RSTC->RCAUSE.bit.BACKUP) mlp_sysMessage("Backup Reset");
}

void initializeKernelStruct(void)
{
	kernel.tickTimer.reset = tickTimer_reset;
	kernel.tickTimer.delay1ms = tickTimer_delay1ms;
	
	kernel.bus.getMessageSlot = bus_getMessageSlot;
	kernel.bus.pushByte = bus_pushByte;
	kernel.bus.pushWord16 = bus_pushWord16;
	kernel.bus.pushWord24 = bus_pushWord24;
	kernel.bus.pushWord32 = bus_pushWord32;
	kernel.bus.pushArray = bus_pushArray;
	kernel.bus.pushString = bus_pushString;
	kernel.bus.send = bus_send;
	kernel.bus.writeHeader = bus_writeHeader;
	
	kernel.nvic.assignInterruptHandler = nvic_assignInterruptHandler;
	kernel.nvic.removeInterruptHandler = nvic_removeInterruptHandler;
	kernel.nvic.setInterruptPriority   = nvic_setInterruptPriority;
	
	kernel.eemem.read = emem_read;
	kernel.eemem.write = emem_write;
	
	kernel.log.message = mlp_appMessage;
	kernel.log.warning = mlp_appWarning;
	kernel.log.error = mlp_appError;
	
	kernel.rand.rand = rand_getRand;
	kernel.rand.inRange = rand_getRandRange;
	
	kernel.wdt.clear = watchDogTimer_clear;
	
	kernel.file.open = file_open;
	kernel.file.close = file_close;
	kernel.file.read = file_read;
	kernel.file.write = file_write;
	kernel.file.seek = file_seek;
	kernel.file.rewind = file_rewind;
	kernel.file.size = file_size;
	
	kernel.appSignals = &appSignals;
	kernel.kernelSignals = &kernelSignals;	
}

void busMessageHandler(void)
{
	if(bus_rxBufferEmpty()) return;

	uint8_t rxIndex = bus_getRxFrameIndex();
	can_rxFrame_t *rxFrame = bus_getRxFrame(rxIndex);
	
	bus_rxMessage_t rxMessage = {
		.sourceAddress = bus_getSourceAddress(rxFrame),
		.protocol = bus_getProtocol(rxFrame),
		.command = bus_getCommand(rxFrame),
		.data = &rxFrame->data[0],
		.dataLength = bus_getRxDataSize(rxFrame),
		.broadcast = (bus_getSourceAddress(rxFrame) == BUS_BROADCAST_ADDRESS)
	};

	bool processed = false;
	
	switch(rxMessage.protocol)
	{
		case busProtocol_deviceManagementProtocol:
			processed = dmp_receiveHandler(&rxMessage);
			break;
		
		case busProtocol_fileTransferProtocol:
			processed = ftp_receiveHandler(&lfs, &rxMessage);
			break;
		
		default:
			if(systemControl_appIsActive()){
				processed = appHead->onRx(&rxMessage);
			}
			break;
	}
	
	bus_freeRxFrame(rxIndex);
	
	if(processed){
		userIo_rxLed();
	}
}

int main(void)
{
	system_initialize();
	systemControl_initialize();
	
	nvic_initialize();
	userIo_initialize();
	tickTimer_initialize(120000000);
	watchDogTimer_initialize(wdt_earlyWarningHandler);
	rand_initialize();
	
	kernel.clk_1MHz = gclk_getFreeGenerator();
	gclk_generatorEnable(kernel.clk_1MHz, GCLK_SRC_XOSC1, 16);
	
	kernel.clk_16MHz = gclk_getFreeGenerator();
	gclk_generatorEnable(kernel.clk_16MHz, GCLK_SRC_XOSC1, 1);

	gclk_peripheralEnable(GCLK_CAN0, kernel.clk_16MHz);
	bus_initialize(dmp_getDeviceAddress(), false);
	pin_enablePeripheralMux(PIN_PORT_A, 22, PIN_FUNCTION_I);
	pin_enablePeripheralMux(PIN_PORT_A, 23, PIN_FUNCTION_I);
	
	dmp_initialize(&appHead->appName[0], &sysControlHandler);
	dmp_sendExtendedHeartbeat();
	
	ftp_initialize();
	

	initializeKernelStruct();

	reportBootReason();
	dataSystem_initialize(&lfs);
	
	mlp_sysMessage("Boot process completed");

    while (1) 
    {
		watchDogTimer_clear();
		userIo_handler();
		dmp_handler();
		ftp_handler();
		systemControl_handler();
		busMessageHandler();
    }
}
