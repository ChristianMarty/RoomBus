/*
 * kernel.cpp
 *
 * Created: 19.04.2019 18:52:57
 * Author : Christian
 */ 

#define APP_START_ADDR 0x10000

#include "sam.h"

#include "protocol/deviceManagementProtocol.h"
#include "protocol/messageLogProtocol.h"
#include "protocol/fileTransferProtocol.h"

#include "kernel/bus.h"
#include "kernel/kernel.h"
#include "kernel/standardIO.h"
#include "kernel/systemControl.h"
#include "kernel/dataSystem.h"

#include "drv/SAMx5x/sysTickTimer.h"
#include "drv/SAMx5x/interrupt.h"
#include "drv/SAMx5x/system.h"
#include "drv/SAMx5x/pin.h"
#include "drv/SAMx5x/uart.h"
#include "drv/SAMx5x/genericClockController.h"
#include "drv/SAMx5x/rand.h"
#include "drv/SAMx5x/wdt.h"
#include "drv/SAMx5x/dma.h"
#include "drv/SAMx5x/eeprom.h"

#include "utility/string.h"
#include "utility/softCRC.h"
#include "utility/cobs.h"

uint32_t ledTimer;
uint32_t greenLedTimer;
uint32_t redLedTimer;

uint32_t sysControlTaskTick;
bool led = false;

bool ledRed;
bool ledGreen;
bool ledBlue;

volatile uint8_t eememSysSavedSettings EEPROM;
volatile uint8_t eememAppData[256] EEPROM;

appHead_t *appHead = (appHead_t*)(APP_START_ADDR);
 
sysStatusData_t sysStatus;
sysControlData_t sysControl;

sysSavedSettings_t sysSavedSettings;

systemControl_t sysControlHandler ={.sysControl = &sysControl, .sysStatus = &sysStatus};

kernelSignals_t kernelSignals;
appSignals_t appSignals;

kernel_t kernel;

appState_t appState;

uint32_t appCrcCalc = 0;

lfs_t lfs;

bool appDbgMode;

app_benchmark_t appBenchmark;

int file_open(lfs_file_t *file, const char *path, int flags)
{
	return lfs_file_open(&lfs, file, path, flags);
}

int file_close(lfs_file_t *file)
{
	return lfs_file_close(&lfs, file);
}

lfs_ssize_t file_read(lfs_file_t *file, void *buffer, lfs_size_t size)
{
	return lfs_file_read(&lfs, file, buffer, size);
}

lfs_ssize_t file_write(lfs_file_t *file, const void *buffer, lfs_size_t size)
{
	return lfs_file_write(&lfs, file, buffer, size);
}

lfs_soff_t file_seek(lfs_file_t *file, lfs_soff_t off, int whence)
{
	return lfs_file_seek(&lfs, file, off, whence);
}

int file_rewind(lfs_file_t *file)
{
	return lfs_file_rewind(&lfs, file);
}

lfs_soff_t file_size(lfs_file_t *file)
{
	return lfs_file_size(&lfs, file);
}



uint32_t appCRC(void)
{
	PAC->WRCTRL.reg = 0x00010021; // Unlock
	
	DSU->STATUSA.bit.DONE = 1;
	
	DSU->DATA.reg = 0xFFFFFFFF;
	DSU->ADDR.reg = (APP_START_ADDR+4); //(appHead->appSize);
	DSU->LENGTH.reg = appHead->appSize;
	
	DSU->CTRL.bit.CRC = true;
	
	while(DSU->STATUSA.bit.DONE == 0);
	
	return DSU->DATA.reg;
}

bool checkAppValid(void)
{
	appDbgMode = false;
	if((appHead->appSize == 0xEEFF0000)&&(appHead->appCRC == 0xAABBCCDD))
	{
		dmp_setAppCrc(0xAABBCCDD);
		appDbgMode = true;
		return true; // For debug 
	}
	
	if((appHead->appSize != 0xFFFFFFFF)&&(appHead->appCRC != 0xFFFFFFFF)) // If App is not empty
	{
		uint32_t crc = appCRC();
		if(appHead->appCRC == crc)
		{
			 dmp_setAppCrc(crc);
			 return true;
		}
	}
	
	dmp_setAppCrc(0xFFFFFFFF);
	return false;
}

