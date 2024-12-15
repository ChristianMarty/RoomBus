//**********************************************************************************************************************
// FileName : main.cpp
// FilePath :
// Author   : Christian Marty
// Date		: 15.06.2024
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#include "main.h"

#include "protocol/stateSystemProtocol.h"
#include "protocol/triggerSystemProtocol.h"
#include "protocol/eventSystemProtocol.h"
#include "protocol/serialBridgeProtocol.h"

#include "interface/lightBus.h"
#include "interface/modbusMaster.h"

#include "powerMeterRead.h"
#include "pcController.h"

int main(void);
bool onReceive(uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size);
kernel_t kernel __attribute__((section(".kernelCall")));

__attribute__((section(".appHeader"))) appHead_t appHead ={
/*appCRC	 */ 0xAABBCCDD, // Will be written by Bootload tool
/*appSize	 */ 0xEEFF0000, // Will be written by Bootload tool
/*appRevMaj	 */ 0x01,
/*appRevMin	 */ 0x00,
/*appName[60]*/ "230V Energiemeter",
/*main		 */ main,
/*onRx		 */ onReceive
};

lightBus_t lightBus_1;

struct{
	bsRelay_t relay;
	ssp_state_t outputState;
	ssp_state_t outputStateOld;
} auxiliaryOutput;

//**** State Configuration ********************************************************************************************
#define STATE_SIGNAL_INTERVAL 10
const ssp_stateSignal_t stateSignals[] = {
	{0x01, "PC Status", STATE_SIGNAL_INTERVAL},
	{0x00, "K3 - Auxiliary Status", STATE_SIGNAL_INTERVAL},
	{0x02, "K2 - Peripheral Relay Status", STATE_SIGNAL_INTERVAL},
	{0x03, "K1 - Computer Relay Status", STATE_SIGNAL_INTERVAL}
};
#define stateSystemSignaListSize ARRAY_LENGTH(stateSignals)

ssp_itemState_t stateSystemSignaStatusList[stateSystemSignaListSize];

const stateSystemProtocol_t stateSystem = {
	.signals = stateSignals,
	.signalSize = stateSystemSignaListSize,
	.slots = nullptr,
	.slotSize = 0,
	._signalState = stateSystemSignaStatusList,
	._slotState = nullptr
};

//**** Trigger Configuration ******************************************************************************************
void triggerHandler(uint16_t triggerChannelNumber)
{
	switch(triggerChannelNumber)
	{		
		case 0x00: bsRelay_set(&auxiliaryOutput.relay, true); break;
		case 0x01: bsRelay_set(&auxiliaryOutput.relay, false); break;
		case 0x02: bsRelay_toggle(&auxiliaryOutput.relay); break;
		
		case 0x03: pcController.initTurnOn = true; break;
		case 0x04: pcController.initTurnOff = true; break;
	}
}

const tsp_triggerSlot_t triggerSlotList[] = {
	{0x00, "Aux On",      triggerHandler},
	{0x01, "Aux Off",     triggerHandler},
	{0x02, "Aux Toggle",  triggerHandler},
				
	{0x03, "PC Turn On",  triggerHandler},
	{0x04, "PC Turn Off", triggerHandler}
	
};
#define triggerSlotListSize ARRAY_LENGTH(triggerSlotList)

tsp_itemState_t triggerSlotStateList[triggerSlotListSize];
const triggerSystemProtocol_t triggerSystem = {
	.signals = nullptr,
	.signalSize = 0,
	.slots = triggerSlotList,
	.slotSize = triggerSlotListSize,
	._signalState = nullptr,
	._slotState = triggerSlotStateList
};



//**** Serial Bridge Configuration ************************************************************************************

void sbp_transmit_port0(const uint8_t *data, uint8_t size);
const sbp_port_t serialPortList[] = {
	{sbp_type_uart, sbp_transmit_port0}
};
#define serialPortListSize (sizeof(serialPortList)/sizeof(sbp_port_t))

void sbp_transmit_port0(const uint8_t *data, uint8_t size)
{
	lightBus_send(kernel, &lightBus_1, data, size); 
}

void lightBus_onReceive(const uint8_t *data, uint8_t size, bool error)
{
	sbp_status_t status = sbp_status_t::sbp_state_ok;
	if(error) status = sbp_status_t::sbp_state_crcError;
	sbp_sendData(kernel,0x7E,0,status,data,size);
}

const serialBridgeProtocol_t serialBridge = {
	.ports = serialPortList,
	.portSize = serialPortListSize
};

// ------------------------------------------------------------------------------------------------

void onReceive(const kernel_t *kernel, uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size)
{	
	switch(protocol)
	{
		case busProtocol_triggerSystemProtocol:	tsp_receiveHandler(&triggerSystem, sourceAddress, command, data, size); break;
		case busProtocol_stateSystemProtocol:	ssp_receiveHandler(&stateSystem, sourceAddress, command, data, size); break;
		case busProtocol_serialBridgeProtocol:	sbp_receiveHandler(&serialBridge, sourceAddress, command, data, size); break;
		case busProtocol_valueSystemProtocol:	vsp_receiveHandler(&valueSystem, sourceAddress, command, data, size); break;
	}
}

