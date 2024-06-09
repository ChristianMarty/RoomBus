
#ifndef EVENT_SYSTEM_PROTOCOL_H_
#define EVENT_SYSTEM_PROTOCOL_H_

#include "sam.h"
#include "kernel/kernel.h"

typedef enum {
	esp_cmd_eventSlot = 0,
	esp_cmd_signalNameReport = 1,
	esp_cmd_slotNameReport = 2,
	esp_cmd_signalNameRequest = 3,
	esp_cmd_slotNameRequest = 4
}esp_commands_t;

typedef void (*esp_eventAction_t)(uint8_t eventChannelNumber, bool state);

typedef struct{
	const uint8_t destinationAddress;
	const uint8_t channel;
	const char *description;
}esp_eventSignal_t;

typedef struct{
	uint32_t timer;
	bool state;
}esp_eventState_t;

typedef struct{
	const uint8_t sourceAddress;
	const uint8_t channel;
	const char *description;
	const uint16_t timeout;
	esp_eventState_t* const state;
	const esp_eventAction_t action;
}esp_eventSlot_t;

typedef struct{
	const esp_eventSignal_t* signals;
	const uint8_t signalSize;

	const esp_eventSlot_t* slots;
	const uint8_t slotSize;
}eventSystemProtocol_t;

void esp_init(const kernel_t *kernel, const eventSystemProtocol_t* esp);

void esp_mainHandler(const kernel_t *kernel, const eventSystemProtocol_t* esp);

void esp_receiveHandler(const kernel_t *kernel, uint8_t sourceAddress, uint8_t command, const uint8_t *data, uint8_t size, const eventSystemProtocol_t* esp);

void esp_parseEvent(const kernel_t *kernel, uint8_t sourceAddress, const uint8_t *data, uint8_t size);

void esp_parseSignalNameRequest(const kernel_t *kernel, uint8_t sourceAddress, const uint8_t *data, uint8_t size, const esp_eventSignal_t *signalList, uint8_t signalListSize);

void esp_parseSlotNameRequest(const kernel_t *kernel, uint8_t sourceAddress, const uint8_t *data, uint8_t size, const esp_eventSlot_t *slotList, uint8_t slotListSize);

void esp_onEvent_callback(const kernel_t *kernel, uint8_t sourceAddress, uint8_t eventChannelNumber);

void esp_sendEvent(const kernel_t *kernel, uint8_t destinationAddress, uint8_t *eventList, uint8_t eventListSize);

void esp_sendEventSignal(const kernel_t *kernel, const esp_eventSignal_t *eventSignal);


#endif /* EVENT_SYSTEM_PROTOCOL_H_ */