//**********************************************************************************************************************
// FileName : stateReportProtocol.h
// FilePath : protocol/
// Author   : Christian Marty
// Date		: 21.07.2024
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#ifndef STATE_REPORT_PROTOCOL_H_
#define STATE_REPORT_PROTOCOL_H_

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
}srp_commands_t;

typedef enum {
	srp_state_off = 0,
	srp_state_on = 1,
	srp_state_transitioning = 2,
	srp_state_undefined = 3
}srp_state_t;

typedef void (*srp_stateChangeAction_t)(uint16_t stateChannelNumber, srp_state_t state);

typedef struct{
	uint8_t sendInformationPending :1;
	uint8_t sendSignalPending :1;
	uint8_t reserved :6;
	
	srp_state_t state;
	uint32_t timer;
}srp_itemState_t;

typedef struct{
	const uint16_t channel;
	const char *description;
	const uint16_t interval; // in s
}srp_stateSignal_t;

typedef struct{
	const uint16_t channel;
	const char *description;
	const uint16_t timeout; // in s
	
	const srp_stateChangeAction_t action;
}srp_stateSlot_t;

typedef struct{
	const srp_stateSignal_t* signals;
	const uint8_t signalSize;
	
	const srp_stateSlot_t* slots;
	const uint8_t slotSize;
	
	srp_itemState_t *_signalState;
	srp_itemState_t *_slotState;
}stateReportProtocol_t;

void srp_initialize(const stateReportProtocol_t *srp);
void srp_mainHandler(const stateReportProtocol_t *srp);
bool srp_receiveHandler(const stateReportProtocol_t *srp, uint8_t sourceAddress, uint8_t command, const uint8_t *data, uint8_t size);

void srp_setStateByIndex(const stateReportProtocol_t *srp, uint8_t stateChannelIndex, srp_state_t state);
srp_state_t srp_getStateByIndex(const stateReportProtocol_t *srp, uint8_t stateChannelIndex);

#endif /* STATE_REPORT_PROTOCOL_H_ */

#ifdef __cplusplus
}
#endif