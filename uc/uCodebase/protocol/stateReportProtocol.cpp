//**********************************************************************************************************************
// FileName : stateReportProtocol.cpp
// FilePath : protocol/
// Author   : Christian Marty
// Date		: 21.07.2024
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#include "stateReportProtocol.h"

#ifdef __cplusplus
extern "C" {
#endif

bool parseStateReport(const stateReportProtocol_t* srp, uint8_t sourceAddress, const uint8_t *data, uint8_t size);
bool parseStateRequest(const stateReportProtocol_t* srp, uint8_t sourceAddress, const uint8_t *data, uint8_t size);
bool parseStateReportSignalInformationRequest(const stateReportProtocol_t* srp, uint8_t sourceAddress, const uint8_t *data, uint8_t size);
bool parseStateReportSlotInformationRequest(const stateReportProtocol_t* srp, uint8_t sourceAddress, const uint8_t *data, uint8_t size);

bool sendStateReportSignalInformation(const srp_stateSignal_t *stateSignal);
bool sendStateReportSlotInformation(const srp_stateSlot_t *stateSlot);
void sendStateReports(const stateReportProtocol_t* srp);

void srp_initialize(const stateReportProtocol_t *srp)
{
	for(int i = 0; i < srp->slotSize; i++){
		kernel.tickTimer.reset(&(srp->_slotState[i].timer));
		
		srp->_slotState[i].sendInformationPending = false;
		srp->_slotState[i].state = srp_state_undefined;
	}
	
	for(int i = 0; i < srp->signalSize; i++){
		kernel.tickTimer.reset(&(srp->_signalState[i].timer));
		
		srp->_signalState[i].sendInformationPending = false;
		srp->_signalState[i].state = srp_state_undefined;
	}
	
}

void srp_mainHandler(const stateReportProtocol_t *srp)
{
	for(int i = 0; i < srp->signalSize; i++){
		if(kernel.tickTimer.delay1ms(&(srp->_signalState[i].timer), srp->signals[i].interval*1000)){
			srp->_signalState[i].sendSignalPending = true;
		}
	}
	
	// send signal information
	for(uint8_t i = 0;  i < srp->signalSize; i++) {
		if(srp->_signalState[i].sendInformationPending == true) {
			if(sendStateReportSignalInformation(&srp->signals[i])) {
				srp->_signalState[i].sendInformationPending = false;
			}
			break;
		}
	}
	
	// send slot information
	for(uint8_t i = 0;  i < srp->slotSize; i++) {
		if(srp->_slotState[i].sendInformationPending == true){
			if(sendStateReportSlotInformation(&srp->slots[i])) {
				srp->_slotState[i].sendInformationPending = false;
			}
			break;
		}
	}
	
	sendStateReports(srp);
}

bool srp_receiveHandler(const stateReportProtocol_t *srp, uint8_t sourceAddress, uint8_t command, const uint8_t *data, uint8_t size)
{
	switch(command){
		case srp_cmd_state: return parseStateReport(srp, sourceAddress, data, size);
		case srp_cmd_stateRequest: return parseStateRequest(srp, sourceAddress, data, size);
		case srp_cmd_signalInformationRequest: return parseStateReportSignalInformationRequest(srp, sourceAddress, data, size);
		case srp_cmd_slotInformationRequest: return parseStateReportSlotInformationRequest(srp, sourceAddress, data, size);
	}
	return false;
}

void srp_setStateByIndex(const stateReportProtocol_t *srp, uint8_t stateChannelIndex, srp_state_t state)
{
	if(stateChannelIndex >= srp->signalSize) return;
	
	srp_itemState_t *item = &srp->_signalState[stateChannelIndex];
	
	item->state = state;
	item->sendSignalPending = true;
	kernel.tickTimer.reset(&(item->timer));
}

srp_state_t srp_getStateByIndex(const stateReportProtocol_t *srp, uint8_t stateChannelIndex)
{
	if(stateChannelIndex >= srp->slotSize) return srp_state_undefined;
	else return srp->_slotState[stateChannelIndex].state;	
}

//**********************************************************************************************************************
// Private
//**********************************************************************************************************************

bool parseStateReport(const stateReportProtocol_t* srp, uint8_t sourceAddress, const uint8_t *data, uint8_t size)
{
	bool found = false;
	for(uint8_t i = 0; i < size; i+=3)
	{
		uint16_t channel = (data[i]<<8)|data[i+1];
		srp_state_t state = (srp_state_t) data[i+2];
			
		// TODO: optimize
		for(uint8_t j = 0; j < srp->slotSize; j++){
			
			const srp_stateSlot_t *slot = &srp->slots[j];
			srp_itemState_t *slotState = &srp->_slotState[j];
			
			if(channel == slot->channel){
					
				kernel.tickTimer.reset(&(slotState->timer));
					
				srp_state_t oldState = slotState->state;
				slotState->state = state;
					
				if(slot->action != nullptr && oldState != slotState->state){
					slot->action(slot->channel, state);
				}
					
				found = true;
				break;
			}
		}
	}

	return found;
}

bool parseStateRequest(const stateReportProtocol_t* srp, uint8_t sourceAddress, const uint8_t *data, uint8_t size)
{
	// in case size is 0 -> send all
	if(size == 0){
		for(uint8_t j = 0; j < srp->signalSize; j++){
			srp->_signalState[j].sendSignalPending = true;
		}
		return true;
	}
	
	// look for requested channels
	bool found = false;
	for(uint8_t i = 0; i < size; i+=2)
	{
		uint16_t channel = (data[i]<<8)|data[i+1];
			
		// TODO: optimize
		for(uint8_t j = 0; j < srp->signalSize; j++)
		{
			if(channel == srp->signals[j].channel)
			{
				srp->_signalState[j].sendSignalPending = true;
				found = true;
				break;
			}
		}
	}
	return found;
	
}

bool parseStateReportSignalInformationRequest(const stateReportProtocol_t* srp, uint8_t sourceAddress, const uint8_t *data, uint8_t size)
{
	// in case size is 0 -> send all
	if(size == 0){
		for(uint8_t j = 0; j < srp->signalSize; j++){
			srp->_signalState[j].sendInformationPending = true;
		}
		return true;
	}
		
	// look for requested channels
	bool found = false;
	for(uint8_t i = 0; i < size; i+=2)
	{
		uint16_t channel = (data[i]<<8)|data[i+1];
			
		// TODO: optimize
		for(uint8_t j = 0; j < srp->signalSize; j++)
		{
			if(channel == srp->signals[j].channel)
			{
				srp->_signalState[j].sendInformationPending = true;
				found = true;
				break;
			}
		}
	}
	return found;
}

bool parseStateReportSlotInformationRequest(const stateReportProtocol_t* srp, uint8_t sourceAddress, const uint8_t *data, uint8_t size)
{
	// in case size is 0 -> send all
	if(size == 0){
		for(uint8_t j = 0; j < srp->slotSize; j++){
			srp->_slotState[j].sendInformationPending = true;
		}
		return true;
	}
		
	// look for requested channels
	bool found = false;
	for(uint8_t i = 0; i < size; i+=2)
	{
		uint16_t channel = (data[i]<<8)|data[i+1];
			
		// TODO: optimize
		for(uint8_t j = 0; j < srp->slotSize; j++)
		{
			if(channel == srp->slots[j].channel)
			{
				srp->_slotState[j].sendInformationPending = true;
				found = true;
				break;
			}
		}
	}
	return found;
}

bool sendStateReportSignalInformation(const srp_stateSignal_t *stateSignal)
{
	bus_message_t msg;
	if(kernel.bus.getMessageSlot(&msg) == false) return false; // Abort if TX buffer full
		
	kernel.bus.writeHeader(&msg, BROADCAST, busProtocol_stateReportProtocol, srp_cmd_signalInformationReport, busPriority_low);
	kernel.bus.pushWord16(&msg, stateSignal->channel);
	kernel.bus.pushWord16(&msg, stateSignal->interval);
	kernel.bus.pushString(&msg, &stateSignal->description[0]);
	kernel.bus.send(&msg);
		
	return true;
}

bool sendStateReportSlotInformation(const srp_stateSlot_t *stateSlot)
{
	bus_message_t msg;
	if(kernel.bus.getMessageSlot(&msg) == false) return false; // Abort if TX buffer full
		
	kernel.bus.writeHeader(&msg, BROADCAST, busProtocol_stateReportProtocol, srp_cmd_slotInformationReport, busPriority_low);
	kernel.bus.pushWord16(&msg, stateSlot->channel);
	kernel.bus.pushWord16(&msg, stateSlot->timeout);
	kernel.bus.pushString(&msg, &stateSlot->description[0]);
	kernel.bus.send(&msg);
		
	return true;
}

void sendStateReports(const stateReportProtocol_t* srp)
{
	bus_message_t msg;
	uint8_t stateCount = 0;
		
	for(uint8_t i = 0;  i < srp->signalSize; i++)
	{
		if(srp->_signalState[i].sendSignalPending == false) continue;
			
		if(stateCount==0){
			if( kernel.bus.getMessageSlot(&msg) == false ){
				return; // Abort if TX buffer full
			}
			kernel.bus.writeHeader(&msg, BROADCAST, busProtocol_stateReportProtocol, srp_cmd_state, busPriority_low);
		}
			
		const srp_stateSignal_t *sig = &srp->signals[i];
		srp->_signalState[i].sendSignalPending = false;
		kernel.bus.pushWord16(&msg, sig->channel);
		kernel.bus.pushByte(&msg, srp->_signalState[i].state);
			
		stateCount++;
			
		if(stateCount>=21){
			kernel.bus.send(&msg);
			stateCount = 0;
		}
	}

	if(stateCount){
		kernel.bus.send(&msg);
	}
}

#ifdef __cplusplus
}
#endif