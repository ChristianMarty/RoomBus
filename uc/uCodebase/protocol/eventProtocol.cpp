//**********************************************************************************************************************
// FileName : eventProtocol.cpp
// FilePath : protocol/
// Author   : Christian Marty
// Date		: 15.06.2024
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#include "eventProtocol.h"
#include "string.h"

#ifdef __cplusplus
extern "C" {
#endif

bool parseEvent(const eventSystemProtocol_t* esp, uint8_t sourceAddress, const uint8_t *data, uint8_t size);
bool parseEventSignalInformationRequest(const eventSystemProtocol_t* esp, uint8_t sourceAddress, const uint8_t *data, uint8_t size);
bool parseEventSlotInformationRequest(const eventSystemProtocol_t* esp, uint8_t sourceAddress, const uint8_t *data, uint8_t size);

bool sendEventSignalInformation(const esp_eventSignal_t *eventSignal);
bool sendEventSlotInformation(const esp_eventSlot_t *eventSlot);
void sendEvents(const eventSystemProtocol_t* esp);

void esp_initialize(const eventSystemProtocol_t* esp)
{ 
	for(int i = 0; i < esp->slotSize; i++){
		kernel.tickTimer.reset(&(esp->_slotState[i].timer));	
		
		esp->_slotState[i].sendInformationPending = false;
		esp->_slotState[i].state = false;
	}
	
	for(int i = 0; i < esp->signalSize; i++){
		kernel.tickTimer.reset(&(esp->_signalState[i].timer));
		
		esp->_signalState[i].sendInformationPending = false;
		esp->_signalState[i].state = false;
	}
}

void esp_mainHandler(const eventSystemProtocol_t* esp)
{
	// handle slot timeout
	for(int i = 0; i < esp->slotSize; i++){
		if(kernel.tickTimer.delay1ms( &(esp->_slotState[i].timer), esp->slots[i].timeout)){
			bool oldState = esp->_slotState[i].state;
			esp->_slotState[i].state = false;
			
			if(esp->slots[i].action != nullptr && oldState != esp->_slotState[i].state ){
				esp->slots[i].action(esp->slots[i].channel, false);
			}
		}
	}
	
	// handle signal interval
	for(int i = 0; i < esp->signalSize; i++){
		if(esp->_signalState[i].state == true && kernel.tickTimer.delay1ms(&(esp->_signalState[i].timer), esp->signals[i].interval)){
			esp->_signalState[i].sendSignalPending = true;
		}
	}
	
	// send slot information
	for(uint8_t i = 0;  i < esp->slotSize; i++) {
		
		if(esp->_slotState[i].sendInformationPending == true){
			if(sendEventSlotInformation(&esp->slots[i])) {
				esp->_slotState[i].sendInformationPending = false;
			}
			break;
		}
	}
	
	// send signal information
	for(uint8_t i = 0;  i < esp->signalSize; i++) {
		if(esp->_signalState[i].sendInformationPending == true) {
			if(sendEventSignalInformation(&esp->signals[i])) {
				esp->_signalState[i].sendInformationPending = false;
			}
			break;
		}
	}
	
	sendEvents(esp);
}

bool esp_receiveHandler(const eventSystemProtocol_t* esp, uint8_t sourceAddress, uint8_t command, const uint8_t *data, uint8_t size)
{
	switch(command){
		case esp_cmd_event: return parseEvent(esp, sourceAddress, data, size);
		case esp_cmd_slotInformationRequest: return parseEventSlotInformationRequest(esp, sourceAddress, data, size);
		case esp_cmd_signalInformationRequest: return parseEventSignalInformationRequest(esp, sourceAddress, data, size);
	}
	return false;
}

bool esp_getStateByIndex(const eventSystemProtocol_t* esp, uint8_t index)
{	
	if(index >= esp->slotSize) return false;
	return esp->_slotState[index].state;
}

bool esp_getStateByChannel(const eventSystemProtocol_t* esp, uint16_t channel)
{
	for(int i = 0; i < esp->slotSize; i++){
		if(esp->slots[i].channel ==  channel){
			return esp->_slotState[i].state;
		}
	}
}

void esp_setStateByIndex(const eventSystemProtocol_t* esp, uint8_t index, bool state)
{
	if(index >= esp->signalSize) return;
	esp->_signalState[index].state = state;
}

void esp_setStateByChannel(const eventSystemProtocol_t* esp, uint16_t channel, bool state)
{
	for(int i = 0; i < esp->signalSize; i++){
		if(esp->signals[i].channel ==  channel){
			esp->_signalState[i].state = state;
			return;
		}
	}
}

bool esp_sendEventByIndex(const eventSystemProtocol_t* esp, uint8_t index)
{
	if(index >= esp->signalSize) return false;
	
	bus_message_t msg;
	if( kernel.bus.getMessageSlot(&msg) == false ) return false; // Abort if TX buffer full
	
	esp_eventSignal_t sig = esp->signals[index];
	esp_itemState_t sigState = esp->_signalState[index];
	
	kernel.bus.writeHeader(&msg, BROADCAST, busProtocol_eventProtocol, esp_cmd_event, busPriority_low);
	kernel.bus.pushWord16(&msg, sig.channel);
	kernel.bus.send(&msg);
	
}

//**********************************************************************************************************************
// Private
//**********************************************************************************************************************

bool sendEventSignalInformation(const esp_eventSignal_t *eventSignal)
{
	bus_message_t msg;
	if( kernel.bus.getMessageSlot(&msg) == false ) return false; // Abort if TX buffer full
	
	kernel.bus.writeHeader(&msg, BROADCAST, busProtocol_eventProtocol, esp_cmd_signalInformationReport, busPriority_low);
	kernel.bus.pushWord16(&msg, eventSignal->channel);
	kernel.bus.pushWord16(&msg, eventSignal->interval);
	kernel.bus.pushString(&msg, &eventSignal->description[0]);
	kernel.bus.send(&msg);
	
	return true;
}

bool sendEventSlotInformation(const esp_eventSlot_t *eventSlot)
{
	bus_message_t msg;
	if( kernel.bus.getMessageSlot(&msg) == false ) return false; // Abort if TX buffer full
	
	kernel.bus.writeHeader(&msg, BROADCAST, busProtocol_eventProtocol, esp_cmd_slotInformationReport, busPriority_low);
	kernel.bus.pushWord16(&msg, eventSlot->channel);
	kernel.bus.pushWord16(&msg, eventSlot->timeout);
	kernel.bus.pushString(&msg, &eventSlot->description[0]);
	kernel.bus.send(&msg);
	
	return true;
}

bool parseEvent(const eventSystemProtocol_t* esp, uint8_t sourceAddress, const uint8_t *data, uint8_t size)
{
	bool found = false;
	for(uint8_t i = 0; i < size; i+=2)
	{
		uint16_t channel = (data[i]<<8)|data[i+1];
		
		// TODO: optimize
		for(uint8_t j = 0; j < esp->slotSize; j++){
			
			if(channel == esp->slots[j].channel){
				
				kernel.tickTimer.reset(&(esp->_slotState[j].timer));
				
				bool oldState = esp->_slotState[j].state;
				esp->_slotState[j].state = true;
				
				if(esp->slots[j].action != nullptr && oldState != esp->_slotState[j].state){
					esp->slots[j].action(esp->slots[j].channel, true);
				}
				
				found = true;
				break;
			}
		}
	}

	return found;
}

void sendEvents(const eventSystemProtocol_t* esp)
{
	bus_message_t msg;
	uint8_t eventCount = 0;
	
	for(uint8_t i = 0;  i < esp->signalSize; i++)
	{
		if(esp->_signalState[i].sendSignalPending == false) continue;
		
		if(eventCount==0){
			if( kernel.bus.getMessageSlot(&msg) == false ){
				return; // Abort if TX buffer full
			}
			kernel.bus.writeHeader(&msg, BROADCAST, busProtocol_eventProtocol, esp_cmd_event, busPriority_low);
		}
		
		const esp_eventSignal_t *sig = &esp->signals[i];
		esp->_signalState[i].sendSignalPending = false;
		kernel.bus.pushWord16(&msg, sig->channel);
		
		eventCount++;
		
		if(eventCount==32){
			kernel.bus.send(&msg);
			eventCount = 0;
		}
	}

	if(eventCount){
		kernel.bus.send(&msg);
	}
}

bool parseEventSignalInformationRequest(const eventSystemProtocol_t* esp, uint8_t sourceAddress, const uint8_t *data, uint8_t size)
{
	// in case size is 0 -> send all
	if(size == 0){
		for(uint8_t j = 0; j < esp->signalSize; j++){
			esp->_signalState[j].sendInformationPending = true;
		}
		return true;
	}
		
	// look for requested channels
	bool found = false;
	for(uint8_t i = 0; i < size; i+=2)
	{
		uint16_t channel = (data[i]<<8)|data[i+1];
			
		// TODO: optimize
		for(uint8_t j = 0; j < esp->signalSize; j++)
		{
			if(channel == esp->signals[j].channel)
			{
				esp->_signalState[j].sendInformationPending = true;
				found = true;
				break;
			}
		}
	}
	return found;
}

bool parseEventSlotInformationRequest(const eventSystemProtocol_t* esp, uint8_t sourceAddress, const uint8_t *data, uint8_t size)
{
	// in case size is 0 -> send all
	if(size == 0){
		for(uint8_t j = 0; j < esp->slotSize; j++){
			esp->_slotState[j].sendInformationPending = true;
		}
		return true;
	}
		
	// look for requested channels
	bool found = false;
	for(uint8_t i = 0; i < size; i+=2)
	{
		uint16_t channel = (data[i]<<8)|data[i+1];
			
		// TODO: optimize
		for(uint8_t j = 0; j < esp->slotSize; j++)
		{
			if(channel == esp->slots[j].channel)
			{
				esp->_slotState[j].sendInformationPending = true;
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