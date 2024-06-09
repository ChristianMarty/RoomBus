/*
 * app.cpp
 *
 * Created: 20.04.2019 20:05:30
 * Author : Christian
 */ 

#include "sam.h"
#include "kernel/kernel.h"
#include "kernel/busController_IO.h"

#include "Raumsteuerung/stateReportProtocol.h"
#include "Raumsteuerung/triggerProtocol.h"
#include "Raumsteuerung/eventProtocol.h"
#include "Raumsteuerung/serialBridgProtocol.h"

#include "Raumsteuerung/lightBus.h"
#include "Raumsteuerung/bistableRelay.h"

#include "utility/edgeDetect.h"

#include "powerMeterRead.h"
#include "pcController.h"

int main(const kernel_t *kernel);
void onReceive(const kernel_t *kernel, uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size);
kernel_t const *_kernel;

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
	srp_state_t outputState;
	srp_state_t outputStateOld;
} auxiliaryOutput;


void triggerHandler(uint8_t triggerChannelNumber)
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
	{UNFILTERED, 0x00, "Aux On",      triggerHandler},
	{UNFILTERED, 0x01, "Aux Off",     triggerHandler},
	{UNFILTERED, 0x02, "Aux Toggle",  triggerHandler},
				
	{UNFILTERED, 0x03, "PC Turn On",  triggerHandler},
	{UNFILTERED, 0x04, "PC Turn Off", triggerHandler}
	
};
#define triggerSlotListSize (sizeof(triggerSlotList)/sizeof(tsp_triggerSlot_t))

esp_eventState_t eventState[1];
const esp_eventSignal_t eventSignals[] = {
};

const eventSystemProtocol_t eventSystem = {
	.signals = eventSignals,
	.signalSize = (sizeof(eventSignals)/sizeof(esp_eventSignal_t)),
	.slots = nullptr,
	.slotSize = 0
};


const srp_stateReportChannel_t stateReportList[] = {
	{0x01, "PC Status", &pcController.reportedState},
	{0x00, "K3 - Auxiliary Status", &auxiliaryOutput.outputState},
	{0x02, "K2 - Peripheral Relay Status", &pcController.peripheralState},
	{0x03, "K1 - Computer Relay Status", &pcController.computerState}
};
#define stateReportListSize (sizeof(stateReportList)/sizeof(srp_stateReportChannel_t))


const stateReportProtocol_t stateReport = {
	.states = nullptr,
	.stateSize = 0,
	.channels = stateReportList,
	.channelSize = (sizeof(stateReportList)/sizeof(srp_stateReportChannel_t))
};

tsp_itemStatus_t _slotsStatus[triggerSlotListSize];
const triggerSlotProtocol_t triggerSystem = {
	.signals = nullptr,
	.signalSize = 0,
	.slots = triggerSlotList,
	.slotSize = triggerSlotListSize,
	._slotsStatus = &_slotsStatus[0]
};


// --- serialBridgeProtocol -----------------------------------------------------------------------

void sbp_transmit_port0(const uint8_t *data, uint8_t size);
const sbp_port_t serialPortList[] = {
	{sbp_type_uart, sbp_transmit_port0}
};
#define serialPortListSize (sizeof(serialPortList)/sizeof(sbp_port_t))

void sbp_transmit_port0(const uint8_t *data, uint8_t size)
{
	lightBus_send(_kernel, &lightBus_1, data, size); 
}

void lightBus_onReceive(const uint8_t *data, uint8_t size, bool error)
{
	sbp_status_t status = sbp_status_t::sbp_state_ok;
	if(error) status = sbp_status_t::sbp_state_crcError;
	sbp_sendData(_kernel,0x7E,0,status,data,size);
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
		case busProtocol_triggerProtocol:		tsp_receiveHandler(kernel, sourceAddress, command, data, size, &triggerSystem); break;
		case busProtocol_stateReportProtocol:	srp_receiveHandler(kernel, sourceAddress, command, data, size, &stateReport); break;
		case busProtocol_eventProtocol:			esp_receiveHandler(kernel, sourceAddress, command, data, size, &eventSystem); break;
		case busProtocol_serialBridgeProtocol:	sbp_receiveHandler(kernel, sourceAddress, command, data, size, &serialBridge); break;
		case busProtocol_valueReportProtocol:	vrp_receiveHandler(kernel, sourceAddress, command, data, size, &valueReportProtocol); break;
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
int main(const kernel_t *kernel)
{
	_kernel = kernel;
	
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