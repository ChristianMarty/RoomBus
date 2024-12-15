//**********************************************************************************************************************
// FileName : main.cpp
// FilePath :
// Author   : Christian Marty
// Date		: 20.04.2019
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#include "main.h"

#include "common/kernel.h"

#include "protocol/triggerSystemProtocol.h"
#include "protocol/stateSystemProtocol.h"

#include "driver/SAMx5x/pin.h"

#include "utility/edgeDetect.h"

#include "peripheral/nextion.h"

int main(void);
bool onReceive(uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size);
kernel_t kernel __attribute__((section(".kernelCall")));

__attribute__((section(".appHeader"))) appHead_t appHead ={
/*appCRC	 */ 0xAABBCCDD, // Will be written by Bootload tool
/*appSize	 */ 0xEEFF0000, // Will be written by Bootload tool
/*appRevMaj	 */ 0x01,
/*appRevMin	 */ 0x00,
/*appName[60]*/ "Touch display",
/*main		 */ main,
/*onRx		 */ onReceive
};

void stdEventHandler(uint8_t index, uint8_t page, uint8_t component);
void hdmiMatrixHandler(uint8_t index, uint8_t page, uint8_t component);

const nextion_touchEventAction_t touchEventAction[] = {
	{ 1, 1, "bt0", stdEventHandler},
	{ 1, 2, "bt1", stdEventHandler},
	{ 1, 3, "bt2", stdEventHandler},
		
	{ 2, 1, "bt0", stdEventHandler},
	{ 2, 2, "bt1", stdEventHandler},
	{ 2, 3, "bt2", hdmiMatrixHandler},
	{ 2, 4, "bt3", hdmiMatrixHandler},
	{ 2, 5, "bt4", hdmiMatrixHandler},
	{ 2, 6, "bt5", hdmiMatrixHandler},
		
	{ 3, 1, "bt0", stdEventHandler},
	{ 3, 2, "bt1", stdEventHandler},
	{ 3, 3, "bt2", stdEventHandler},
	{ 3, 4, "bt3", stdEventHandler},
	{ 3, 5, "bt4", stdEventHandler},
	
	{ 4, 1, "bt1", stdEventHandler}
};
#define touchEventListSize ARRAY_LENGTH(touchEventAction)
nextion_buttonState_t stateList[touchEventListSize];

void displayOn(void)
{
	pin_setOutput(IO_D17, true);
}

void displayOff(void)
{
	pin_setOutput(IO_D17, false);
}

uart_c uart_0;
nextion_t nextion_0 = {
	.touchEventAction = touchEventAction,
	.buttonState = stateList,
	.touchEventActionSize = touchEventListSize,
	.uart = &uart_0,
	.displayOn = displayOn,
	.displayOff = displayOff
};

void uart_onTx(void){
	uart_0.TxInterrupt();
}

void uart_onRx(void){
	nextion_rxHandler(&nextion_0, uart_0.RxInterrupt());
}

//**** State Configuration ********************************************************************************************

#define SLOT_TIMEOUT 10
void stateChangeAction(uint16_t stateChannelNumber, ssp_state_t state)
{
	
	
}

const ssp_stateSlot_t stateSlots[] = {
	{ 0x02, "Light 1", SLOT_TIMEOUT, stateChangeAction },
	{ 0x03, "Light 2", SLOT_TIMEOUT, stateChangeAction },
	{ 0x04, "Light 3", SLOT_TIMEOUT, stateChangeAction },
		
	{ 0x29, "Amp Power", SLOT_TIMEOUT, stateChangeAction },	
	
	{ 0x80, "Projector Power", SLOT_TIMEOUT, stateChangeAction },
	{ 0x81, "Projector Blackout", SLOT_TIMEOUT, stateChangeAction },
	{ 0x82, "HDMI 1 (PC)", SLOT_TIMEOUT, stateChangeAction },
	{ 0x83, "HDMI 2 (MP)", SLOT_TIMEOUT, stateChangeAction },
	{ 0x84, "HDMI 3", SLOT_TIMEOUT, stateChangeAction },
	{ 0x85, "HDMI 4", SLOT_TIMEOUT, stateChangeAction }
};
#define stateSystemSlotListSize ARRAY_LENGTH(stateSlots)

ssp_itemState_t stateSystemSlotStatusList[stateSystemSlotListSize];
const stateSystemProtocol_t stateSystem = {
	.signals = nullptr,
	.signalSize = 0,
	.slots = stateSlots,
	.slotSize = stateSystemSlotListSize,
	._signalState = nullptr,
	._slotState = stateSystemSlotStatusList
};

//**** Trigger Configuration ******************************************************************************************

const tsp_triggerSignal_t triggerSignals[] = {
	{ 0x07, "Button 1"},
	{ 0x0A, "Button 2"},
	{ 0x0D, "Button 3"},
		
	{ 0x5E, "Amp"},		
			
	{ 0x82, "Projector On/Off"},
	{ 0x85, "Projector Blackout"},
		
	{ 0x86, "HDMI Out A - In 1"},
	{ 0x87, "HDMI Out A - In 2"},
	{ 0x88, "HDMI Out A - In 3"},
	{ 0x89, "HDMI Out A - In 4"},
	{ 0x8E, "HDMI Off"}
};
#define triggerSignalListSize ARRAY_LENGTH(triggerSignals)

