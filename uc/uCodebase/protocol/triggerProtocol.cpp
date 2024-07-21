//**********************************************************************************************************************
// FileName : triggerProtocol.cpp
// FilePath : protocol/
// Author   : Christian Marty
// Date		: 12.06.2024
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#include "triggerProtocol.h"
#include "string.h"

#ifdef __cplusplus
extern "C" {
#endif
	
bool parseTrigger(const triggerSystemProtocol_t* tsp, uint8_t sourceAddress, const uint8_t *data, uint8_t size);
bool parseTriggerSignalInformationRequest(const triggerSystemProtocol_t* tsp, uint8_t sourceAddress, const uint8_t *data, uint8_t size);
bool parseTriggerSlotInformationRequest(const triggerSystemProtocol_t* tsp, uint8_t sourceAddress, const uint8_t *data, uint8_t size);

bool sendTriggerSignalInformation(const tsp_triggerSignal_t *triggerSignal);
bool sendTriggerSoltInformation(const tsp_triggerSlot_t *triggerSlot);
void sendTriggers(const triggerSystemProtocol_t* tsp);

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
			if(sendTriggerSoltInformation(&tsp->slots[i])) {
				tsp->_slotState[i].sendInformationPending = false;
			}
			break;
		}
	}
	
	for(uint8_t i = 0;  i < tsp->signalSize; i++) {
		if(tsp->_signalState[i].sendInformationPending == true) {
			if(sendTriggerSignalInformation(&tsp->signals[i])) {
				tsp->_signalState[i].sendInformationPending = false;
			}
			break;
		}
	}
	
	sendTriggers(tsp);
}

bool  tsp_receiveHandler(const triggerSystemProtocol_t* tsp, uint8_t sourceAddress, uint8_t command, const uint8_t *data, uint8_t size)
{
	switch(command){
		case tsp_cmd_trigger: return parseTrigger(tsp, sourceAddress, data, size);
		case tsp_cmd_slotInformationRequest: return parseTriggerSlotInformationRequest(tsp, sourceAddress, data, size);
		case tsp_cmd_signalInformationRequest: return parseTriggerSignalInformationRequest(tsp, sourceAddress, data, size);
	}
	return false;
}

void tsp_sendTriggerByIndex(const triggerSystemProtocol_t* tsp, const uint8_t *indexList, uint8_t indexListLength)
{
	for(uint8_t i = 0; i < indexListLength; i++)
	{
		tsp->_signalState[indexList[i]].sendTriggerPending = true;
	}
}

//**********************************************************************************************************************
// Private
//**********************************************************************************************************************

bool sendTriggerSignalInformation(const tsp_triggerSignal_t *triggerSignal)
{
	bus_message_t msg;
	if( kernel.bus.getMessageSlot(&msg) == false ) return false; // Abort if TX buffer full

	kernel.bus.writeHeader(&msg, BROADCAST, busProtocol_triggerProtocol, tsp_cmd_signalInformationReport, busPriority_low);
	kernel.bus.pushWord16(&msg, triggerSignal->channel);
	kernel.bus.pushString(&msg, &triggerSignal->description[0]);
	kernel.bus.send(&msg);
	
	return true;
}

bool sendTriggerSoltInformation(const tsp_triggerSlot_t *triggerSlot)
{
	bus_message_t msg;
	if( kernel.bus.getMessageSlot(&msg) == false ) return false; // Abort if TX buffer full

	kernel.bus.writeHeader(&msg, BROADCAST, busProtocol_triggerProtocol, tsp_cmd_slotInformationReport, busPriority_low);
	kernel.bus.pushWord16(&msg, triggerSlot->channel);
	kernel.bus.pushString(&msg, &triggerSlot->description[0]);
	kernel.bus.send(&msg);
	
	return true;
}

bool parseTrigger(const triggerSystemProtocol_t* tsp, uint8_t sourceAddress, const uint8_t *data, uint8_t size)
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

void sendTriggers(const triggerSystemProtocol_t* tsp)
{
	bus_message_t msg;
	uint8_t triggerCount = 0;
	
	for(uint8_t i = 0;  i < tsp->signalSize; i++) 
	{
		if(tsp->_signalState[i].sendTriggerPending == false) continue;
		
		if(triggerCount==0){
			if( kernel.bus.getMessageSlot(&msg) == false ){
				return; // Abort if TX buffer full
			}
			kernel.bus.writeHeader(&msg, BROADCAST, busProtocol_triggerProtocol, tsp_cmd_trigger, busPriority_low);
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

bool parseTriggerSignalInformationRequest(const triggerSystemProtocol_t* tsp, uint8_t sourceAddress, const uint8_t *data, uint8_t size)
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

bool parseTriggerSlotInformationRequest(const triggerSystemProtocol_t* tsp, uint8_t sourceAddress, const uint8_t *data, uint8_t size)
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