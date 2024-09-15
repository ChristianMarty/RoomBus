//**********************************************************************************************************************
// FileName : valueSystemProtocol.cpp
// FilePath : protocol/
// Author   : Christian Marty
// Date		: 02.08.2024
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#include "valueSystemProtocol.h"
#include "utility/pack.h"

#ifdef __cplusplus
extern "C" {
#endif

bool _vsp_parseValueReport(const valueSystemProtocol_t* vsp, uint8_t sourceAddress, const uint8_t *data, uint8_t size);
bool _vsp_parseValueRequest(const valueSystemProtocol_t* vsp, uint8_t sourceAddress, const uint8_t *data, uint8_t size);
bool _vsp_parseValueCommands(const valueSystemProtocol_t* vsp, uint8_t sourceAddress, const uint8_t *data, uint8_t size);
bool _vsp_executeValueCommand(const valueSystemProtocol_t* vsp, uint8_t channelIndex, vsp_valueCommands_t command, vsp_valueData_t value);
bool _vsp_parseValueReportSignalInformationRequest(const valueSystemProtocol_t* vsp, uint8_t sourceAddress, const uint8_t *data, uint8_t size);
bool _vsp_parseValueReportSlotInformationRequest(const valueSystemProtocol_t* vsp, uint8_t sourceAddress, const uint8_t *data, uint8_t size);

bool _vsp_sendValueReportSignalInformation(const vsp_valueSignal_t *stateSignal);
bool _vsp_sendValueReportSlotInformation(const vsp_valueSlot_t *stateSlot);
void _vsp_sendValues(const valueSystemProtocol_t* vsp);



void vsp_initialize(const valueSystemProtocol_t* vsp)
{
	for(int i = 0; i < vsp->slotSize; i++){
		kernel.tickTimer.reset(&(vsp->_slotState[i].timer));
		vsp->_slotState[i].sendInformationPending = false;
	}
	
	for(int i = 0; i < vsp->signalSize; i++){
		kernel.tickTimer.reset(&(vsp->_signalState[i].timer));
		vsp->_signalState[i].sendInformationPending = false;
	}
}

void vsp_mainHandler(const valueSystemProtocol_t* vsp)
{
	// handle slot timeout
	for(int i = 0; i < vsp->slotSize; i++){
		if(kernel.tickTimer.delay1ms(&(vsp->_slotState[i].timer), vsp->slots[i].timeout*1000)){
			// TODO: what to do?
		}
	}
		
	// handle signal interval
	for(int i = 0; i < vsp->signalSize; i++){
		if(kernel.tickTimer.delay1ms(&(vsp->_signalState[i].timer), vsp->signals[i].interval*1000)){
			vsp->_signalState[i].sendSignalPending = true;
		}
	}
		
	// send slot information
	for(uint8_t i = 0;  i < vsp->slotSize; i++) {
		if(vsp->_slotState[i].sendInformationPending == true){
			if(_vsp_sendValueReportSlotInformation(&vsp->slots[i])) {
				vsp->_slotState[i].sendInformationPending = false;
			}
			break;
		}
	}
		
	// send signal information
	for(uint8_t i = 0;  i < vsp->signalSize; i++) {
		if(vsp->_signalState[i].sendInformationPending == true) {
			if(_vsp_sendValueReportSignalInformation(&vsp->signals[i])) {
				vsp->_signalState[i].sendInformationPending = false;
			}
			break;
		}
	}
		
	_vsp_sendValues(vsp);
}

bool vsp_receiveHandler(const valueSystemProtocol_t* vsp, uint8_t sourceAddress, uint8_t command, const uint8_t *data, uint8_t size)
{
	switch(command){
		case vsp_cmd_valueReport: return _vsp_parseValueReport(vsp, sourceAddress, data, size);
		case vsp_cmd_valueRequest: return _vsp_parseValueRequest(vsp, sourceAddress, data, size);
		case vsp_cmd_valueCommand: return _vsp_parseValueCommands(vsp, sourceAddress, data, size);
		
		case vsp_cmd_signalInformationRequest: return _vsp_parseValueReportSignalInformationRequest(vsp, sourceAddress, data, size);
		case vsp_cmd_slotInformationRequest: return _vsp_parseValueReportSlotInformationRequest(vsp, sourceAddress, data, size);
        default: return false;
	}
}


bool vsp_sendValueReportByChannel(const valueSystemProtocol_t* vsp, uint16_t channel, vsp_valueData_t value)
{
	for(uint8_t i = 0;  i < vsp->signalSize; i++) {
		if(vsp->signals[i].channel == channel) {
			
			vsp->_signalState[i].value = value;
			vsp->_signalState[i].sendSignalPending = true;
			kernel.tickTimer.reset(&(vsp->_signalState[i].timer));
			return true;
		}
	}
	return false;
}

bool vsp_sendValueCommandByChannel(const valueSystemProtocol_t* vsp, uint16_t channel, vsp_valueCommands_t command, vsp_valueData_t value)
{
	return false;
}

bool vsp_sendValueCommandByIndex(const valueSystemProtocol_t* vsp, uint8_t index, vsp_valueCommands_t command, vsp_valueData_t value)
{
	bus_message_t msg;
	if(!kernel.bus.getMessageSlot(&msg)) return false; // Abort if TX buffer full
	
	vsp_valueSlot_t slot = vsp->slots[index];
	vsp_itemState_t state  = vsp->_slotState[index];
		
	kernel.bus.writeHeader(&msg, BROADCAST, busProtocol_valueSystemProtocol, vsp_cmd_valueCommand, busPriority_normal);
	kernel.bus.pushWord16(&msg, slot.channel);
	kernel.bus.pushByte(&msg, command);
	kernel.bus.pushWord32(&msg, value.Long);
	
	kernel.bus.send(&msg);
		
	return true;
}

vsp_valueData_t vsp_valueByIndex(const valueSystemProtocol_t* vsp, uint8_t index)
{
	if(vsp->slotSize < index){
		vsp_valueData_t value; 
		value.Long= 0; 
		return value;
	}
	return vsp->_slotState[index].value;
	
}

//**********************************************************************************************************************
// Private
//**********************************************************************************************************************
bool _vsp_parseValueReport(const valueSystemProtocol_t* vsp, uint8_t sourceAddress, const uint8_t *data, uint8_t size)
{
	uint16_t channel = unpack_uint16(&data[0]);
	
	for(uint8_t i = 0;  i < vsp->slotSize; i++) {
		if(vsp->slots[i].channel == channel){
			
			const vsp_valueSlot_t *slot = &vsp->slots[i];
			vsp_itemState_t *slotState = &vsp->_slotState[i];
			
			uint32_t value = unpack_uint32(&data[2]);
			slotState->value.Long = value;
			if(slot->action != nullptr){
				slot->action(channel, slotState->value);
			}
			
			kernel.tickTimer.reset(&(slotState->timer));
			return true;
		}
	}
	
	return false;
}

bool _vsp_parseValueRequest(const valueSystemProtocol_t* vsp, uint8_t sourceAddress, const uint8_t *data, uint8_t size)
{
	// in case size is 0 -> send all
	if(size == 0){
		for(uint8_t j = 0; j < vsp->signalSize; j++){
			vsp->_signalState[j].sendSignalPending = true;
		}
		return true;
	}
		
	// look for requested channels
	bool found = false;
	for(uint8_t i = 0; i < size; i+=2)
	{
		uint16_t channel = (data[i]<<8)|data[i+1];
			
		// TODO: optimize
		for(uint8_t j = 0; j < vsp->signalSize; j++)
		{
			if(channel == vsp->signals[j].channel)
			{
				vsp->_signalState[j].sendSignalPending = true;
				found = true;
				break;
			}
		}
	}
	return found;
}

bool _vsp_parseValueCommands(const valueSystemProtocol_t* vsp, uint8_t sourceAddress, const uint8_t *data, uint8_t size)
{
	uint16_t channel = unpack_uint16(&data[0]);
    vsp_valueCommands_t command = (vsp_valueCommands_t) data[2];
    vsp_valueData_t value={.Long = unpack_uint32(&data[3])}; // technically buffer overflow in some cases but data gets overwritten later

    const vsp_valueSignal_t *signal = nullptr;
    uint8_t channelIndex;
    for (uint8_t i = 0; i < vsp->signalSize; i++) {
        if (vsp->signals[i].channel == channel) {
            signal = &vsp->signals[i];
            channelIndex = i;
            break;
        }
    }
    if (signal == nullptr) { // in case channel was not found
        return false;
    }

    if(_vsp_executeValueCommand(vsp, channelIndex, command, value)) {
        vsp->_signalState[channelIndex].sendSignalPending = true;
        kernel.tickTimer.reset(&(vsp->_signalState[channelIndex].timer));
        if(signal->action != nullptr) {
            signal->action(channel, vsp->_signalState[channelIndex].value);
        }
        return true;
    }

	return false;
}


bool _vsp_executeValueCommand(const valueSystemProtocol_t* vsp, uint8_t channelIndex, vsp_valueCommands_t command, vsp_valueData_t value) {

    vsp_operation_t valueCommand = vsp_decodeOperation(command);

    const vsp_valueSignal_t *signal = &vsp->signals[channelIndex];
    vsp_itemState_t *signalState = &vsp->_signalState[channelIndex];

    if (valueCommand.operation == vsp_operation_setMinimum) {
        signalState->value = signal->min;
        return true;
    }
    if (valueCommand.operation == vsp_operation_setMaximum) {
        signalState->value = signal->max;
        return true;
    }
    vsp_unitType_t signalUnitType = vsp_uomToType(signal->uom);

    if (valueCommand.operation == vsp_operation_invert) {
        switch (signalUnitType) {
            case vsp_unitType_long:
                value.Long = ((signal->max.Long - signal->min.Long) - signalState->value.Long) + signal->min.Long;
                break;
            case vsp_unitType_number:
                value.Number =
                        ((signal->max.Number - signal->min.Number) - signalState->value.Number) + signal->min.Number;
                break;
            case vsp_unitType_undefined:
                return false; // todo: add error
        }
        signalState->value = value;
        return true;
    }

    if (valueCommand.unit == vsp_unitType_undefined) return false;
    if (valueCommand.operation == vsp_operation_undefined) return false;
    if (valueCommand.enforcement == vsp_enforcement_undefined) return false;

    vsp_valueData_t newValue = vsp_convertDataUnitType(valueCommand.unit, signalUnitType, value);

    if (valueCommand.operation == vsp_operation_add){
        if (signalUnitType == vsp_unitType_long) {
            newValue.Long = signalState->value.Long + newValue.Long;
        } else if (signalUnitType == vsp_unitType_number){
            newValue.Number = signalState->value.Number + newValue.Number;
        }
    }else if (valueCommand.operation == vsp_operation_subtract){
        if (signalUnitType == vsp_unitType_long) {
			if(newValue.Long > signalState->value.Long)newValue.Long = 0; // todo: handle "reject" case
            else newValue.Long = signalState->value.Long - newValue.Long;
        } else if (signalUnitType == vsp_unitType_number){
            return false; // this command does not exist
        }
    }

    if (signalUnitType == vsp_unitType_long) {
        if (newValue.Long <= signal->max.Long && newValue.Long >= signal->min.Long) {
            signalState->value = newValue;
            return true;
        }
    } else if (signalUnitType == vsp_unitType_number) {
        if (newValue.Number <= signal->max.Number && newValue.Number >= signal->min.Number) {
            signalState->value = newValue;
            return true;
        }
    }
    // from here if new value is not inside limits
    if(valueCommand.enforcement == vsp_enforcement_reject) {
        return false;
    }

    if(valueCommand.enforcement == vsp_enforcement_clamp) {
        if (signalUnitType == vsp_unitType_long) {
            if (newValue.Long > signal->max.Long) {
                signalState->value = signal->max;
                return true;
            }else if (newValue.Long < signal->min.Long) {
                signalState->value = signal->min;
                return true;
            }
        } else if (signalUnitType == vsp_unitType_number){
            if (newValue.Number > signal->max.Number) {
                signalState->value = signal->max;
                return true;
            }else if (newValue.Number < signal->min.Number) {
                signalState->value = signal->min;
                return true;
            }
        }
        return false; // should be unreachable
    }

    return false;
}

bool _vsp_parseValueReportSignalInformationRequest(const valueSystemProtocol_t* vsp, uint8_t sourceAddress, const uint8_t *data, uint8_t size)
{
	// in case size is 0 -> send all
	if(size == 0){
		for(uint8_t j = 0; j < vsp->signalSize; j++){
			vsp->_signalState[j].sendInformationPending = true;
		}
		return true;
	}
		
	// look for requested channels
	bool found = false;
	for(uint8_t i = 0; i < size; i+=2)
	{
		uint16_t channel = (data[i]<<8)|data[i+1];
			
		// TODO: optimize
		for(uint8_t j = 0; j < vsp->signalSize; j++)
		{
			if(channel == vsp->signals[j].channel)
			{
				vsp->_signalState[j].sendInformationPending = true;
				found = true;
				break;
			}
		}
	}
	return found;
}

bool _vsp_parseValueReportSlotInformationRequest(const valueSystemProtocol_t* vsp, uint8_t sourceAddress, const uint8_t *data, uint8_t size)
{
	// in case size is 0 -> send all
	if(size == 0){
		for(uint8_t j = 0; j < vsp->slotSize; j++){
			vsp->_slotState[j].sendInformationPending = true;
		}
		return true;
	}
		
	// look for requested channels
	bool found = false;
	for(uint8_t i = 0; i < size; i+=2)
	{
		uint16_t channel = (data[i]<<8)|data[i+1];
			
		// TODO: optimize
		for(uint8_t j = 0; j < vsp->slotSize; j++)
		{
			if(channel == vsp->slots[j].channel)
			{
				vsp->_slotState[j].sendInformationPending = true;
				found = true;
				break;
			}
		}
	}
	return found;
}

bool _vsp_sendValueReportSignalInformation(const vsp_valueSignal_t *stateSignal)
{
	bus_message_t msg;
	if(!kernel.bus.getMessageSlot(&msg)) return false; // Abort if TX buffer full
		
	kernel.bus.writeHeader(&msg, BROADCAST, busProtocol_valueSystemProtocol, vsp_cmd_signalInformationReport, busPriority_low);
	kernel.bus.pushWord16(&msg, stateSignal->channel);
	kernel.bus.pushWord16(&msg, stateSignal->interval);
	
	uint16_t uom = stateSignal->uom;
	if(stateSignal->readOnly) uom |= 0x8000;
	kernel.bus.pushWord16(&msg, uom);
	
	kernel.bus.pushWord32(&msg, stateSignal->min.Long);
	kernel.bus.pushWord32(&msg, stateSignal->max.Long);
	
	kernel.bus.pushString(&msg, &stateSignal->description[0]);
	kernel.bus.send(&msg);
		
	return true;
}

bool _vsp_sendValueReportSlotInformation(const vsp_valueSlot_t *stateSlot)
{
	bus_message_t msg;
	if(!kernel.bus.getMessageSlot(&msg)) return false; // Abort if TX buffer full
		
	kernel.bus.writeHeader(&msg, BROADCAST, busProtocol_valueSystemProtocol, vsp_cmd_slotInformationReport, busPriority_low);
	kernel.bus.pushWord16(&msg, stateSlot->channel);
	kernel.bus.pushWord16(&msg, stateSlot->timeout);
	kernel.bus.pushString(&msg, &stateSlot->description[0]);
	kernel.bus.send(&msg);
		
	return true;
}

void _vsp_sendValues(const valueSystemProtocol_t *vsp)
{
	for(uint8_t i = 0;  i < vsp->signalSize; i++)
	{
		if(vsp->_signalState[i].sendSignalPending == false) continue;
		
		bus_message_t msg;
		if(!kernel.bus.getMessageSlot(&msg)){
			return; // Abort if TX buffer full
		}
		
		kernel.bus.writeHeader(&msg, BROADCAST, busProtocol_valueSystemProtocol, vsp_cmd_valueReport, busPriority_low);
		kernel.bus.pushWord16(&msg, vsp->signals[i].channel);
		kernel.bus.pushWord32(&msg, vsp->_signalState[i].value.Long);
		kernel.bus.send(&msg);
		
		vsp->_signalState[i].sendSignalPending = false;
	}
}

vsp_unitType_t vsp_uomToType(vsp_uom_t uom)
{
	if(uom == vsp_uom_long) return vsp_unitType_long;
	else if(uom == vsp_uom_number) return vsp_unitType_number;

	// todo: add rest of uom
	return vsp_unitType_long;
}

vsp_valueData_t vsp_convertDataUnitType(vsp_unitType_t inputType, vsp_unitType_t outputType, vsp_valueData_t input)
{
    if(inputType == outputType) return input;

    if(outputType == vsp_unitType_long){
        if(inputType == vsp_unitType_number) {
            vsp_valueData_t output;
            output.Long = (uint32_t) input.Number;
            return output;
        }
    }else if(outputType == vsp_unitType_number) {
        if (inputType == vsp_unitType_long) {
            vsp_valueData_t output;
            output.Number = (float) input.Long;
            return output;
        }
    }

    return input; // in case of unknown conversion, return the original data
}

vsp_operation_t vsp_decodeOperation(uint8_t operation)
{
    vsp_operation_t output = {
        .unit = vsp_unitType_undefined,
        .enforcement = vsp_enforcement_undefined,
        .operation = vsp_operation_undefined
    };

    switch (operation) {
        case vsp_vCmd_setMinimum:
            output.operation = vsp_operation_setMinimum;
            break;

        case vsp_vCmd_setMaximum:
            output.operation = vsp_operation_setMaximum;
            break;

        case vsp_vCmd_invert:
            output.operation = vsp_operation_invert;
            break;

        case vsp_vCmd_setLongValueReject:
            output.operation = vsp_operation_set;
            output.enforcement = vsp_enforcement_reject;
            output.unit = vsp_unitType_long;
            break;

        case vsp_vCmd_setLongValueClamp:
            output.operation = vsp_operation_set;
            output.enforcement = vsp_enforcement_clamp;
            output.unit = vsp_unitType_long;
            break;

        case vsp_vCmd_addLongReject:
            output.operation = vsp_operation_add;
            output.enforcement = vsp_enforcement_reject;
            output.unit = vsp_unitType_long;
            break;

        case vsp_vCmd_addLongClamp:
            output.operation = vsp_operation_add;
            output.enforcement = vsp_enforcement_clamp;
            output.unit = vsp_unitType_long;
            break;

        case vsp_vCmd_subtractLongReject:
            output.operation = vsp_operation_subtract;
            output.enforcement = vsp_enforcement_reject;
            output.unit = vsp_unitType_long;
            break;

        case vsp_vCmd_subtractLongClamp:
            output.operation = vsp_operation_subtract;
            output.enforcement = vsp_enforcement_clamp;
            output.unit = vsp_unitType_long;
            break;


        case vsp_vCmd_setNumberValueReject:
            output.operation = vsp_operation_set;
            output.enforcement = vsp_enforcement_reject;
            output.unit = vsp_unitType_number;
            break;

        case vsp_vCmd_setNumberValueClamp:
            output.operation = vsp_operation_set;
            output.enforcement = vsp_enforcement_clamp;
            output.unit = vsp_unitType_number;
            break;

        case vsp_vCmd_addNumberReject:
            output.operation = vsp_operation_add;
            output.enforcement = vsp_enforcement_reject;
            output.unit = vsp_unitType_number;
            break;

        case vsp_vCmd_addNumberClamp:
            output.operation = vsp_operation_add;
            output.enforcement = vsp_enforcement_clamp;
            output.unit = vsp_unitType_number;
            break;
    }

    return output;
}

#ifdef __cplusplus
}
#endif