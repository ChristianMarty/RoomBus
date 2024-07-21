//**********************************************************************************************************************
// FileName : triggerProtocol.h
// FilePath : protocol/
// Author   : Christian Marty
// Date		: 12.06.2024
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#ifndef TRIGGER_SYSTEM_PROTOCOL_H_
#define TRIGGER_SYSTEM_PROTOCOL_H_

/* EXAMPLE *************************************************************************************************************

case busProtocol_triggerProtocol:		return tsp_receiveHandler(kernel, &triggerSystem, sourceAddress, command, data, size);

const tsp_triggerSignal_t triggerSignals[] = {
	// Add trigger signal list
	{<Channel>, <Description>}
};
#define triggerSignalListSize (sizeof(triggerSignals)/sizeof(tsp_triggerSignal_t))

const tsp_triggerSlot_t triggerSlotList[] = {
	// Add trigger slot list
	{<Channel>, <Description>, <triggerAction>}
};
#define  triggerSlotListSize (sizeof(triggerSlotList)/sizeof(tsp_triggerSlot_t))

tsp_itemState_t triggerSignalStateList[triggerSignalListSize];
tsp_itemState_t triggerSlotStateList[triggerSlotListSize];

const triggerSystemProtocol_t triggerSystem = {
	.signals = triggerSignals,
	.signalSize = triggerSignalListSize,
	.slots = triggerSlots,
	.slotSize = triggerSlotListSize,
	._signalState = triggerSignalStateList,
	._slotState = triggerSlotStateList
};

***********************************************************************************************************************/
#include "sam.h"
#include "kernel/kernel.h"

#ifdef __cplusplus
extern "C" {
#endif
	
typedef enum {
	tsp_cmd_trigger,
	tsp_cmd_reserved0,
	tsp_cmd_reserved1,
	tsp_cmd_reserved2,
	
	tsp_cmd_signalInformationReport,
	tsp_cmd_slotInformationReport,
	
	tsp_cmd_signalInformationRequest,
	tsp_cmd_slotInformationRequest
}tsp_commands_t;

typedef void (*tsp_triggerAction_t)(uint16_t triggerChannelNumber);

typedef struct{
	uint8_t sendInformationPending :1;
	uint8_t sendTriggerPending :1;
	uint8_t reserved :6;
}tsp_itemState_t;

typedef struct{
	const uint16_t channel;
	const char *description;
}tsp_triggerSignal_t;

typedef struct{
	const uint16_t channel;
	const char *description;
	
	const tsp_triggerAction_t action;
}tsp_triggerSlot_t;

typedef struct{
	const tsp_triggerSignal_t* signals;
	const uint8_t signalSize;
	
	const tsp_triggerSlot_t* slots;
	const uint8_t slotSize;
	
	tsp_itemState_t *_signalState;
	tsp_itemState_t *_slotState;	
}triggerSystemProtocol_t;

void tsp_initialize(const triggerSystemProtocol_t* tsp);
void tsp_mainHandler(const triggerSystemProtocol_t* tsp);
bool tsp_receiveHandler(const triggerSystemProtocol_t* tsp, uint8_t sourceAddress, uint8_t command, const uint8_t *data, uint8_t size);

void tsp_sendTriggerByIndex(const triggerSystemProtocol_t* tsp, const uint8_t *indexList, uint8_t indexListLength);

#ifdef __cplusplus
}
#endif

#endif /* TRIGGER_SYSTEM_PROTOCOL_H_ */