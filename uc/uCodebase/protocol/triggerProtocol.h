
#ifndef TRIGGER_SYSTEM_PROTOCOL_H_
#define TRIGGER_SYSTEM_PROTOCOL_H_

#include "sam.h"
#include "kernel/kernel.h"

typedef enum {
	tsp_cmd_triggerSlot = 0,
	tsp_cmd_signalNameReport = 1,
	tsp_cmd_slotNameReport = 2,
	tsp_cmd_signalNameRequest = 3,
	tsp_cmd_slotNameRequest = 4
}tsp_commands_t;

typedef void (*tsp_triggerAction_t)(uint8_t triggerChannelNumber);

typedef struct{
	uint8_t sendNamePending :1;
	uint8_t reserved :7;
}tsp_itemStatus_t;

typedef struct{
	const uint8_t groupIndex;
	const uint8_t destinationAddress;
	const uint8_t channel;
	const char *description;
}tsp_triggerSignal_t;

typedef struct{
	const uint8_t sourceAddress;
	const uint8_t channel;
	const char *description;
	const tsp_triggerAction_t action;
}tsp_triggerSlot_t;

typedef struct{
	const tsp_triggerSignal_t* signals;
	const uint8_t signalSize;
	const tsp_triggerSlot_t* slots;
	const uint8_t slotSize;
	
	tsp_itemStatus_t *_slotsStatus;
	
}triggerSlotProtocol_t;

void tsp_mainHandler(const kernel_t *kernel, const triggerSlotProtocol_t* tsp);

void tsp_receiveHandler(const kernel_t *kernel, uint8_t sourceAddress, uint8_t command, const uint8_t *data, uint8_t size, const triggerSlotProtocol_t* tsp);

void tsp_parseTrigger(const kernel_t *kernel, uint8_t sourceAddress, const uint8_t *data, uint8_t size);

void tsp_parseSignalNameRequest(const kernel_t *kernel, uint8_t sourceAddress, const uint8_t *data, uint8_t size, const tsp_triggerSignal_t *signalList, const uint8_t signalListSize);

void tsp_parseSlotNameRequest(const kernel_t *kernel, uint8_t sourceAddress, const uint8_t *data, uint8_t size, const triggerSlotProtocol_t* tsp);

void tsp_triggerAction(const kernel_t *kernel, const uint8_t triggerChannelNumber, const tsp_triggerSlot_t *slotList, const uint8_t slotListSize);

void tsp_onTrigger_callback(const kernel_t *kernel, uint8_t sourceAddress, uint8_t triggerChannelNumber); 

void tep_sendTriggerByGroup(const kernel_t *kernel, const uint8_t *indexList ,const uint8_t indexListSize, const tsp_triggerSignal_t *signalList, uint8_t signalListSize);

void tep_sendTriggerByChannel(const kernel_t *kernel, uint8_t destinationAddress, uint8_t *triggerList, uint8_t triggerListSize);

#endif /* TRIGGER_SYSTEM_PROTOCOL_H_ */