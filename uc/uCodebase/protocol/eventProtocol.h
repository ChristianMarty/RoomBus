//**********************************************************************************************************************
// FileName : eventProtocol.h
// FilePath : protocol/
// Author   : Christian Marty
// Date		: 15.06.2024
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#ifndef EVENT_SYSTEM_PROTOCOL_H_
#define EVENT_SYSTEM_PROTOCOL_H_

/* EXAMPLE *************************************************************************************************************

case busProtocol_eventProtocol:		return esp_receiveHandler(kernel, &eventSystem, sourceAddress, command, data, size);

const esp_eventSignal_t eventSignal[] = {
	// Add event signal list
	{<Channel>, <Description>, <Interval [ms]>}

};
#define eventSignalListSize (sizeof(eventSignal)/sizeof(esp_eventSignal_t))

const esp_eventSlot_t eventSlots[] = {
	// Add event slot list
	{<Channel>, <Description>, <Interval [ms]>, <triggerAction>}
		
	{0x0100, "LED 3 Slot", 2000, nullptr}
};
#define eventSlotListSize (sizeof(eventSlots)/sizeof(esp_eventSlot_t))

esp_itemState_t eventSignalStatusList[triggerSlotListSize];
esp_itemState_t eventSlotStatusList[eventSlotListSize];

const eventSystemProtocol_t eventSystem = {
	.signals = eventSignal,
	.signalSize = eventSignalListSize,
	.slots = eventSlots,
	.slotSize = eventSlotListSize,
	._signalState = eventSignalStatusList,
	._slotState = eventSlotStatusList
};

***********************************************************************************************************************/
#include "sam.h"
#include "common/kernel.h"

#ifdef __cplusplus
extern "C" {
#endif
	
typedef enum {
	esp_cmd_event,
	esp_cmd_reserved0,
	esp_cmd_reserved1,
	esp_cmd_reserved2,
	
	esp_cmd_signalInformationReport,
	esp_cmd_slotInformationReport,
	
	esp_cmd_signalInformationRequest,
	esp_cmd_slotInformationRequest
}esp_commands_t;

typedef void (*esp_eventAction_t)(uint16_t eventChannelNumber, bool state);

typedef struct{
	uint32_t timer;
	
	uint8_t state :1;
	uint8_t sendInformationPending :1;
	uint8_t sendSignalPending :1;
	uint8_t reserved :5;
}esp_itemState_t;

typedef struct{
	const uint16_t channel;
	const char *description;
	const uint16_t interval;
}esp_eventSignal_t;

typedef struct{
	const uint16_t channel;
	const char *description;
	const uint16_t timeout;
	const esp_eventAction_t action;
}esp_eventSlot_t;

typedef struct{
	const esp_eventSignal_t* signals;
	const uint8_t signalSize;

	const esp_eventSlot_t* slots;
	const uint8_t slotSize;
	
	esp_itemState_t *_signalState;
	esp_itemState_t *_slotState;
}eventSystemProtocol_t;

void esp_initialize(const eventSystemProtocol_t* esp);
void esp_mainHandler(const eventSystemProtocol_t* esp);
bool esp_receiveHandler(const eventSystemProtocol_t* esp, uint8_t sourceAddress, uint8_t command, const uint8_t *data, uint8_t size);

void esp_setStateByIndex(const eventSystemProtocol_t* esp, uint8_t index, bool state);
void esp_setStateByChannel(const eventSystemProtocol_t* esp, uint16_t channel, bool state);

bool esp_sendEventByIndex(const eventSystemProtocol_t* esp, uint8_t index);

bool esp_getStateByIndex(const eventSystemProtocol_t* esp, uint8_t index);
bool esp_getStateByChannel(const eventSystemProtocol_t* esp, uint16_t channel);

#ifdef __cplusplus
}
#endif

#endif /* EVENT_SYSTEM_PROTOCOL_H_ */