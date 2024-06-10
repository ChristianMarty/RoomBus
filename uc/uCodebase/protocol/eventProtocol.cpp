

#include "eventProtocol.h"
#include "string.h"


void esp_init(const kernel_t *kernel, const eventSystemProtocol_t* esp)
{ 
	for(int i = 0; i < esp->slotSize; i++)
	{
		kernel->tickTimer.reset(&(esp->slots[i].state->timer));	
		esp->slots[i].state->state = false;
	}
}

void esp_mainHandler(const kernel_t *kernel, const eventSystemProtocol_t* esp)
{
	for(int i = 0; i < esp->slotSize; i++)
	{
		if(kernel->tickTimer.delay1ms(&esp->slots[i].state->timer, esp->slots[i].timeout))
		{
			esp->slots[i].state->state = false;
			if(esp->slots[i].action != nullptr) esp->slots[i].action(esp->slots[i].channel, esp->slots[i].state->state);
		}
	}
	
	for(int i = 0; i < esp->signalSize; i++)
	{
		/*if(kernel->tickTimer.delay1ms(&esp->slots[i].state->timer, esp->slots[i].timeout))
		{
			esp->slots[i].state->state = false;
			if(esp->slots[i].action != nullptr) esp->slots[i].action(esp->slots[i].channel, esp->slots[i].state->state);
		}*/
	}
}

void esp_receiveHandler(const kernel_t *kernel, uint8_t sourceAddress, uint8_t command, const uint8_t *data, uint8_t size, const eventSystemProtocol_t* esp)
{
	if(command == esp_cmd_eventSlot)
	{
		for(uint8_t i = 0; i < size; i++)
		{
			for(uint8_t j = 0; j <esp->slotSize; j++)
			{
				if(data[i] == esp->slots[j].channel)
				{
					if((esp->slots[j].sourceAddress == UNFILTERED) || (esp->slots[j].sourceAddress == sourceAddress))
					{
						kernel->tickTimer.reset(&(esp->slots[j].state->timer));
						esp->slots[j].state->state = true;
						break;
					}
				}
			}
		}
	}
	else if(command == esp_cmd_slotNameRequest)
	{
		
		esp_parseSlotNameRequest(kernel, sourceAddress, data, size, esp->slots, esp->slotSize);
	}
}

void esp_parseEvent(const kernel_t *kernel, uint8_t sourceAddress, uint8_t *data, uint8_t size)
{
	for(uint8_t i = 0; i<size; i++)
	{
		esp_onEvent_callback(kernel, sourceAddress, data[i]);	
	}
}

void esp_parseSignalNameRequest(const kernel_t *kernel, uint8_t sourceAddress, const uint8_t *data, uint8_t size, const esp_eventSignal_t *signalList, const uint8_t signalListSize)
{
	for(uint8_t i = 0; i < size; i++)
	{
		for(uint8_t j = 0; j <signalListSize; j++)
		{
			if(i == signalList[j].channel)
			{
				bus_message_t msg;
				kernel->bus.getMessageSlot(&msg);
				kernel->bus.writeHeader(&msg,BUS_BROADCAST_ADDRESS, busProtocol_eventProtocol, esp_cmd_signalNameReport, busPriority_low);
				kernel->bus.pushByte(&msg, data[i]);
				kernel->bus.pushString(&msg, &signalList[j].description[0]);
				kernel->bus.send(&msg);
				break;
			}
		}
	}
}

void esp_parseSlotNameRequest(const kernel_t *kernel, uint8_t sourceAddress, const uint8_t *data, uint8_t size, const esp_eventSlot_t *slotList, uint8_t slotListSize)
{
	for(uint8_t i = 0; i < size; i++)
	{
		for(uint8_t j = 0; j <slotListSize; j++)
		{
			if(i == slotList[j].channel)
			{
				bus_message_t msg;
				kernel->bus.getMessageSlot(&msg);
				kernel->bus.writeHeader(&msg,BUS_BROADCAST_ADDRESS, busProtocol_eventProtocol, esp_cmd_slotNameReport, busPriority_low);
				kernel->bus.pushByte(&msg, data[i]);
				kernel->bus.pushWord16(&msg,slotList[j].timeout);
				kernel->bus.pushString(&msg, &slotList[j].description[0]);
				kernel->bus.send(&msg);
				break;
			}
		}
	}
}

void esp_sendEvent(const kernel_t *kernel, uint8_t destinationAddress, uint8_t *eventList, uint8_t eventListSize)
{
	if(eventListSize > 64) eventListSize = 64;
 	
	bus_message_t msg;
	kernel->bus.getMessageSlot(&msg);
	kernel->bus.writeHeader(&msg,destinationAddress, busProtocol_eventProtocol, esp_cmd_eventSlot, busPriority_low);
	kernel->bus.pushArray(&msg, &eventList[0],eventListSize);
	kernel->bus.send(&msg);
}

void esp_sendEventSignal(const kernel_t *kernel, const esp_eventSignal_t *eventSignal)
{
		bus_message_t msg;
		kernel->bus.getMessageSlot(&msg);
		kernel->bus.writeHeader(&msg, eventSignal->destinationAddress, busProtocol_eventProtocol, esp_cmd_eventSlot, busPriority_low);
		kernel->bus.pushByte(&msg, eventSignal->channel);
		kernel->bus.send(&msg);
}
