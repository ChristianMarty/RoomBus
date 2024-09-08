//**********************************************************************************************************************
// FileName : stateSystemProtocol.h
// FilePath : protocol/
// Author   : Christian Marty
// Date		: 08.09.2024
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#ifndef STATE_SYSTEM_PROTOCOL_H_
#define STATE_SYSTEM_PROTOCOL_H_

/* EXAMPLE *************************************************************************************************************


***********************************************************************************************************************/
#include "common/typedef.h"
#include "common/kernel.h"

#ifdef __cplusplus
extern "C" {
#endif
	
typedef enum {
	srp_cmd_state,
	srp_cmd_stateRequest,
	srp_cmd_reserved0,
	srp_cmd_reserved1,
	
	srp_cmd_signalInformationReport,
	srp_cmd_slotInformationReport,
	
	srp_cmd_signalInformationRequest,
	srp_cmd_slotInformationRequest
}ssp_commands_t;

typedef enum {
	srp_state_off = 0,
	srp_state_on = 1,
	ssp_state_transitioning = 2,
	srp_state_undefined = 3
}ssp_state_t;

typedef void (*ssp_stateChangeAction_t)(uint16_t stateChannelNumber, ssp_state_t state);

typedef struct{
	uint8_t sendInformationPending :1;
	uint8_t sendSignalPending :1;
	uint8_t reserved :6;
	
	ssp_state_t state;
	uint32_t timer;
}ssp_itemState_t;

typedef struct{
	const uint16_t channel;
	const char *description;
	const uint16_t interval; // in s
}ssp_stateSignal_t;

typedef struct{
	const uint16_t channel;
	const char *description;
	const uint16_t timeout; // in s
	
	const ssp_stateChangeAction_t action;
}ssp_stateSlot_t;

typedef struct{
	const ssp_stateSignal_t* signals;
	const uint8_t signalSize;
	
	const ssp_stateSlot_t* slots;
	const uint8_t slotSize;
	
	ssp_itemState_t *_signalState;
	ssp_itemState_t *_slotState;
}stateSystemProtocol_t;

void ssp_initialize(const stateSystemProtocol_t *ssp);
void ssp_mainHandler(const stateSystemProtocol_t *ssp);
bool ssp_receiveHandler(const stateSystemProtocol_t *ssp, uint8_t sourceAddress, uint8_t command, const uint8_t *data, uint8_t size);

void ssp_setStateByIndex(const stateSystemProtocol_t *ssp, uint8_t stateChannelIndex, ssp_state_t state);
ssp_state_t ssp_getStateByIndex(const stateSystemProtocol_t *ssp, uint8_t stateChannelIndex);

#endif /* STATE_SYSTEM_PROTOCOL_H_ */

#ifdef __cplusplus
}
#endif