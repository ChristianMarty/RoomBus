

#include "triggerProtocol.h"
#include "string.h"


bool sendSoltName(const kernel_t *kernel, const tsp_triggerSlot_t *slot);



void tsp_mainHandler(const kernel_t *kernel, const triggerSlotProtocol_t* tsp)
{
	for(uint8_t i = 0;  i < tsp->slotSize; i++)
	{
		if(tsp->_slotsStatus[i].sendNamePending == true)
		{
			if( sendSoltName(kernel,&tsp->slots[i])) tsp->_slotsStatus[i].sendNamePending = false;
			break;
		}
	}	
	
}

void tsp_receiveHandler(const kernel_t *kernel, uint8_t sourceAddress, uint8_t command, const uint8_t *data, uint8_t size, const triggerSlotProtocol_t* tsp)
{
	if(command == tsp_cmd_triggerSlot) tsp_parseTrigger(kernel, sourceAddress, data, size);
	else if(command == tsp_cmd_slotNameRequest) tsp_parseSlotNameRequest(kernel, sourceAddress, data, size, tsp);
}

void tsp_parseTrigger(const kernel_t *kernel, uint8_t sourceAddress, const uint8_t *data, uint8_t size)
{
	for(uint8_t i = 0; i<size; i++)
	{
		tsp_onTrigger_callback(kernel, sourceAddress, data[i]);
	} 
}

void tsp_triggerAction(const kernel_t *kernel, const uint8_t triggerChannelNumber, const tsp_triggerSlot_t *slotList, const uint8_t slotListSize)
{
	for(uint8_t i = 0; i<slotListSize; i++)
	{
		if(triggerChannelNumber == slotList[i].channel) 
		{	
			slotList[i].action(triggerChannelNumber);
			break;
		}
	}
}

void tsp_parseSignalNameRequest(const kernel_t *kernel, uint8_t sourceAddress, const uint8_t *data, uint8_t size, const tsp_triggerSignal_t *signalList, const uint8_t signalListSize)
{
	for(uint8_t i = 0; i < size; i++)
	{
		for(uint8_t j = 0; j <signalListSize; j++)
		{
			if(data[i] == signalList[j].channel)
			{
				bus_message_t msg;
				kernel->bus.getMessageSlot(&msg);
				kernel->bus.writeHeader(&msg,BUS_BROADCAST_ADDRESS, busProtocol_triggerProtocol, 0x01, busPriority_low);
				kernel->bus.pushByte(&msg, signalList[j].destinationAddress);
				kernel->bus.pushByte(&msg, signalList[j].channel);
				kernel->bus.pushString(&msg, &signalList[j].description[0]);
				kernel->bus.send(&msg);
				break;
			}
		}
	}
}

bool sendSoltName(const kernel_t *kernel, const tsp_triggerSlot_t *slot)
{
	bus_message_t msg;
	if( kernel->bus.getMessageSlot(&msg) == false ) return false; // Abort if tx buffer full

	kernel->bus.writeHeader(&msg,BUS_BROADCAST_ADDRESS, busProtocol_triggerProtocol, 0x02, busPriority_low);
	kernel->bus.pushByte(&msg, slot->sourceAddress);
	kernel->bus.pushByte(&msg, slot->channel);
	kernel->bus.pushString(&msg, &slot->description[0]);
	kernel->bus.send(&msg);
	
	return true;
}


void tsp_parseSlotNameRequest(const kernel_t *kernel, uint8_t sourceAddress, const uint8_t *data, uint8_t size, const triggerSlotProtocol_t* tsp)
{
	if(size == 0)
	{
		for(uint8_t j = 0; j < tsp->slotSize; j++)
		{
			tsp->_slotsStatus[j].sendNamePending = true;
		}
	}
	
	for(uint8_t i = 0; i < size; i++)
	{
		for(uint8_t j = 0; j < tsp->slotSize; j++)
		{
			if(data[i] ==  tsp->slots[j].channel)
			{
				tsp->_slotsStatus[j].sendNamePending = true;
				break;
			}
		}
	}
}

void tep_sendTriggerByGroup(const kernel_t *kernel, const uint8_t *indexList ,const uint8_t indexListSize, const tsp_triggerSignal_t *signalList, uint8_t signalListSize)
{
	for(uint8_t j = 0; j< indexListSize; j++)
	{
		for(uint8_t i = 0 ; i < signalListSize; i++)
		{
			if(indexList[j] == signalList[i].groupIndex)
			{
				bus_message_t msg;
				kernel->bus.getMessageSlot(&msg);
				kernel->bus.writeHeader(&msg,signalList[i].destinationAddress, busProtocol_triggerProtocol, 0x00, busPriority_low);
				kernel->bus.pushByte(&msg, signalList[i].channel );
				kernel->bus.send(&msg);
			}
		}
	}
}

void tep_sendTriggerByChannel(const kernel_t *kernel, uint8_t destinationAddress, uint8_t *triggerList, uint8_t triggerListSize)
{
	if(triggerListSize > 64) triggerListSize = 64;
 	
	bus_message_t msg;
	kernel->bus.getMessageSlot(&msg);
	kernel->bus.writeHeader(&msg,destinationAddress, busProtocol_triggerProtocol, 0x00, busPriority_low);
	kernel->bus.pushArray(&msg, &triggerList[0],triggerListSize);
	kernel->bus.send(&msg);
}