void tsp_onTrigger_callback(const kernel_t *kernel, uint8_t sourceAddress, uint8_t triggerChannelNumber)
{
	tsp_triggerAction(kernel, triggerChannelNumber, triggerSlotList, triggerSlotListSize);
}

void tsp_onStateChange_callback(const kernel_t *kernel)
{
	
}

// ------------------------------------------------------------------------------------------------
void uart_5_onRx(void)
{
	lightBus_onRx(&lightBus_1);
}

void uart_5_onTx(void)
{
	lightBus_onTx(&lightBus_1);
}

// ------------------------------------------------------------------------------------------------
void uart_2_onRx(void)
{
	modbusMaster_rxInterrupt(&modbusMaster);
}

void uart_2_onTx(void)
{
	modbusMaster_txInterrupt(&modbusMaster);
}

// ------------------------------------------------------------------------------------------------
void modbusMaster_onReadingCompleted(const modbusMaster_reading_t *reading)
{
	vrp_sendValues(_kernel, &valueReportProtocol);
}

// ------------------------------------------------------------------------------------------------
int main(void)
{
	// App init Code
	if(kernel->kernelSignals->initApp)
	{
		//app_init();
		Reset_Handler();
		
		// Init Light 1 BUS
		lightBus_init(kernel, &lightBus_1, SERCOM5);
		pin_enablePeripheralMux(IO_D12, PIN_FUNCTION_C);
		pin_enablePeripheralMux(IO_D13, PIN_FUNCTION_C);
		kernel->nvic.assignInterruptHandler(SERCOM5_2_IRQn,uart_5_onRx);
		kernel->nvic.assignInterruptHandler(SERCOM5_1_IRQn,uart_5_onTx);
		lightBus_1.onReceive = lightBus_onReceive;
		
		// Init Modbus
		modbusMaster_init(kernel, &modbusMaster, SERCOM2);
		pin_enablePeripheralMux(IO_D04, PIN_FUNCTION_C);
		pin_enablePeripheralMux(IO_D05, PIN_FUNCTION_C);
		pin_enablePeripheralMux(IO_D06, PIN_FUNCTION_C);
		kernel->nvic.assignInterruptHandler(SERCOM2_2_IRQn,uart_2_onRx);
		kernel->nvic.assignInterruptHandler(SERCOM2_1_IRQn,uart_2_onTx);
		
		modbusMaster_attachReadings(kernel, &modbusMaster, &modbusMaster_reading[0], modbusMaster_reading_size);
		
		// Enable Modbus Transceiver Power
		pin_enableOutput(IO_D07);
		pin_setOutput(IO_D07, true);
		
		// Configure relay outputs
		bsRelay_init(&pcController.peripheralRelay,IO_A13, IO_A12);
		bsRelay_init(&pcController.computerRelay,  IO_A11, IO_A10);
		bsRelay_init(&auxiliaryOutput.relay,       IO_A09, IO_A08);
		
		auxiliaryOutput.outputState = srp_state_undefined;
		auxiliaryOutput.outputStateOld = srp_state_transitioning;
		
		// PC interface
		pcController_init(kernel);
		
		pin_enableInput(IO_A07);  // PC Power LED 
		pin_enableInput(IO_A04);  // Power Button
		
		pin_enableOutput(IO_A05); // Switch light
		pin_enableOutput(IO_A06); // PC Power button

		kernel->appSignals->appReady = true;
	}

	// Main code here
	if(kernel->appSignals->appReady == true)
	{
		lightBus_handler(kernel, &lightBus_1);
		modbusMaster_handler(kernel, &modbusMaster);
		
		esp_mainHandler(kernel, &eventSystem);
		tsp_mainHandler(kernel, &triggerSystem);
		
		// Auxiliary Output Handler
		bsRelay_handler(kernel, &auxiliaryOutput.relay);
		
		switch( bsRelay_state(&auxiliaryOutput.relay)){
			case bsRelay_unknown: auxiliaryOutput.outputState = srp_state_undefined; break;
			case bsRelay_on:	  auxiliaryOutput.outputState = srp_state_on;        break;
			case bsRelay_off:	  auxiliaryOutput.outputState = srp_state_off;       break;
			case bsRelay_transitioning:  break;
		}
		
		// PC Control Handler
		if( pcController_handler(kernel) || 
			auxiliaryOutput.outputState != auxiliaryOutput.outputStateOld)
		{
			auxiliaryOutput.outputStateOld = auxiliaryOutput.outputState;
			srp_sendGroupReport(kernel, &stateReport);
			
		}
		
	}
	
	// App deinit code
    if(kernel->kernelSignals->shutdownApp)
    {
		kernel->appSignals->shutdownReady = true;
    }
}