//**********************************************************************************************************************
// FileName : deviceManagementProtocol.cpp
// FilePath : protocol/kernel/
// Author   : Christian Marty
// Date		: 02.08.2025
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#ifdef __cplusplus
extern "C" {
#endif
	
#include "protocol/kernel/deviceManagementProtocol.h"
#include "protocol/kernel/messageLogProtocol.h"

#include "driver/SAMx5x/kernel/tickTimer.h"
#include "driver/SAMx5x/kernel/eeprom.h"
#include "driver/SAMx5x/kernel/flash.h"
#include "driver/SAMx5x/kernel/system.h"

#include "kernel/firmwareUpdate.h"
#include "utility/string.h"
#include "utility/pack.h"

#include <main.h> 

typedef enum {
	dmp_cmd_deviceToHost = 0,
	dmp_cmd_hostToDevice = 1
}dmp_commands_t;


#define RESPONSE_PRIORITY busPriority_low

#define APP_START_ADDRESS (NVMCTRL_BLOCK_SIZE*4)
#define APP_END_ADDRESS (NVMCTRL_BLOCK_SIZE*64)

uint8_t HARDWARE_NAME[] = KERNEL_HARDWARE_NAME;
uint8_t HARDWARE_NAME_LENGTH = (sizeof(HARDWARE_NAME)-1);

extern eememData_t eememData;

uint16_t heartbeatIntervalTime;
uint16_t extendedHeartbeatIntervalTime; 

tickTimer_t heartbeatTimer = 0;
tickTimer_t extendedHeartbeatTimer = 0;

void sendSystemInfo(void);
void sendHardwareName(void);
void sendApplicationName(void);
void sendDeviceName(void);
void writeHeartbeatSettings(const uint8_t *data, uint8_t size);
void writeControl(const uint8_t *data, uint8_t size);
void setControl(const uint8_t *data, uint8_t size);
void clrControl(const uint8_t *data, uint8_t size);
void enteradministratorAccess(const uint8_t *data, uint8_t size);
void exitadministratorAccess(void);
void writeDeviceName(const uint8_t *data, uint8_t size);
void writeadministratorAccessKey(const uint8_t *data, uint8_t size);
void writeAddress(uint8_t sourceAddress, const uint8_t *data, uint8_t size);
void sendDiagnosticsReport(void);
void sendEepromReport(uint8_t sourceAddress, const uint8_t *data, uint8_t size);
void echo(uint8_t sourceAddress, const uint8_t *data, uint8_t size);
void reboot(uint8_t sourceAddress, const uint8_t *data, uint8_t size);

uint32_t appStartAddress;

systemControl_t *sysControlPtr;
uint8_t *appNamePtr;
uint8_t appNameSize;

uint8_t protocolTyp = busProtocol_deviceManagementProtocol;

void dmp_initialize(uint8_t *appName, systemControl_t *sysControl)
{
	sysControlPtr = sysControl;
	appNamePtr = appName;
	
	appStartAddress = 0x10000;
	
	heartbeatIntervalTime = eeporm_readWord(&eememData.heartbeatInterval);
	extendedHeartbeatIntervalTime = eeporm_readWord(&eememData.extendedHeartbeatInterval);
	
	if(heartbeatIntervalTime == 0) heartbeatIntervalTime = 10;  
	if(extendedHeartbeatIntervalTime == 0) extendedHeartbeatIntervalTime = 600;
}

void dmp_handler(void)
{
	if(tickTimer_delay1ms(&heartbeatTimer,(heartbeatIntervalTime*1000))){
		dmp_sendHeartbeat();
	}

	if(tickTimer_delay1ms(&extendedHeartbeatTimer,(extendedHeartbeatIntervalTime*10000))){
		dmp_sendExtendedHeartbeat();
	}

	firmwareUpdate_handler(sysControlPtr);
}

uint8_t dmp_getDeviceAddress(void)
{
	uint8_t tmp = eeporm_readByte(&eememData.deviceAddress);
	if(tmp >= 127){
		tmp = 0; // reset address to 0 if it is out of range: e.g. for first startup
	}
	return tmp;
}

bool dmp_receiveHandler(const bus_rxMessage_t *message)
{	
	const uint8_t *data = &message->data[1];
	uint8_t dataLength = message->length-1;
	
	switch(message->data[0])
	{
		// Normal commands 
		case dmp_cmdSystemInformationRequest:
			dmp_sendExtendedHeartbeat();
			return true;

		case dmp_cmdHeartbeatRequest:
			dmp_sendHeartbeat();
			return true;

		case dmp_cmdWriteControl:
			writeControl(data, dataLength);
			return true;

		case dmp_cmdSetControl:
			setControl(data, dataLength);
			return true;

		case dmp_cmdClearControl:
			clrControl(data, dataLength);
			return true;

		case dmp_cmdEnteradministratorAccess:
			enteradministratorAccess(data, dataLength);
			return true;
			
		case dmp_cmdExitadministratorAccess:
			exitadministratorAccess();
			return true;
		
		case dmp_cmdReboot:
			reboot(message->sourceAddress, data, dataLength);
			return true;
		
		case dmp_canDiagnosticsRequest:
			sendDiagnosticsReport();
			return true;
		
		case dmp_echo:
			echo(message->sourceAddress, data, dataLength);
			return true;
	}
	
	if(sysControlPtr->sysStatus.bit.administratorAccess){
		switch(message->data[0])
		{
			// Administration mode commands
			case dmp_cmdHeartbeatSettings:
				writeHeartbeatSettings(data, dataLength);
				sendSystemInfo();
				return true;
					
			case dmp_cmdSetadministratorAccessKey:
				writeadministratorAccessKey(data, dataLength);
				return true;
			
			case dmp_cmdSetDeviceName:
				writeDeviceName(data, dataLength);
				sendDeviceName();
				return true;

			case dmp_cmdSetAddress:
				writeAddress(message->sourceAddress, data, dataLength);
				return true;

			case dmp_cmdEraseApp:
				firmwareUpdate_eraseApp(message->sourceAddress, data, dataLength);
				return true;

			case dmp_cmdBtl:
				firmwareUpdate_writeAppData(message->sourceAddress, data, dataLength);
				return true;
				
			case dmp_eepromReadRequest:
				sendEepromReport(message->sourceAddress, data, dataLength);
				return true;
		}
	}else{
		switch(message->data[0])
		{
			case dmp_cmdHeartbeatSettings:
			case dmp_cmdSetadministratorAccessKey:
			case dmp_cmdSetDeviceName:
			case dmp_cmdSetAddress:
			case dmp_cmdEraseApp:
			case dmp_cmdBtl:
			case dmp_eepromReadRequest:
				mlp_sysWarning("Administrator access required");
				return true;
		}
	}
	
	return false;
}

void dmp_sendHeartbeat(void)
{
	bus_txMessage_t msg;
	if(bus_getMessageSlot(&msg))
	{
		bus_writeHeader(&msg,BUS_BROADCAST_ADDRESS, busProtocol_deviceManagementProtocol, dmp_cmd_deviceToHost, RESPONSE_PRIORITY);
		bus_pushByte(&msg,dmp_cmdHeartbeat);
		bus_pushWord32(&msg, sysControlPtr->sysStatus.reg); 
		bus_send(&msg);
		
		tickTimer_reset(&heartbeatTimer);
	}
}

void dmp_sendExtendedHeartbeat(void)
{
	sendSystemInfo();
	sendHardwareName();
	sendApplicationName();
	sendDeviceName();
}

void sendSystemInfo(void)
{
	bus_txMessage_t msg;
	if(bus_getMessageSlot(&msg))
	{
		bus_writeHeader(&msg, BUS_BROADCAST_ADDRESS, busProtocol_deviceManagementProtocol, dmp_cmd_deviceToHost, RESPONSE_PRIORITY);
		bus_pushByte(&msg, dmp_cmdSystemInfo);
		bus_pushWord32(&msg, sysControlPtr->sysStatus.reg); 
		bus_pushByte(&msg, eeporm_readByte(&eememData.hardwareRevisionMajor));
		bus_pushByte(&msg, eeporm_readByte(&eememData.hardwareRevisionMinor));
		bus_pushByte(&msg, KERNEL_VERSION_MAJOR);
		bus_pushByte(&msg, KERNEL_VERSION_MINOR);
		bus_pushWord16(&msg, heartbeatIntervalTime);
		bus_pushWord16(&msg, extendedHeartbeatIntervalTime);
		bus_pushWord32(&msg, sysControlPtr->appCrc); 
		bus_pushWord32(&msg, appStartAddress); 
		bus_pushWord32(&msg, system_deviceId()); 
		bus_pushWord32(&msg, system_serialNumberWord0());
		bus_pushWord32(&msg, system_serialNumberWord1());
		bus_pushWord32(&msg, system_serialNumberWord2());
		bus_pushWord32(&msg, system_serialNumberWord3());
		bus_send(&msg);
		
		tickTimer_reset(&extendedHeartbeatTimer);
	}
}

void sendHardwareName(void)
{
	bus_txMessage_t msg;
	if(bus_getMessageSlot(&msg))
	{
		bus_writeHeader(&msg,BUS_BROADCAST_ADDRESS, busProtocol_deviceManagementProtocol, dmp_cmd_deviceToHost, RESPONSE_PRIORITY);
		bus_pushByte(&msg,dmp_cmdHardwareName);
		bus_pushArray(&msg, &HARDWARE_NAME[0],HARDWARE_NAME_LENGTH);
		bus_send(&msg);
	}
}

void sendApplicationName(void)
{
	bus_txMessage_t msg;
	if(bus_getMessageSlot(&msg))
	{
		bus_writeHeader(&msg,BUS_BROADCAST_ADDRESS, busProtocol_deviceManagementProtocol, dmp_cmd_deviceToHost, RESPONSE_PRIORITY);
		bus_pushByte(&msg,dmp_cmdApplicationName);
		bus_pushArray(&msg, appNamePtr, string_getLength((char*)appNamePtr));
		bus_send(&msg);
	}
}

void sendDeviceName(void)
{
	uint8_t DeviceName[60];

	uint8_t length = eeporm_readByte(&eememData.deviceNameLength);
	if(length > sizeof(DeviceName)) length = 0; // in case length is to long -> e.g. on first boot
	eeprom_readArray(&DeviceName[0], &eememData.deviceName[0], length);
	
	bus_txMessage_t msg;
	if(bus_getMessageSlot(&msg))
	{
		bus_writeHeader(&msg,BUS_BROADCAST_ADDRESS, busProtocol_deviceManagementProtocol, dmp_cmd_deviceToHost, RESPONSE_PRIORITY);
		bus_pushByte(&msg,dmp_cmdDeviceName);
		bus_pushArray(&msg, &DeviceName[0],length);
		bus_send(&msg);
	}
}

void sendDiagnosticsReport(void)
{
	bus_txMessage_t msg;
	if(bus_getMessageSlot(&msg))
	{
		bus_writeHeader(&msg,BUS_BROADCAST_ADDRESS, busProtocol_deviceManagementProtocol, dmp_cmd_deviceToHost, RESPONSE_PRIORITY);
		bus_pushByte(&msg,dmp_canDiagnosticsReport);
		bus_pushWord24(&msg,bus_getErrorCounter());
		bus_pushByte(&msg,bus_getLastErrorCode());
		bus_pushWord32(&msg, sysControlPtr->appBenchmark.us_avg);
		bus_pushWord32(&msg, sysControlPtr->appBenchmark.us_min);
		bus_pushWord32(&msg, sysControlPtr->appBenchmark.us_max);
		bus_send(&msg);
	}
	
	benchmark_reset(&sysControlPtr->appBenchmark);
}

void writeHeartbeatSettings(const uint8_t *data, uint8_t size)
{
	uint16_t u16Temp = unpack_uint16(&data[0]);
	if(u16Temp == 0) u16Temp = 1;
	eeprom_writeWord(u16Temp,&eememData.heartbeatInterval);
	heartbeatIntervalTime = eeporm_readWord(&eememData.heartbeatInterval);
	
	u16Temp = unpack_uint16(&data[2]);
	if(u16Temp == 0) u16Temp = 1;
	eeprom_writeWord(u16Temp,&eememData.extendedHeartbeatInterval);
	extendedHeartbeatIntervalTime = eeporm_readWord(&eememData.extendedHeartbeatInterval);
}

void writeControl(const uint8_t *data, uint8_t size)
{
	if(size!= 4) return;
	sysControlPtr->sysControl.reg = unpack_uint32(&data[0]);
}

void setControl(const uint8_t *data, uint8_t size)
{
	if(size!= 4) return;
	sysControlPtr->sysControl.reg |= unpack_uint32(&data[0]);
}

void clrControl(const uint8_t *data, uint8_t size)
{
	if(size!= 4) return;
	sysControlPtr->sysControl.reg &= ~unpack_uint32(&data[0]);
}

void enteradministratorAccess(const uint8_t *data, uint8_t size)
{
	bool keyValid = false;
	uint8_t deviceName[60];

	uint8_t length = eeporm_readByte(&eememData.administratorAccessKeyLength);
	if(length > sizeof(deviceName)){
		length = 0; // in case length is to long -> e.g. on first boot
	}

	if(length == size){
		eeprom_readArray(&deviceName[0], &eememData.administratorAccessKey[0], length);
		keyValid = string_isEqual((char*)&data[0], (char*)&deviceName[0], size);
	}

	if(!keyValid && !sysControlPtr->sysStatus.bit.administratorAccess){
		mlp_sysWarning("Administration key invalid");
	}

	sysControlPtr->sysStatus.bit.administratorAccess = keyValid;
	sysControlPtr->sysStatusOld.bit.administratorAccess = keyValid;

	dmp_sendHeartbeat();
}

void exitadministratorAccess(void)
{
	sysControlPtr->sysStatus.bit.administratorAccess = false;
}

void writeadministratorAccessKey(const uint8_t *data, uint8_t size)
{
	if(size-1 < MAX_STRING_SIZE){
		eeprom_writeArray(&data[0], &eememData.administratorAccessKey[0], size);
		eeprom_writeByte(size, &eememData.administratorAccessKeyLength);
	}
}

void writeDeviceName(const uint8_t *data, uint8_t size)
{
	if(size-1 < MAX_STRING_SIZE){
		eeprom_writeArray(&data[0], &eememData.deviceName[0], size);
		eeprom_writeByte(size, &eememData.deviceNameLength);
	}

	sendDeviceName();
}

void writeAddress(uint8_t sourceAddress, const uint8_t *data, uint8_t size)
{	
	if( (size==1) && ((data[0] != 0x00)||(data[0] < 0x7F)) )
	{
		eeprom_writeByte(data[0], &eememData.deviceAddress);
		if( data[0] == eeporm_readByte(&eememData.deviceAddress)) system_reboot();
	}
}

void sendEepromReport(uint8_t sourceAddress, const uint8_t *data, uint8_t size)
{
	if(size != 3){
		mlp_sysWarning("EEPROM Request: parameter error");
		return;
	}
	
	uint16_t eepromOffset = unpack_uint16(&data[0]);
	uint8_t eepromSize = data[2];
		
	if(eepromOffset+eepromSize > EEPROM_SIZE){
		mlp_sysWarning("EEPROM Request: out of range");
		return;
	}
	
	if(eepromSize > 60){
		mlp_sysWarning("EEPROM Request: size > 60");
		return;
	}
	
	uint8_t eepromData[60];
	volatile uint8_t *eepormAddress = (uint8_t*)(&eememData)+eepromOffset; 
	eeprom_readArray(&eepromData[0], eepormAddress, eepromSize);
	
	bus_txMessage_t msg;
	if(bus_getMessageSlot(&msg))
	{
		bus_writeHeader(&msg,sourceAddress, busProtocol_deviceManagementProtocol, dmp_cmd_deviceToHost, RESPONSE_PRIORITY);
		bus_pushByte(&msg, dmp_eepromReadReport);
		bus_pushWord16(&msg, eepromOffset);
		bus_pushArray(&msg, &eepromData[0], eepromSize);
		bus_send(&msg);
	}
}

void echo(uint8_t sourceAddress, const uint8_t *data, uint8_t size)
{
	bus_txMessage_t msg;
	if(bus_getMessageSlot(&msg))
	{
		bus_writeHeader(&msg, sourceAddress, busProtocol_deviceManagementProtocol, dmp_cmd_deviceToHost, RESPONSE_PRIORITY);
		bus_pushByte(&msg,dmp_echo);
		bus_pushArray(&msg, data,size);
		bus_send(&msg);
	}
}

void reboot(uint8_t sourceAddress, const uint8_t *data, uint8_t size)
{
	if(size == 0){ 
		system_reboot();
	}
}


#ifdef __cplusplus
}
#endif