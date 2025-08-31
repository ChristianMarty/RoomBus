//**********************************************************************************************************************
// FileName : triggerSystemProtocol.cpp
// FilePath : protocol/
// Author   : Christian Marty
// Date		: 08.09.2024
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#include "triggerSystemProtocol.h"
#include "string.h"

#ifdef __cplusplus
extern "C" {
#endif
	
bool _tsp_parseTrigger(const triggerSystemProtocol_t* tsp, uint8_t sourceAddress, const uint8_t *data, uint8_t size);
bool _tsp_parseTriggerSignalInformationRequest(const triggerSystemProtocol_t* tsp, uint8_t sourceAddress, const uint8_t *data, uint8_t size);
bool _tsp_parseTriggerSlotInformationRequest(const triggerSystemProtocol_t* tsp, uint8_t sourceAddress, const uint8_t *data, uint8_t size);

bool _tsp_sendTriggerSignalInformation(const tsp_triggerSignal_t *triggerSignal);
bool _tsp_sendTriggerSoltInformation(const tsp_triggerSlot_t *triggerSlot);
void _tsp_sendTriggers(const triggerSystemProtocol_t* tsp);

void tsp_initialize(const triggerSystemProtocol_t* tsp)
{
	for(uint8_t i = 0;  i < tsp->slotSize; i++) {
		tsp->_slotState[i].sendInformationPending = false;
		tsp->_slotState[i].sendTriggerPending = false;
	}
	
	for(uint8_t i = 0;  i < tsp->signalSize; i++) {
		tsp->_signalState[i].sendInformationPending = false;
		tsp->_signalState[i].sendTriggerPending = false;
	}
}

void tsp_mainHandler(const triggerSystemProtocol_t* tsp)
{
	for(uint8_t i = 0;  i < tsp->slotSize; i++) {
		
		if(tsp->_slotState[i].sendInformationPending == true){
			if(_tsp_sendTriggerSoltInformation(&tsp->slots[i])) {
				tsp->_slotState[i].sendInformationPending = false;
			}
			break;
		}
	}
	
	for(uint8_t i = 0;  i < tsp->signalSize; i++) {
		if(tsp->_signalState[i].sendInformationPending == true) {
			if(_tsp_sendTriggerSignalInformation(&tsp->signals[i])) {
				tsp->_signalState[i].sendInformationPending = false;
			}
			break;
		}
	}
	
	_tsp_sendTriggers(tsp);
}

bool  tsp_receiveHandler(const triggerSystemProtocol_t* tsp, const bus_rxMessage_t *message)
{
	switch(message->command){
		case tsp_cmd_trigger: return _tsp_parseTrigger(tsp, message->sourceAddress, message->data, message->length);
		case tsp_cmd_slotInformationRequest: return _tsp_parseTriggerSlotInformationRequest(tsp, message->sourceAddress, message->data, message->length);
		case tsp_cmd_signalInformationRequest: return _tsp_parseTriggerSignalInformationRequest(tsp, message->sourceAddress, message->data, message->length);
	}
	return false;
}


void tsp_sendTriggersByIndex(const triggerSystemProtocol_t* tsp, const uint8_t *indexList, uint8_t indexListLength)
{
	for(uint8_t i = 0; i < indexListLength; i++)
	{
		tsp->_signalState[indexList[i]].sendTriggerPending = true;
	}
}

bool tsp_sendTriggerByIndex(const triggerSystemProtocol_t* tsp, uint8_t index)
{
	if(tsp->signalSize < index) return false;
	
	tsp->_signalState[index].sendTriggerPending = true;
	return true;
}

//**********************************************************************************************************************
// Private
//**********************************************************************************************************************

bool _tsp_sendTriggerSignalInformation(const tsp_triggerSignal_t *triggerSignal)
{
	bus_txMessage_t msg;
	if( kernel.bus.getMessageSlot(&msg) == false ) return false; // Abort if TX buffer full

	kernel.bus.writeHeader(&msg, BROADCAST, busProtocol_triggerSystemProtocol, tsp_cmd_signalInformationReport, busPriority_low);
	kernel.bus.pushWord16(&msg, triggerSignal->channel);
	kernel.bus.pushString(&msg, &triggerSignal->description[0]);
	kernel.bus.send(&msg);
	
	return true;
}

bool _tsp_sendTriggerSoltInformation(const tsp_triggerSlot_t *triggerSlot)
{
	bus_txMessage_t msg;
	if( kernel.bus.getMessageSlot(&msg) == false ) return false; // Abort if TX buffer full

	kernel.bus.writeHeader(&msg, BROADCAST, busProtocol_triggerSystemProtocol, tsp_cmd_slotInformationReport, busPriority_low);
	kernel.bus.pushWord16(&msg, triggerSlot->channel);
	kernel.bus.pushString(&msg, &triggerSlot->description[0]);
	kernel.bus.send(&msg);
	
	return true;
}

bool _tsp_parseTrigger(const triggerSystemProtocol_t* tsp, uint8_t sourceAddress, const uint8_t *data, uint8_t size)
{
	bool found = false;
	for(uint8_t i = 0; i < size; i+=2)
	{
		uint16_t channel = (data[i]<<8)|data[i+1];
		
		// TODO: optimize
		for(uint8_t j = 0; j < tsp->slotSize; j++){
			if(channel == tsp->slots[j].channel){
				tsp->slots[j].action(channel);
				found = true;
				break;
			}
		}
	}

	return found;
}

void _tsp_sendTriggers(const triggerSystemProtocol_t* tsp)
{
	bus_txMessage_t msg;
	uint8_t triggerCount = 0;
	
	for(uint8_t i = 0;  i < tsp->signalSize; i++) 
	{
		if(tsp->_signalState[i].sendTriggerPending == false) continue;
		
		if(triggerCount==0){
			if( kernel.bus.getMessageSlot(&msg) == false ){
				return; // Abort if TX buffer full
			}
			kernel.bus.writeHeader(&msg, BROADCAST, busProtocol_triggerSystemProtocol, tsp_cmd_trigger, busPriority_low);
		}
		
		const tsp_triggerSignal_t *sig = &tsp->signals[i];
		tsp->_signalState[i].sendTriggerPending = false;
		kernel.bus.pushWord16(&msg, sig->channel);
		
		triggerCount++;
		
		if(triggerCount==32){
			kernel.bus.send(&msg);
			triggerCount = 0;
		}
	}
	
	if(triggerCount){
		kernel.bus.send(&msg);
	}
}

bool _tsp_parseTriggerSignalInformationRequest(const triggerSystemProtocol_t* tsp, uint8_t sourceAddress, const uint8_t *data, uint8_t size)
{
	// in case size is 0 -> send all
	if(size == 0){
		for(uint8_t j = 0; j < tsp->signalSize; j++){
			tsp->_signalState[j].sendInformationPending = true;
		}
		return true;
	}
	
	// look for requested channels
	bool found = false;
	for(uint8_t i = 0; i < size; i+=2)
	{
		uint16_t channel = (data[i]<<8)|data[i+1];
		
		// TODO: optimize
		for(uint8_t j = 0; j < tsp->signalSize; j++)
		{
			if(channel == tsp->signals[j].channel)
			{
				tsp->_signalState[j].sendInformationPending = true;
				found = true;
				break;
			}
		}
	}
	return found;
}

bool _tsp_parseTriggerSlotInformationRequest(const triggerSystemProtocol_t* tsp, uint8_t sourceAddress, const uint8_t *data, uint8_t size)
{
	// in case size is 0 -> send all
	if(size == 0){
		for(uint8_t j = 0; j < tsp->slotSize; j++){
			tsp->_slotState[j].sendInformationPending = true;
		}
		return true;
	}
	
	// look for requested channels
	bool found = false;
	for(uint8_t i = 0; i < size; i+=2)
	{
		uint16_t channel = (data[i]<<8)|data[i+1];
		
		// TODO: optimize
		for(uint8_t j = 0; j < tsp->slotSize; j++)
		{
			if(channel == tsp->slots[j].channel)
			{
				tsp->_slotState[j].sendInformationPending = true;
				found = true;
				break;
			}
		}
	}
	return found;
}

#ifdef __cplusplus
}
#endif