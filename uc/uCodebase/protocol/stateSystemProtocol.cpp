//**********************************************************************************************************************
// FileName : stateSystemProtocol.cpp
// FilePath : protocol/
// Author   : Christian Marty
// Date		: 08.09.2024
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#include "stateSystemProtocol.h"

#ifdef __cplusplus
extern "C" {
#endif

bool _ssp_parseStateReport(const stateSystemProtocol_t* srp, uint8_t sourceAddress, const uint8_t *data, uint8_t size);
bool _ssp_parseStateRequest(const stateSystemProtocol_t* srp, uint8_t sourceAddress, const uint8_t *data, uint8_t size);
bool _ssp_parseStateReportSignalInformationRequest(const stateSystemProtocol_t* srp, uint8_t sourceAddress, const uint8_t *data, uint8_t size);
bool _ssp_parseStateReportSlotInformationRequest(const stateSystemProtocol_t* srp, uint8_t sourceAddress, const uint8_t *data, uint8_t size);

bool _ssp_sendStateReportSignalInformation(const ssp_stateSignal_t *stateSignal);
bool _ssp_sendStateReportSlotInformation(const ssp_stateSlot_t *stateSlot);
void _ssp_sendStateReports(const stateSystemProtocol_t* ssp);

void ssp_initialize(const stateSystemProtocol_t *ssp)
{
	for(int i = 0; i < ssp->slotSize; i++){
		kernel.tickTimer.reset(&(ssp->_slotState[i].timer));
		
		ssp->_slotState[i].sendInformationPending = false;
		ssp->_slotState[i].state = srp_state_undefined;
	}
	
	for(int i = 0; i < ssp->signalSize; i++){
		kernel.tickTimer.reset(&(ssp->_signalState[i].timer));
		
		ssp->_signalState[i].sendInformationPending = false;
		ssp->_signalState[i].state = srp_state_undefined;
	}
	
}

void ssp_mainHandler(const stateSystemProtocol_t *ssp)
{
	for(int i = 0; i < ssp->signalSize; i++){
		if(kernel.tickTimer.delay1ms(&(ssp->_signalState[i].timer), ssp->signals[i].interval*1000)){
			ssp->_signalState[i].sendSignalPending = true;
		}
	}
	
	// send signal information
	for(uint8_t i = 0;  i < ssp->signalSize; i++) {
		if(ssp->_signalState[i].sendInformationPending == true) {
			if(_ssp_sendStateReportSignalInformation(&ssp->signals[i])) {
				ssp->_signalState[i].sendInformationPending = false;
			}
			break;
		}
	}
	
	// send slot information
	for(uint8_t i = 0;  i < ssp->slotSize; i++) {
		if(ssp->_slotState[i].sendInformationPending == true){
			if(_ssp_sendStateReportSlotInformation(&ssp->slots[i])) {
				ssp->_slotState[i].sendInformationPending = false;
			}
			break;
		}
	}
	
	_ssp_sendStateReports(ssp);
}

bool ssp_receiveHandler(const stateSystemProtocol_t *srp, uint8_t sourceAddress, uint8_t command, const uint8_t *data, uint8_t size)
{
	switch(command){
		case srp_cmd_state: return _ssp_parseStateReport(srp, sourceAddress, data, size);
		case srp_cmd_stateRequest: return _ssp_parseStateRequest(srp, sourceAddress, data, size);
		case srp_cmd_signalInformationRequest: return _ssp_parseStateReportSignalInformationRequest(srp, sourceAddress, data, size);
		case srp_cmd_slotInformationRequest: return _ssp_parseStateReportSlotInformationRequest(srp, sourceAddress, data, size);
	}
	return false;
}

void ssp_setStateByIndex(const stateSystemProtocol_t *srp, uint8_t stateChannelIndex, ssp_state_t state)
{
	if(stateChannelIndex >= srp->signalSize) return;
	
	ssp_itemState_t *item = &srp->_signalState[stateChannelIndex];
	
	item->state = state;
	item->sendSignalPending = true;
	kernel.tickTimer.reset(&(item->timer));
}

ssp_state_t ssp_getStateByIndex(const stateSystemProtocol_t *srp, uint8_t stateChannelIndex)
{
	if(stateChannelIndex >= srp->slotSize) return srp_state_undefined;
	else return srp->_slotState[stateChannelIndex].state;	
}

//**********************************************************************************************************************
// Private
//**********************************************************************************************************************

bool _ssp_parseStateReport(const stateSystemProtocol_t* srp, uint8_t sourceAddress, const uint8_t *data, uint8_t size)
{
	bool found = false;
	for(uint8_t i = 0; i < size; i+=3)
	{
		uint16_t channel = (data[i]<<8)|data[i+1];
		ssp_state_t state = (ssp_state_t) data[i+2];
			
		// TODO: optimize
		for(uint8_t j = 0; j < srp->slotSize; j++){
			
			const ssp_stateSlot_t *slot = &srp->slots[j];
			ssp_itemState_t *slotState = &srp->_slotState[j];
			
			if(channel == slot->channel){
					
				kernel.tickTimer.reset(&(slotState->timer));
					
				ssp_state_t oldState = slotState->state;
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

bool _ssp_parseStateRequest(const stateSystemProtocol_t* srp, uint8_t sourceAddress, const uint8_t *data, uint8_t size)
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

bool _ssp_parseStateReportSignalInformationRequest(const stateSystemProtocol_t* ssp, uint8_t sourceAddress, const uint8_t *data, uint8_t size)
{
	// in case size is 0 -> send all
	if(size == 0){
		for(uint8_t j = 0; j < ssp->signalSize; j++){
			ssp->_signalState[j].sendInformationPending = true;
		}
		return true;
	}
		
	// look for requested channels
	bool found = false;
	for(uint8_t i = 0; i < size; i+=2)
	{
		uint16_t channel = (data[i]<<8)|data[i+1];
			
		// TODO: optimize
		for(uint8_t j = 0; j < ssp->signalSize; j++)
		{
			if(channel == ssp->signals[j].channel)
			{
				ssp->_signalState[j].sendInformationPending = true;
				found = true;
				break;
			}
		}
	}
	return found;
}

bool _ssp_parseStateReportSlotInformationRequest(const stateSystemProtocol_t* ssp, uint8_t sourceAddress, const uint8_t *data, uint8_t size)
{
	// in case size is 0 -> send all
	if(size == 0){
		for(uint8_t j = 0; j < ssp->slotSize; j++){
			ssp->_slotState[j].sendInformationPending = true;
		}
		return true;
	}
		
	// look for requested channels
	bool found = false;
	for(uint8_t i = 0; i < size; i+=2)
	{
		uint16_t channel = (data[i]<<8)|data[i+1];
			
		// TODO: optimize
		for(uint8_t j = 0; j < ssp->slotSize; j++)
		{
			if(channel == ssp->slots[j].channel)
			{
				ssp->_slotState[j].sendInformationPending = true;
				found = true;
				break;
			}
		}
	}
	return found;
}

bool _ssp_sendStateReportSignalInformation(const ssp_stateSignal_t *stateSignal)
{
	bus_message_t msg;
	if(kernel.bus.getMessageSlot(&msg) == false) return false; // Abort if TX buffer full
		
	kernel.bus.writeHeader(&msg, BROADCAST, busProtocol_stateSystemProtocol, srp_cmd_signalInformationReport, busPriority_low);
	kernel.bus.pushWord16(&msg, stateSignal->channel);
	kernel.bus.pushWord16(&msg, stateSignal->interval);
	kernel.bus.pushString(&msg, &stateSignal->description[0]);
	kernel.bus.send(&msg);
		
	return true;
}

bool _ssp_sendStateReportSlotInformation(const ssp_stateSlot_t *stateSlot)
{
	bus_message_t msg;
	if(kernel.bus.getMessageSlot(&msg) == false) return false; // Abort if TX buffer full
		
	kernel.bus.writeHeader(&msg, BROADCAST, busProtocol_stateSystemProtocol, srp_cmd_slotInformationReport, busPriority_low);
	kernel.bus.pushWord16(&msg, stateSlot->channel);
	kernel.bus.pushWord16(&msg, stateSlot->timeout);
	kernel.bus.pushString(&msg, &stateSlot->description[0]);
	kernel.bus.send(&msg);
		
	return true;
}

void _ssp_sendStateReports(const stateSystemProtocol_t* ssp)
{
	bus_message_t msg;
	uint8_t stateCount = 0;
		
	for(uint8_t i = 0;  i < ssp->signalSize; i++)
	{
		if(ssp->_signalState[i].sendSignalPending == false) continue;
			
		if(stateCount==0){
			if( kernel.bus.getMessageSlot(&msg) == false ){
				return; // Abort if TX buffer full
			}
			kernel.bus.writeHeader(&msg, BROADCAST, busProtocol_stateSystemProtocol, srp_cmd_state, busPriority_low);
		}
			
		const ssp_stateSignal_t *sig = &ssp->signals[i];
		ssp->_signalState[i].sendSignalPending = false;
		kernel.bus.pushWord16(&msg, sig->channel);
		kernel.bus.pushByte(&msg, ssp->_signalState[i].state);
			
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