tsp_itemState_t triggerSignalStateList[triggerSignalListSize];

const triggerSystemProtocol_t triggerSystem = {	
	.signals = triggerSignals,
	.signalSize = triggerSignalListSize,
	.slots = nullptr,
	.slotSize = 0,
	._signalState = triggerSignalStateList,
	._slotState = nullptr
};


//*********************************************************************************************************************
bool onReceive(uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size){
	switch(protocol){
		case busProtocol_triggerSystemProtocol:		return tsp_receiveHandler(&triggerSystem, sourceAddress, command, data, size);
		case busProtocol_stateSystemProtocol:	return ssp_receiveHandler(&stateSystem, sourceAddress, command, data, size);
		default: return false;
	}
}

void stdEventHandler(uint8_t index, uint8_t page, uint8_t component)
{
	uint8_t triggerIndex = 0xFF;
	
	if(page == 1){
		if(component == 1) triggerIndex = 0;
		else if(component == 2) triggerIndex = 1;
		else if(component == 3) triggerIndex = 2;
	}else if(page == 2){
		if(component == 1) triggerIndex = 4;
		else if(component == 2) triggerIndex = 5;
	}else if(page == 4){
		if(component == 1) triggerIndex = 3;
	}	
	
	tsp_sendTriggerByIndex(&triggerSystem, triggerIndex);
}

void hdmiMatrixHandler(uint8_t index, uint8_t page, uint8_t component)
{
	if(page != 2) return;
	
	uint8_t triggerIndex = 0xFF;
	
	bool state = nextion_0.buttonState[index].state;
	
	if(state) triggerIndex = 10;
	else if(component == 3) triggerIndex = 6;
	else if(component == 4) triggerIndex = 7;
	else if(component == 5) triggerIndex = 8;
	else if(component == 6) triggerIndex = 9;
	
	tsp_sendTriggerByIndex(&triggerSystem, triggerIndex);
}

int main()
{
	// App initialize Code
	if(kernel.kernelSignals->initApp)
	{
		Reset_Handler();

		pin_enableInput(IO_D04);
		pin_enableInput(IO_D05);
		pin_enableInput(IO_D06);
		pin_enableInput(IO_D07);	
		
		pin_enableOutput(IO_D17);
		pin_setOutput(IO_D17,true);	
		
		// Uart
		uart_0.initUart(kernel.clk_16MHz, SERCOM5, 115200, uart_c::none);
		pin_enablePeripheralMux(PIN_PORT_B, 16, PIN_FUNCTION_C);
		pin_enablePeripheralMux(PIN_PORT_B, 17, PIN_FUNCTION_C);
		kernel.nvic.assignInterruptHandler(SERCOM5_2_IRQn, uart_onRx);
		kernel.nvic.assignInterruptHandler(SERCOM5_1_IRQn, uart_onTx);
		
		nextion_init(&nextion_0);
		tsp_initialize(&triggerSystem);
		ssp_initialize(&stateSystem);
		
		kernel.appSignals->appReady = true;		
	}

	// Main app
	if(kernel.appSignals->appReady == true)
	{	
		if(!pin_getInput(IO_D06)) nextion_setPage(&nextion_0, 1);
		else if(!pin_getInput(IO_D07)) nextion_setPage(&nextion_0, 2);
		else if(!pin_getInput(IO_D04)) nextion_setPage(&nextion_0, 3);
		else if(!pin_getInput(IO_D05)) nextion_setPage(&nextion_0, 4);
		
		nextion_handler(&nextion_0);
		
		tsp_mainHandler(&triggerSystem);
		ssp_mainHandler(&stateSystem);
		
		// Update button state
		nextion_0.buttonState[0].state = stateSystemSlotStatusList[0].state == srp_state_on ? true : false; 
		nextion_0.buttonState[1].state = stateSystemSlotStatusList[1].state == srp_state_on ? true : false; 
		nextion_0.buttonState[2].state = stateSystemSlotStatusList[2].state == srp_state_on ? true : false; 
		
		nextion_0.buttonState[14].state = stateSystemSlotStatusList[3].state == srp_state_on ? true : false;
		
		nextion_0.buttonState[3].state = stateSystemSlotStatusList[4].state == srp_state_on ? true : false;
		nextion_0.buttonState[4].state = stateSystemSlotStatusList[5].state == srp_state_on ? true : false;
		nextion_0.buttonState[5].state = stateSystemSlotStatusList[6].state == srp_state_on ? true : false;
		nextion_0.buttonState[6].state = stateSystemSlotStatusList[7].state == srp_state_on ? true : false;
		nextion_0.buttonState[7].state = stateSystemSlotStatusList[8].state == srp_state_on ? true : false;
		nextion_0.buttonState[8].state = stateSystemSlotStatusList[9].state == srp_state_on ? true : false;
	}
	
	// App de-initialize code
    if(kernel.kernelSignals->shutdownApp)
    {
		kernel.appSignals->shutdownReady = true;
    }
	
}