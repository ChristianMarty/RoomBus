//**********************************************************************************************************************
// FileName : deviceManagementProtocol.cpp
// FilePath : protocol/
// Author   : Christian Marty
// Date		: 09.06.2024
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************

#include "protocol/deviceManagementProtocol.h"
#include "driver/SAMx5x/tickTimer.h"
#include "driver/SAMx5x/eeprom.h"
#include "driver/SAMx5x/flash.h"
#include "driver/SAMx5x/system.h"
#include "utility/string.h"
#include "kernel/firmwareUpdate.h"
#include "utility/pack.h"


#include <main.h>

#ifdef __cplusplus
extern "C" {
#endif


#define RESPONSE_PRIORITY busPriority_low

#define SETUP_MODE_KEY 0x1234

#define APP_START_ADDRESS (NVMCTRL_BLOCK_SIZE*4)
#define APP_END_ADDRESS (NVMCTRL_BLOCK_SIZE*64)

uint8_t HARDWARE_NAME[] =  KERNEL_HARDWARE_NAME;
uint8_t HARDWARE_NAME_LENGTH  =(sizeof(HARDWARE_NAME)-1);

volatile uint16_t heartbeatInterval EEPROM = 10;
volatile uint16_t extendedHeartbeatInterval EEPROM = 60;

volatile uint8_t deviceAddress EEPROM = 0;

#define DEVICE_NAME_MAX_SIZE 60


volatile uint8_t deviceName[DEVICE_NAME_MAX_SIZE] EEPROM = {"New Device"};
volatile uint8_t deviceNameLength EEPROM = 11;

uint16_t heartbeatIntervalTime;
uint16_t extendedHeartbeatIntervalTime; 

uint32_t heartbeatTimer = 0;
uint32_t extendedHeartbeatTimer = 0;

void sendHardwareName(void);
void sendApplicationName(void);
void sendDeviceName(void);
void writeHeartbeatSettings(uint8_t *data, uint8_t size);
void writeControl(uint8_t *data, uint8_t size);
void setControl(uint8_t *data, uint8_t size);
void clrControl(uint8_t *data, uint8_t size);
void enterRootMode(uint8_t *data, uint8_t size);
void writeDeviceName(uint8_t *data, uint8_t size);
void writeAddress(uint8_t sourceAddress, uint8_t *data, uint8_t size);
void sendDiagnosticsReport(void);
void echo(uint8_t sourceAddress, uint8_t *data, uint8_t size);
void reboot(uint8_t sourceAddress, uint8_t *data, uint8_t size);

uint32_t appCRC;
uint32_t appStartAddress;

sysControlData_t *sysControlPtr;
sysStatusData_t *sysStatusPtr;
appState_t *appStatePtr;
uint8_t *appNamePtr;
uint8_t appNameSize;

app_benchmark_t *benchmark;

uint8_t protocolTyp = busProtocol_deviceManagementProtocol;

void dmp_init(uint8_t *appName, sysControlData_t *sysControl, sysStatusData_t *sysStatus, appState_t *appState, app_benchmark_t *appBenchmark)
{
	sysControlPtr = sysControl;
	sysStatusPtr = sysStatus;
	appStatePtr = appState;
	appNamePtr = appName;
	benchmark = appBenchmark;
	
	appStartAddress = 0x10000;
	
	heartbeatIntervalTime = eeporm_readWord(&heartbeatInterval);
	extendedHeartbeatIntervalTime = eeporm_readWord(&extendedHeartbeatInterval);
	
	if(heartbeatIntervalTime == 0) heartbeatIntervalTime = 10;  
	if(extendedHeartbeatIntervalTime == 0) extendedHeartbeatIntervalTime = 600;
}

void dmp_handler(void)
{
	if(tickTimer_delay1ms(&heartbeatTimer,(heartbeatIntervalTime*1000)))
	{
		dmp_sendHeartbeat();
	}
	
	if(tickTimer_delay1ms(&extendedHeartbeatTimer,(extendedHeartbeatIntervalTime*10000)))
	{
		dmp_sendExtendedHeartbeat();
	} 
	
	fu_handler(sysControlPtr, sysStatusPtr, appStatePtr);
}

uint8_t dmp_getDeviceAddress(void)
{
	uint8_t tmp = eeporm_readByte(&deviceAddress);
	if(tmp >= 127) tmp = 0; // reset address to 0 if it is out of range: e.g. for first startup
	return tmp;
}

bool dmp_setupMode(void)
{
	return (sysStatusPtr->bit.setupModeEn);
}

void dmp_setAppCrc(uint32_t crc)
{
	appCRC = crc;
}

uint32_t dmp_getAppCrc(void)
{
	return appCRC;
}

void dmp_receiveHandler(uint8_t sourceAddress, uint8_t command, uint8_t *data, uint8_t size)
{	
	switch(data[0])
	{
		// Non setup mode commands 
		case dmp_cmdSystemInfoRequest:		dmp_sendExtendedHeartbeat();
											break;
		
		case dmp_cmdHeartbeatRequest:		dmp_sendHeartbeat();
											break;
												
		case dmp_cmdHeartbeatSettings:		writeHeartbeatSettings(&data[1],size-1);
											break;
												
		case dmp_cmdWriteControl:			writeControl(&data[1],size-1);
											break;
												
		case dmp_cmdSetControl:				setControl(&data[1],size-1);
											break;
												
		case dmp_cmdClearControl:			clrControl(&data[1],size-1);
											break;
												
		case dmp_cmdEnterRootMode:			enterRootMode(&data[1],size-1);
											break;
		
		case dmp_canDiagnosticsRequest:		sendDiagnosticsReport();
											break;
		
		case dmp_echo:						echo(sourceAddress, &data[1],size-1);
	}
	
	if(sysStatusPtr->bit.setupModeEn )
	{
		switch(data[0])
		{
			// Root mode commands
			case dmp_cmdSetDeviceName:			writeDeviceName(&data[1],size-1);
												break;																					
		
			case dmp_cmdReboot:					reboot(sourceAddress, &data[1],size-1);
												break;
									
			case dmp_cmdSetAddress:				writeAddress(sourceAddress, &data[1],size-1);
												break;
												
			case dmp_cmdEraseApp:				fu_eraseApp(sourceAddress, &data[1],size-1);	
												break;
												
			case dmp_cmdBtl:					fu_writeAppData(sourceAddress, &data[1],size-1);
												break;
		}
	}
	
}

void dmp_sendHeartbeat(void)
{
	bus_message_t msg;
	if(bus_getMessageSlot(&msg))
	{
		uint32_t status = *(uint32_t*)sysStatusPtr;
		
		bus_writeHeader(&msg,BUS_BROADCAST_ADDRESS, busProtocol_deviceManagementProtocol, 0, RESPONSE_PRIORITY);
		bus_pushByte(&msg,dmp_cmdHeartbeat);
		bus_pushWord32(&msg, status); 
		bus_send(&msg);
		
		tickTimer_reset(&heartbeatTimer);
	}
}

void dmp_sendExtendedHeartbeat(void)
{
	dmp_sendSystemInfo();
	sendHardwareName();
	sendApplicationName();
	sendDeviceName();
}

void dmp_sendSystemInfo(void)
{
	uint32_t status = sysStatusPtr->reg;

	bus_message_t msg;
	if(bus_getMessageSlot(&msg))
	{
		bus_writeHeader(&msg,BUS_BROADCAST_ADDRESS, busProtocol_deviceManagementProtocol, 0, RESPONSE_PRIORITY);
		bus_pushByte(&msg, dmp_cmdSystemInfo);
		bus_pushWord32(&msg, status); 
		bus_pushByte(&msg, HARDWARE_VERSION_MAJOR);
		bus_pushByte(&msg, HARDWARE_VERSION_MINOR);
		bus_pushByte(&msg, KERNEL_VERSION_MAJOR);
		bus_pushByte(&msg, KERNEL_VERSION_MINOR);
		bus_pushWord16(&msg, heartbeatInterval);
		bus_pushWord16(&msg, extendedHeartbeatInterval);
		bus_pushWord32(&msg, appCRC); 
		bus_pushWord32(&msg, appStartAddress); 
		bus_pushWord32(&msg, DSU->DID.reg); 
		bus_pushWord32(&msg, *(uint32_t*)0x008061FC); //Serial Number Word 0
		bus_pushWord32(&msg, *(uint32_t*)0x00806010); //Serial Number Word 1
		bus_pushWord32(&msg, *(uint32_t*)0x00806014); //Serial Number Word 2
		bus_pushWord32(&msg, *(uint32_t*)0x00806018); //Serial Number Word 3
		bus_send(&msg);
		
		tickTimer_reset(&extendedHeartbeatTimer);
	}
}

void sendHardwareName(void)
{
	bus_message_t msg;
	if(bus_getMessageSlot(&msg))
	{
		bus_writeHeader(&msg,BUS_BROADCAST_ADDRESS, busProtocol_deviceManagementProtocol, 0, RESPONSE_PRIORITY);
		bus_pushByte(&msg,dmp_cmdHardwareName);
		bus_pushArray(&msg, &HARDWARE_NAME[0],HARDWARE_NAME_LENGTH);
		bus_send(&msg);
	}
}

void sendApplicationName(void)
{
	bus_message_t msg;
	if(bus_getMessageSlot(&msg))
	{
		bus_writeHeader(&msg,BUS_BROADCAST_ADDRESS, busProtocol_deviceManagementProtocol, 0, RESPONSE_PRIORITY);
		bus_pushByte(&msg,dmp_cmdApplicationName);
		bus_pushArray(&msg, appNamePtr, string_getLength((char*)appNamePtr));
		bus_send(&msg);
	}
}

void sendDeviceName(void)
{
	uint8_t DeviceName[60];

	uint8_t length = eeporm_readByte(&deviceNameLength);
	if(length > sizeof(DeviceName)) length = 0; // in case length is to long -> e.g. on first boot
	eeprom_readArray(&DeviceName[0], &deviceName[0], length);
	
	bus_message_t msg;
	if(bus_getMessageSlot(&msg))
	{
		bus_writeHeader(&msg,BUS_BROADCAST_ADDRESS, busProtocol_deviceManagementProtocol, 0, RESPONSE_PRIORITY);
		bus_pushByte(&msg,dmp_cmdDeviceName);
		bus_pushArray(&msg, &DeviceName[0],length);
		bus_send(&msg);
	}
}

void sendDiagnosticsReport(void)
{
	bus_message_t msg;
	if(bus_getMessageSlot(&msg))
	{
		bus_writeHeader(&msg,BUS_BROADCAST_ADDRESS, busProtocol_deviceManagementProtocol, 0, RESPONSE_PRIORITY);
		bus_pushByte(&msg,dmp_canDiagnosticsReport);
		bus_pushWord24(&msg,bus_getErrorCounter());
		bus_pushByte(&msg,bus_getLastErrorCode());
		bus_pushWord32(&msg,benchmark->us_avg);
		bus_pushWord32(&msg,benchmark->us_min);
		bus_pushWord32(&msg,benchmark->us_max);
		bus_send(&msg);
	}
	
	benchmark_reset(benchmark);
}

void writeHeartbeatSettings(uint8_t *data, uint8_t size)
{
	uint16_t u16Temp = unpack_uint16(&data[0]);
	if(u16Temp == 0) u16Temp = 1;
	eeprom_writeWord(u16Temp,&heartbeatInterval);
	heartbeatIntervalTime = eeporm_readWord(&heartbeatInterval);
	
	u16Temp = unpack_uint16(&data[2]);
	if(u16Temp == 0) u16Temp = 1;
	eeprom_writeWord(u16Temp,&extendedHeartbeatInterval);
	extendedHeartbeatIntervalTime = eeporm_readWord(&extendedHeartbeatInterval);
}

void writeControl(uint8_t *data, uint8_t size)
{
	if(size!= 4) return;
	sysControlPtr->reg = unpack_uint32(&data[0]);
}

void setControl(uint8_t *data, uint8_t size)
{
	if(size!= 4) return;
	sysControlPtr->reg |= unpack_uint32(&data[0]);
}

void clrControl(uint8_t *data, uint8_t size)
{
	if(size!= 4) return;
	sysControlPtr->reg &= ~unpack_uint32(&data[0]);
}

void enterRootMode(uint8_t *data, uint8_t size)
{
	uint16_t temp = unpack_uint16(&data[0]);
	if(temp == SETUP_MODE_KEY && size == 2) sysStatusPtr->bit.setupModeEn = true;
	else sysStatusPtr->bit.setupModeEn = false;
}

void writeDeviceName(uint8_t *data, uint8_t size)
{
	if(size-1 < DEVICE_NAME_MAX_SIZE)
	{
		eeprom_writeArray(&data[0],&deviceName[0],size);
		eeprom_writeByte(size,&deviceNameLength);
	}
	
	sendDeviceName();
}

void writeAddress(uint8_t sourceAddress, uint8_t *data, uint8_t size)
{	
	if( (size==1) && ((data[0] != 0x00)||(data[0] < 0x7F)) )
	{
		eeprom_writeByte(data[0], &deviceAddress);
		if( data[0] == eeporm_readByte(&deviceAddress)) system_reboot();
	}
}

void echo(uint8_t sourceAddress, uint8_t *data, uint8_t size)
{
	bus_message_t msg;
	if(bus_getMessageSlot(&msg))
	{
		bus_writeHeader(&msg,sourceAddress, busProtocol_deviceManagementProtocol, 0, RESPONSE_PRIORITY);
		bus_pushByte(&msg,dmp_echo);
		bus_pushArray(&msg, data,size);
		bus_send(&msg);
	}
	
}

void reboot(uint8_t sourceAddress, uint8_t *data, uint8_t size)
{
	if(size == 0) system_reboot();
}


#ifdef __cplusplus
}
#endif