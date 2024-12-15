//**********************************************************************************************************************
// FileName : main.cpp
// FilePath :
// Author   : Christian Marty
// Date		: 25.11.2024
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#include "main.h"

#include "protocol/stateSystemProtocol.h"
#include "protocol/triggerSystemProtocol.h"
#include "protocol/eventSystemProtocol.h"
#include "protocol/serialBridgeProtocol.h"
#include "protocol/valueSystemProtocol.h"

#include "interface/lightBus.h"

#include "addOn/lightBusModul.h"
#include "utility/string.h"

int main(void);
bool onReceive(uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size);
kernel_t kernel __attribute__((section(".kernelCall")));

__attribute__((section(".appHeader"))) appHead_t appHead ={
/*appCRC	 */ 0xAABBCCDD, // Will be written by Bootload tool
/*appSize	 */ 0xEEFF0000, // Will be written by Bootload tool
/*appRevMaj	 */ 0x01,
/*appRevMin	 */ 0x00,
/*appName[60]*/ "Light Controller",
/*main		 */ main,
/*onRx		 */ onReceive
};


//**** Value Configuration ********************************************************************************************
void vrp_brightnessChange(uint16_t valueChannelNumber, vsp_valueData_t value)
{
	uint16_t v = (uint16_t)value.Long;
	uint8_t address = 15;
	
	uint8_t txData[] = {(address<<4)|0x00, (uint8_t)((v>>8)&0xFF), v&0xFF};
	lightBusModul_send(lightBusModul_channel_1,&txData[0], sizeof(txData));	
}

void vrp_temperatureChange(uint16_t valueChannelNumber, vsp_valueData_t value)
{
	uint8_t v = (uint8_t)value.Long;
	uint8_t address = 15;
	
	uint8_t txData[] = {(address<<4)|0x01, v};
	lightBusModul_send(lightBusModul_channel_1,&txData[0], sizeof(txData));
	
	char a[20];
	kernel.log.message(string_uInt32ToHex(value.Long, a));
}

const vsp_valueSignal_t valueSignals[] = {
	{0x10, "Brightness", 60, false, {.Long = 0}, {.Long = 0xFFFF}, vsp_uom_long, vrp_brightnessChange},
	{0x11, "Temperature", 60, false, {.Long = 0}, {.Long = 0xFF}, vsp_uom_long, vrp_temperatureChange}
};
#define valueSignalListSize ARRAY_LENGTH(valueSignals)

vsp_itemState_t valueSignalStateList[valueSignalListSize];

const valueSystemProtocol_t valueSystem = {
	.signals = valueSignals,
	.signalSize = valueSignalListSize,
	.slots = nullptr,
	.slotSize = 0,
	._signalState = valueSignalStateList,
	._slotState = nullptr
};


//**** Value Configuration ********************************************************************************************

void sbp_transmit_port0(const uint8_t *data, uint8_t size);

const sbp_port_t serialPortList[] = {
	{sbp_type_uart, sbp_transmit_port0}
};
#define serialPortListSize (sizeof(serialPortList)/sizeof(sbp_port_t))

const serialBridgeProtocol_t serialBridge = {
	.ports = serialPortList,
	.portSize = serialPortListSize
};

void sbp_transmit_port0(const uint8_t *data, uint8_t size)
{
	lightBusModul_send(lightBusModul_channel_t::lightBusModul_channel_1, data, size);
}

void lightBusModul_onReceive(lightBusModul_channel_t channel, const uint8_t *data, uint8_t size, bool error)
{
	sbp_status_t status = sbp_status_t::sbp_state_ok;
	if(error) status = sbp_status_t::sbp_state_crcError;
	
	if(channel == lightBusModul_channel_t::lightBusModul_channel_1) sbp_sendData(&serialBridge, 0x7E,0,status,data,size);
	else if(channel == lightBusModul_channel_t::lightBusModul_channel_2) sbp_sendData(&serialBridge, 0x7E,1,status,data,size);
}



bool onReceive(uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size)
{	
	switch(protocol){
		case busProtocol_valueSystemProtocol:	return vsp_receiveHandler(&valueSystem, sourceAddress, command, data, size);
		case busProtocol_serialBridgeProtocol:	return sbp_receiveHandler(&serialBridge, sourceAddress, command, data, size);
		default: return false;
	}
}


uint8_t dmx_txBuffer[514];
lightBusModul_t lightBusModul = {
	.dmx_enable = false,
	.dmx_data = dmx_txBuffer,
	.dmx_data_size = sizeof(dmx_txBuffer),
	.lightBusModul_onReceive = lightBusModul_onReceive
};


int main(void)
{
	// App init Code
	if(kernel.kernelSignals->initApp)
	{
		Reset_Handler();
		lightBusModul_init(&lightBusModul);

		kernel.appSignals->appReady = true;
	}

	// Main code here
	if(kernel.appSignals->appReady == true)
	{
		vsp_mainHandler(&valueSystem);
		lightBusModul_handler(&lightBusModul);
	}
	
	// App deinit code
    if(kernel.kernelSignals->shutdownApp)
    {
		//kernel->eemem_write(&ememData[0]);
		kernel.appSignals->shutdownReady = true;
    }
}