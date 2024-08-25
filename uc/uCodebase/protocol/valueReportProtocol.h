//**********************************************************************************************************************
// FileName : valueReportProtocol.h
// FilePath : protocol/
// Author   : Christian Marty
// Date		: 02.08.2024
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#ifndef VALUE_REPORT_PROTOCOL_H_
#define VALUE_REPORT_PROTOCOL_H_

/* EXAMPLE *************************************************************************************************************


***********************************************************************************************************************/
#include "common/kernel.h"

#ifdef __cplusplus
extern "C" {
#endif
	
typedef enum {
	vrp_cmd_valueReport,
	vrp_cmd_valueRequest,
	vrp_cmd_valueCommand,
	vrp_cmd_reserved1,
	
	vrp_cmd_signalInformationReport,
	vrp_cmd_slotInformationReport,
	
	vrp_cmd_signalInformationRequest,
	vrp_cmd_slotInformationRequest
}vrp_commands_t;

typedef enum {
	vrp_vCmd_setMinimum = 0x00,
	vrp_vCmd_setMaximum,
	vrp_vCmd_invert,


    vrp_vCmd_setLongValueReject = 0x10,
	vrp_vCmd_setLongValueClamp,
	
	vrp_vCmd_addLongReject = 0x20,
	vrp_vCmd_addLongClamp,
	
	vrp_vCmd_subtractLongReject = 0x30,
	vrp_vCmd_subtractLongClamp,


    vrp_vCmd_setNumberValueReject = 0x40,
    vrp_vCmd_setNumberValueClamp,

	vrp_vCmd_addNumberReject = 0x50,
	vrp_vCmd_addNumberClamp

}vrp_valueCommands_t;

typedef enum {
    vrp_unitType_undefined,
	vrp_unitType_long,
	vrp_unitType_number
}vrp_unitType_t;

typedef enum {
    vrp_enforcement_undefined,
    vrp_enforcement_reject,
    vrp_enforcement_clamp
}vrp_enforcementType_t;

typedef enum {
    vrp_operation_undefined,
    vrp_operation_setMinimum,
    vrp_operation_setMaximum,
    vrp_operation_invert,
    vrp_operation_set,
    vrp_operation_add,
    vrp_operation_subtract
}vrp_operationType_t;

typedef struct {
    vrp_unitType_t unit;
    vrp_enforcementType_t enforcement;
    vrp_operationType_t operation;
}vrp_operation_t;

typedef enum {
	uom_long,
	uom_number,

	uom_absoluteTime = 0x10,
	uom_relativeTime,
	uom_temperature
}vrp_uom_t;

typedef union {
	uint32_t Long;
	float Number;
}vrp_valueData_t;

typedef struct{
	uint8_t sendInformationPending :1;
	uint8_t sendSignalPending :1;
	uint8_t reserved :5;
	
	vrp_valueData_t value;
	
	uint32_t timer;
}vrp_itemState_t;

typedef void (*vrp_valueChangeAction_t)(uint16_t valueChannelNumber, vrp_valueData_t value);

typedef struct{
	const uint16_t channel;
	const char *description;
	const uint16_t interval; // in s
	
	bool readOnly;
	vrp_valueData_t min;
	vrp_valueData_t max;
	
	vrp_uom_t uom;
	
	const vrp_valueChangeAction_t action;
}vrp_valueSignal_t;

typedef struct{
	const uint16_t channel;
	const char *description;
	const uint16_t timeout; // in s
	
	const vrp_valueChangeAction_t action;
}vrp_valueSlot_t;

typedef struct{
	const vrp_valueSignal_t* signals;
	const uint8_t signalSize;
	
	const vrp_valueSlot_t* slots;
	const uint8_t slotSize;
	
	vrp_itemState_t *_signalState;
	vrp_itemState_t *_slotState;
}valueReportProtocol_t;

void vrp_initialize(const valueReportProtocol_t* vrp);
void vrp_mainHandler(const valueReportProtocol_t* vrp);
bool vrp_receiveHandler(const valueReportProtocol_t* vrp, uint8_t sourceAddress, uint8_t command, const uint8_t *data, uint8_t size);

bool vrp_sendValueReport(const valueReportProtocol_t* vrp, uint16_t channel, vrp_valueData_t value);
bool vrp_sendValueCommand(const valueReportProtocol_t* vrp, uint16_t channel, vrp_valueData_t value);

vrp_unitType_t vrp_uomToType(vrp_uom_t uom);
vrp_valueData_t vrp_convertDataUnitType(vrp_unitType_t inputType, vrp_unitType_t outputType, vrp_valueData_t input);
vrp_operation_t vrp_decodeOperation(uint8_t operation);

#endif /* VALUE_REPORT_PROTOCOL_H_ */

#ifdef __cplusplus
}
#endif