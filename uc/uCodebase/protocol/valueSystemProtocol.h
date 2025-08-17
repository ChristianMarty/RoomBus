//**********************************************************************************************************************
// FileName : valueSystemProtocol.h
// FilePath : protocol/
// Author   : Christian Marty
// Date		: 08.09.2024
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#ifndef VALUE_SYSTEM_PROTOCOL_H_
#define VALUE_SYSTEM_PROTOCOL_H_

/* EXAMPLE *************************************************************************************************************


***********************************************************************************************************************/
#include "common/typedef.h"
#include "common/kernel.h"

#ifdef __cplusplus
extern "C" {
#endif
	
typedef enum {
	vsp_cmd_valueReport,
	vsp_cmd_valueRequest,
	vsp_cmd_valueCommand,
	vsp_cmd_reserved1,
	
	vsp_cmd_signalInformationReport,
	vsp_cmd_slotInformationReport,
	
	vsp_cmd_signalInformationRequest,
	vsp_cmd_slotInformationRequest
}vsp_commands_t;

typedef enum {
	vsp_vCmd_setMinimum = 0x00,
	vsp_vCmd_setMaximum,
	vsp_vCmd_invert,


    vsp_vCmd_setLongValueReject = 0x10,
	vsp_vCmd_setLongValueClamp,
	
	vsp_vCmd_addLongReject = 0x20,
	vsp_vCmd_addLongClamp,
	
	vsp_vCmd_subtractLongReject = 0x30,
	vsp_vCmd_subtractLongClamp,


    vsp_vCmd_setNumberValueReject = 0x40,
    vsp_vCmd_setNumberValueClamp,

	vsp_vCmd_addNumberReject = 0x50,
	vsp_vCmd_addNumberClamp

}vsp_valueCommands_t;

typedef enum {
    vsp_unitType_undefined,
	vsp_unitType_long,
	vsp_unitType_number
}vsp_unitType_t;

typedef enum {
    vsp_enforcement_undefined,
    vsp_enforcement_reject,
    vsp_enforcement_clamp
}vsp_enforcementType_t;

typedef enum {
    vsp_operation_undefined,
    vsp_operation_setMinimum,
    vsp_operation_setMaximum,
    vsp_operation_invert,
    vsp_operation_set,
    vsp_operation_add,
    vsp_operation_subtract
}vsp_operationType_t;

typedef struct {
    vsp_unitType_t unit;
    vsp_enforcementType_t enforcement;
    vsp_operationType_t operation;
}vsp_operation_t;

typedef enum {
	vsp_uom_long,
	vsp_uom_number,

	vsp_uom_absoluteTime = 0x10,
	vsp_uom_relativeTime,
	vsp_uom_temperature
}vsp_uom_t;

typedef union {
	uint32_t Long;
	float Number;
}vsp_valueData_t;

typedef struct{
	uint8_t sendInformationPending :1;
	uint8_t sendSignalPending :1;
	uint8_t reserved :5;
	
	vsp_valueData_t value;
	
	uint32_t timer;
}vsp_itemState_t;

typedef void (*vsp_valueChangeAction_t)(uint16_t valueChannelNumber, vsp_valueData_t value);

typedef struct{
	const uint16_t channel;
	const char *description;
	const uint16_t interval; // in s
	
	bool readOnly;
	vsp_valueData_t min;
	vsp_valueData_t max;
	
	vsp_uom_t uom;
	
	const vsp_valueChangeAction_t action;
}vsp_valueSignal_t;

typedef struct{
	uint16_t channel;
	const char *description;
	uint16_t timeout; // in s
	
	const vsp_valueChangeAction_t action;
}vsp_valueSlot_t;

typedef struct{
	const vsp_valueSignal_t* signals;
	const uint8_t signalSize;
	
	const vsp_valueSlot_t* slots;
	const uint8_t slotSize;
	
	vsp_itemState_t *_signalState;
	vsp_itemState_t *_slotState;
}valueSystemProtocol_t;

void vsp_initialize(const valueSystemProtocol_t* vsp);
void vsp_mainHandler(const valueSystemProtocol_t* vsp);
bool vsp_receiveHandler(const valueSystemProtocol_t* vsp, const bus_rxMessage_t *message);

bool vsp_sendValueReportByChannel(const valueSystemProtocol_t* vsp, uint16_t channel, vsp_valueData_t value);

bool vsp_sendValueCommandByChannel(const valueSystemProtocol_t* vsp, uint16_t channel, vsp_valueCommands_t command, vsp_valueData_t value);
bool vsp_sendValueCommandByIndex(const valueSystemProtocol_t* vsp, uint8_t index, vsp_valueCommands_t command, vsp_valueData_t value);

vsp_valueData_t vsp_valueByIndex(const valueSystemProtocol_t* vsp, uint8_t index);

vsp_unitType_t vsp_uomToType(vsp_uom_t uom);
vsp_valueData_t vsp_convertDataUnitType(vsp_unitType_t inputType, vsp_unitType_t outputType, vsp_valueData_t input);
vsp_operation_t vsp_decodeOperation(uint8_t operation);

#endif /* VALUE_SYSTEM_PROTOCOL_H_ */

#ifdef __cplusplus
}
#endif