void bus_onTransmitt(void)
{
	sysTick_resetDelayCounter(&redLedTimer);
	ledRed = true;
}

void emem_write(uint8_t *data)
{
	eeprom_writeArray(data, &eememAppData[0], sizeof(eememAppData));
}

void emem_read(uint8_t *data)
{
	eeprom_readArray(data, &eememAppData[0], sizeof(eememAppData));
}

uint8_t watchdogWarningConter = 0;
#define WATCHDOG_MAX 10

void wdt_earlyWarningHandler(void)
{
	watchdogWarningConter ++;
	
	systemControl_setWatchdogWarning(&sysControlHandler);
	
	char msg[] = "Watchdog Warning:    ";
	string_fromInt32(watchdogWarningConter,0,&msg[18]);
	mlp_sysWarning(msg);
	
	WDT->INTFLAG.bit.EW = true;
	
	wdt_clear(); // TODO: Makes sense?
	
	if(watchdogWarningConter >= WATCHDOG_MAX) 
	{
		// Turn off app auto start
		systemControl_interveneStartup(&sysControlHandler);
		eeprom_writeByte( systemControl_getSaveStting(&sysControlHandler).reg , &eememSysSavedSettings);

		system_reboot();
	}
}


int main(void)
{
	system_configure();
	
	system_init();
	sysTick_init(120000000);
	stdIO_init();
	
	nvic_init();
	nvic_assignInterruptHandler(SysTick_IRQn,sysTick_interruptHandler);
	nvic_assignInterruptHandler(CAN0_IRQn,bus_interrupt);
	
	wdt_init(wdt_earlyWarningHandler);
	
	kernel.clk_1MHz = gclk_getFreeGenerator();
	gclk_generatorEnable(kernel.clk_1MHz,GCLK_SRC_XOSC1,16);
	
	kernel.clk_16MHz = gclk_getFreeGenerator();
	gclk_generatorEnable(kernel.clk_16MHz,GCLK_SRC_XOSC1,1);
	
	gclk_peripheralEnable(GCLK_SERCOM2_CORE,kernel.clk_16MHz);
	MCLK->APBBMASK.bit.SERCOM2_ = true;

	rand_init();
	
	//Can
	
	// TODO: ADD "promiscuous mode" for can bus
	gclk_peripheralEnable(GCLK_CAN0,kernel.clk_16MHz);
	bus_init(dmp_getDeviceAddress());
	
	pin_enableOutput(PIN_PORT_B,9);
	pin_enablePeripheralMux(PIN_PORT_A,22,PIN_FUNCTION_I);
	pin_enablePeripheralMux(PIN_PORT_A,23,PIN_FUNCTION_I);
	
	dmp_init(&appHead->appName[0], &sysControl, &sysStatus, &appState, &appBenchmark);
	dmp_sendExtendedHeartbeat();
	
	ftp_init();
	
	sysSavedSettings.reg = eeporm_readByte(&eememSysSavedSettings);
	systemControl_init(&sysControlHandler,&sysSavedSettings);
	
	sysTick_resetDelayCounter(&sysControlTaskTick);
	
	if(RSTC->RCAUSE.reg & 0x20) // WDT
	{
		systemControl_setWatchdogError(&sysControlHandler);
		mlp_sysError("System restart after watchdog timeout");
		systemControl_interveneStartup(&sysControlHandler);
		mlp_appWarning("Run on startup disabled");
	}
	
	if(RSTC->RCAUSE.bit.BODCORE) mlp_sysError("System restart after Core Brown Out");
	if(RSTC->RCAUSE.bit.BODVDD) mlp_sysError("System restart after VDD Brown Out");
	if(RSTC->RCAUSE.bit.NVM) mlp_sysError("NVM Reset");
	if(RSTC->RCAUSE.bit.EXT) mlp_sysMessage("External Reset");
	if(RSTC->RCAUSE.bit.POR) mlp_sysMessage("System restart after power on");
	if(RSTC->RCAUSE.bit.SYST) mlp_sysMessage("System Reset Request");
	if(RSTC->RCAUSE.bit.BACKUP) mlp_sysMessage("Backup Reset");
	
	
	uint32_t boot_count =  dataSystem_init(&lfs);	
	
	kernel.tickTimer.reset = sysTick_resetDelayCounter;
	kernel.tickTimer.delay1ms = sysTick_delay1ms;
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
	kernel.wdt.clear = wdt_clear;
	
	kernel.file.open = file_open;
	kernel.file.close = file_close;
	kernel.file.read = file_read;
	kernel.file.write = file_write;
	kernel.file.seek = file_seek;
	kernel.file.rewind = file_rewind;
	kernel.file.size = file_size;
	
	kernel.appSignals = &appSignals;
	kernel.kernelSignals = &kernelSignals;
	
/*	fuseUserPage_t data = fuse_getUserPage();
	
	char Fuse0str[] = "Fuse 0 0xFFFFFFFF";
	char Fuse1str[] = "Fuse 1 0xFFFFFFFF";
	char Fuse2str[] = "Fuse 2 0xFFFFFFFF";
	string_uInt32ToHex(data.byte[0], &Fuse0str[9]);
	string_uInt32ToHex(data.byte[1], &Fuse1str[9]);
	string_uInt32ToHex(data.byte[2], &Fuse2str[9]);
	
	mlp_sysMessage(Fuse0str);
	mlp_sysMessage(Fuse1str);
	mlp_sysMessage(Fuse2str);
	
	fuse_setDefault();
	
	fuseUserPage_t data2 = fuse_getUserPage();
	
	char Fuse3str[] = "Fuse 0 0xFFFFFFFF";
	char Fuse4str[] = "Fuse 1 0xFFFFFFFF";
	char Fuse5str[] = "Fuse 2 0xFFFFFFFF";
	string_uInt32ToHex(data2.byte[0], &Fuse3str[9]);
	string_uInt32ToHex(data2.byte[1], &Fuse4str[9]);
	string_uInt32ToHex(data2.byte[2], &Fuse5str[9]);
	
	mlp_sysMessage(Fuse3str);
	mlp_sysMessage(Fuse4str);
	mlp_sysMessage(Fuse5str);*/
	
	
	// App init
	systemControl_setAppCrcError(&sysControlHandler, !checkAppValid());
	appState = APP_STOP;

	char bootCountStr[] = "Boot Counter:     ";
	string_fromInt32(boot_count,0,&bootCountStr[14]);
	mlp_sysMessage(bootCountStr);
	mlp_sysMessage("Boot process completed");
	
	bool identifyToggle = false;
	
    while (1) 
    {
		wdt_clear();
		
		if(systemControl_hasChanged(&sysControlHandler))
		{
			systemControl_run(&sysControlHandler);
			systemControl_hasChanged(&sysControlHandler); // run again to not trigger twice after systemControl_run
			
			ledBlue = sysStatus.bit.setupModeEn;
			dmp_sendHeartbeat();
			
			eeprom_writeByte( systemControl_getSaveStting(&sysControlHandler).reg , &eememSysSavedSettings);
		}
		
		// 500ms Task
		if(sysTick_delay1ms(&sysControlTaskTick,500))
		{	
			if(watchdogWarningConter > 0) watchdogWarningConter--;
			
			systemControl_setAppCrcError(&sysControlHandler, !checkAppValid());
			
			kernel.kernelSignals->rootMode = sysStatus.bit.setupModeEn;
				
			if(sysStatus.bit.identify)
			{
				identifyToggle = !identifyToggle;
				stdIO_setLedRGB(identifyToggle, identifyToggle, identifyToggle);
			}
		}
		
		if( systemControl_getAppRun(&sysControlHandler))
		{	
			if(appState == APP_STOP) 
			{
				mlp_sysMessage("App Start");
				appState = APP_INIT;
			}
		}
		else
		{
			if(appState == APP_RUN)
			{
				mlp_sysMessage("App Shutdown");
				appState = APP_SHUTDOWN;
			}
		}
		
		if(appState == APP_INIT)
		{
			if(checkAppValid()) 
			{
				kernel.kernelSignals->initApp = true;
			}
			else 
			{
				appState = APP_STOP;
				mlp_sysError("App CRC Error");
				kernel.kernelSignals->initApp = false;
			}
			
			kernel.appSignals->appReady = false;
			kernel.appSignals->shutdownReady = false;
			kernel.appSignals->appError = false;
			kernel.kernelSignals->shutdownApp = false;
		}
		else if(appState == APP_RUN)
		{
			kernel.kernelSignals->initApp = false;
			kernel.kernelSignals->shutdownApp = false;
		}
		else if(appState == APP_SHUTDOWN)
		{
			kernel.kernelSignals->initApp = false;
			kernel.kernelSignals->shutdownApp = true;
		}
		
		if((appState == APP_INIT)||(appState == APP_RUN)||(appState == APP_SHUTDOWN))
		{
			sysStatus.bit.appRuning  = true;
			
			volatile uint16_t startTick = sysTick_getTick_us();
			volatile uint32_t startTime = sysTick_getTickTime();
			
			appHead->appRun(&kernel);
			
			volatile uint32_t endTime = sysTick_getTickTime();
			volatile uint16_t endTick = sysTick_getTick_us();
			
			appBenchmark.tick_last = (uint16_t)(endTick-startTick);
			appBenchmark.ms_last   = (uint16_t)(endTime-startTime);
			
			if(appBenchmark.tick_last >= 1000) 
			{
				appBenchmark.tick_last = (1000-startTick)+endTick;
				if(appBenchmark.ms_last > 0) appBenchmark.ms_last--;
			}
			
			benchmark_run(&appBenchmark);
		}
		else
		{
			sysStatus.bit.appRuning = false;	
		}
		
		if(kernel.appSignals->appReady==true)
		{
			appState = APP_RUN;
		}
		
		if(kernel.appSignals->shutdownReady == true)
		{
			appState = APP_STOP;				
		}
		
		dmp_handler();
		ftp_handler();
		
		
		if(sysTick_delay1ms(&redLedTimer,10))
		{
			ledRed = false;
		}
		
		if(sysTick_delay1ms(&greenLedTimer,10))
		{
			ledGreen = false;
		}

		if(!bus_rxBufferEmpty())
		{
			sysTick_resetDelayCounter(&greenLedTimer);
			
			uint8_t rxIndex = bus_getRxFrameIndex();
			can_rxFrame_t *rx = bus_getRxFrame(rxIndex);
				
			uint8_t srcAddr  = bus_getSourceAddress(rx);
			uint8_t destAddr = bus_getDestinationAddress(rx);
			busProtocol_t protocol = bus_getProtocol(rx);
			uint8_t command  = bus_getCommand(rx);

			ledGreen = true;
			uint8_t length = bus_getRxDataSize(rx);
				
			switch(protocol)
			{
				case busProtocol_deviceManagementProtocol:	dmp_receiveHandler(srcAddr, command, &rx->data[0], length);
															break;
															
				case busProtocol_fileTransferProtocol:		ftp_receiveHandler(&lfs, srcAddr, command, &rx->data[0], length);
															break;
													
				default:									if((appState == APP_INIT)||(appState == APP_RUN)||(appState == APP_SHUTDOWN))
															{
																appHead->onRx(&kernel, srcAddr, protocol, command, &rx->data[0], length);
															}
			}
		
				
			bus_freeRxFrame(rxIndex);
		}
		
		if(!sysControl.bit.identify)
		{
			if(sysControl.bit.ledOnOff) 
			{
				stdIO_setLedRGB(ledRed,ledGreen,ledBlue);
				kernel.kernelSignals->ledDisabled = false;
			}
			else 
			{
				stdIO_setLedRGB(false,false,false);
				kernel.kernelSignals->ledDisabled = true;;
			}
		}	
    }
}
