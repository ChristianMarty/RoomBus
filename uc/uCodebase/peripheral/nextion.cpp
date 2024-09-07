//**********************************************************************************************************************
// FileName : nextion.h
// FilePath : peripheral/
// Author   : Christian Marty
// Date		: 05.09.2024
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#include "nextion.h"

void nextion_resetComRx(nextion_t *nextion);
bool nextion_sendCommand(nextion_t *nextion, uint8_t *command);
void nextion_onTouchEventHandler(nextion_t *nextion, uint8_t page, uint8_t component);

void nextion_updatetPage(nextion_t *nextion);
void nextion_updateButtonState(nextion_t *nextion, uint8_t index);

void nextion_init(nextion_t *nextion)
{
	nextion->displayOff();
	nextion->_state = nextion_state_off;
	kernel.tickTimer.reset(&nextion->_displayOnTimer);
	kernel.tickTimer.reset(&nextion->_displayComTimer);
	
	for(uint8_t i = 0; i < nextion->touchEventActionSize; i++){
		nextion->buttonState[i].state = 0;
		nextion->buttonState[i].oldState = nextion->buttonState[i].state;
	}
}

void nextion_handler(nextion_t *nextion)
{
	if(nextion->_ffCounter == 3){
		if(nextion->_rxBuffer[0] == 0x65 && nextion->_rxBuffer[3] == 0x01){
			kernel.tickTimer.reset(&nextion->_displayOnTimer);
			nextion_onTouchEventHandler(nextion, nextion->_rxBuffer[1], nextion->_rxBuffer[2]);
		}
		nextion_resetComRx(nextion);
	}
	
	if(nextion->_txCounter && !nextion->uart->txBusy()){
		nextion->uart->sendData(&nextion->_txBuffer[0], nextion->_txCounter);
		nextion->_txCounter = 0;
	}
	
	if(nextion->_state == nextion_state_on && kernel.tickTimer.delay1ms(&nextion->_displayOnTimer, nextion_displayTimeout)){
		nextion->displayOff();
		nextion->_currentPage = 0;
		nextion->_lastPage = nextion->_currentPage;
	}
	
	if(nextion->_currentPage != 0 && nextion->_state == nextion_state_off){
		nextion->displayOn();
		nextion->_state = nextion_state_waitForOn;
		kernel.tickTimer.reset(&nextion->_displayOnTimer);
	}
	
	if(nextion->_state == nextion_state_waitForOn && kernel.tickTimer.delay1ms(&nextion->_displayOnTimer, 150)){
		nextion->_state = nextion_state_on;
		nextion_resetComRx(nextion);
		nextion_updatetPage(nextion);
		nextion->_lastPage = nextion->_currentPage;
		kernel.tickTimer.reset(&nextion->_displayOnTimer);
	}
	
	if(nextion->_state == nextion_state_on){
		nextion_updatetPage(nextion);
		for(uint8_t i = 0; i < nextion->touchEventActionSize; i++){
			if(nextion->_currentPage == nextion->touchEventAction[i].page){
				nextion_updateButtonState(nextion, i);
			}
		}
	}
}

void nextion_rxHandler(nextion_t *nextion, uint8_t byte)
{
	nextion->_rxBuffer[nextion->_rxCounter] = byte;
	nextion->_rxCounter++;
	
	if(byte != 0xFF) nextion->_ffCounter = 0;
	else nextion->_ffCounter  ++;
	
	// If buffer overflow -> reset rx buffer
	if(nextion->_rxCounter > sizeof(nextion->_rxBuffer)) {
		nextion_resetComRx(nextion);
	}
}

void nextion_resetComRx(nextion_t *nextion)
{
	nextion->_rxCounter = 0;
	nextion->_ffCounter = 0;
}

bool nextion_sendCommand(nextion_t *nextion, uint8_t *command)
{
	while(command[nextion->_txCounter] != 0){
		nextion->_txBuffer[nextion->_txCounter] = command[nextion->_txCounter];
		nextion->_txCounter++;
		if(nextion->_txCounter+3 >= sizeof(nextion->_txBuffer)){
			return false;
		}
	}
	nextion->_txBuffer[nextion->_txCounter] = 0xFF;
	nextion->_txCounter++;
	nextion->_txBuffer[nextion->_txCounter] = 0xFF;
	nextion->_txCounter++;
	nextion->_txBuffer[nextion->_txCounter] = 0xFF;
	nextion->_txCounter++;
	
	return true;
}	

void nextion_onTouchEventHandler(nextion_t *nextion, uint8_t page, uint8_t component)
{
	for(uint8_t i = 0; i< nextion->touchEventActionSize; i++){
		if(nextion->touchEventAction[i].page == page && nextion->touchEventAction[i].component == component){
			nextion->touchEventAction[i].eventHandler(i, page, component);
		}
	}
}

void nextion_setPage(nextion_t *nextion, uint8_t page)
{
	nextion->_lastPage = nextion->_currentPage;
	nextion->_currentPage = page;
}

void nextion_updatetPage(nextion_t *nextion)
{
	if(nextion->_lastPage == nextion->_currentPage) return;
	
	uint8_t temp[] = "page _";
	temp[5] = nextion->_currentPage+0x30;
	nextion_sendCommand(nextion, temp);
	
	for(uint8_t i = 0; i < nextion->touchEventActionSize; i++){
		if(nextion->_currentPage == nextion->touchEventAction[i].page){
			nextion->buttonState[i].oldState = !nextion->buttonState[i].state;
		}
	}
}

void nextion_updateButtonState(nextion_t *nextion, uint8_t index)
{
	if(nextion->buttonState[index].state == nextion->buttonState[index].oldState) return;
	
	uint8_t temp[] = "bt_.val=_";
	temp[2] = nextion->touchEventAction[index].objName[2];
	
	if(nextion->buttonState[index].state){
		temp[8] = '1';
	}else{
		temp[8] = '0';
	}
	if(nextion_sendCommand(nextion, temp)){
		nextion->buttonState[index].oldState = 	nextion->buttonState[index].state;
	}
}