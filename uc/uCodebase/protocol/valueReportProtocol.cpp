//**********************************************************************************************************************
// FileName : valueReportProtocol.cpp
// FilePath : protocol/
// Author   : Christian Marty
// Date		: 02.08.2024
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#include "valueReportProtocol.h"
#include "utility/pack.h"

#ifdef __cplusplus
extern "C" {
#endif

bool parseValueReport(const valueReportProtocol_t* vrp, uint8_t sourceAddress, const uint8_t *data, uint8_t size);
bool parseValueRequest(const valueReportProtocol_t* vrp, uint8_t sourceAddress, const uint8_t *data, uint8_t size);
bool parseValueCommands(const valueReportProtocol_t* vrp, uint8_t sourceAddress, const uint8_t *data, uint8_t size);
bool executeValueCommand(const valueReportProtocol_t* vrp, uint8_t channelIndex, vrp_valueCommands_t command, vrp_valueData_t value);
bool parseValueReportSignalInformationRequest(const valueReportProtocol_t* vrp, uint8_t sourceAddress, const uint8_t *data, uint8_t size);
bool parseValueReportSlotInformationRequest(const valueReportProtocol_t* vrp, uint8_t sourceAddress, const uint8_t *data, uint8_t size);

bool sendValueReportSignalInformation(const vrp_valueSignal_t *stateSignal);
bool sendValueReportSlotInformation(const vrp_valueSlot_t *stateSlot);
void sendValues(const valueReportProtocol_t* vrp);



void vrp_initialize(const valueReportProtocol_t* vrp)
{
	for(int i = 0; i < vrp->slotSize; i++){
		kernel.tickTimer.reset(&(vrp->_slotState[i].timer));
		vrp->_slotState[i].sendInformationPending = false;
	}
	
	for(int i = 0; i < vrp->signalSize; i++){
		kernel.tickTimer.reset(&(vrp->_signalState[i].timer));
		vrp->_signalState[i].sendInformationPending = false;
	}
}

void vrp_mainHandler(const valueReportProtocol_t* vrp)
{
	// handle slot timeout
	for(int i = 0; i < vrp->signalSize; i++){
		if(kernel.tickTimer.delay1ms(&(vrp->_signalState[i].timer), vrp->slots[i].timeout*1000)){
			vrp->_signalState[i].sendSignalPending = true;
		}
	}
		
	// handle signal interval
	for(int i = 0; i < vrp->signalSize; i++){
		if(kernel.tickTimer.delay1ms(&(vrp->_signalState[i].timer), vrp->signals[i].interval*1000)){
			vrp->_signalState[i].sendSignalPending = true;
		}
	}
		
	// send slot information
	for(uint8_t i = 0;  i < vrp->slotSize; i++) {
			
		if(vrp->_slotState[i].sendInformationPending == true){
			if(sendValueReportSlotInformation(&vrp->slots[i])) {
				vrp->_slotState[i].sendInformationPending = false;
			}
			break;
		}
	}
		
	// send signal information
	for(uint8_t i = 0;  i < vrp->signalSize; i++) {
		if(vrp->_signalState[i].sendInformationPending == true) {
			if(sendValueReportSignalInformation(&vrp->signals[i])) {
				vrp->_signalState[i].sendInformationPending = false;
			}
			break;
		}
	}
		
	sendValues(vrp);
}

bool vrp_receiveHandler(const valueReportProtocol_t* vrp, uint8_t sourceAddress, uint8_t command, const uint8_t *data, uint8_t size)
{
	switch(command){
		case vrp_cmd_valueReport: return parseValueReport(vrp, sourceAddress, data, size);
		case vrp_cmd_valueRequest: return parseValueRequest(vrp, sourceAddress, data, size);
		case vrp_cmd_valueCommand: return parseValueCommands(vrp, sourceAddress, data, size);
		
		case vrp_cmd_signalInformationRequest: return parseValueReportSignalInformationRequest(vrp, sourceAddress, data, size);
		case vrp_cmd_slotInformationRequest: return parseValueReportSlotInformationRequest(vrp, sourceAddress, data, size);
        default: return false;
	}
}


bool vrp_sendValueReport(const valueReportProtocol_t* vrp, uint16_t channel, vrp_valueData_t value)
{
	for(uint8_t i = 0;  i < vrp->signalSize; i++) {
		if(vrp->signals[i].channel == channel) {
			
			vrp->_signalState[i].value = value;
			vrp->_signalState[i].sendSignalPending = true;
			kernel.tickTimer.reset(&(vrp->_signalState[i].timer));
			return true;
		}
	}
	return false;
}

bool vrp_sendValueCommandByChannel(const valueReportProtocol_t* vrp, uint16_t channel, vrp_valueCommands_t command, vrp_valueData_t value)
{
	return false;
}

bool vrp_sendValueCommandByIndex(const valueReportProtocol_t* vrp, uint8_t index, vrp_valueCommands_t command, vrp_valueData_t value)
{
	bus_message_t msg;
	if(!kernel.bus.getMessageSlot(&msg)) return false; // Abort if TX buffer full
	
	vrp_valueSlot_t slot = vrp->slots[index];
	vrp_itemState_t state  = vrp->_slotState[index];
		
	kernel.bus.writeHeader(&msg, BROADCAST, busProtocol_valueReportProtocol, vrp_cmd_valueCommand, busPriority_normal);
	kernel.bus.pushWord16(&msg, slot.channel);
	kernel.bus.pushByte(&msg, command);
	kernel.bus.pushWord32(&msg, value.Long);
	
	kernel.bus.send(&msg);
		
	return true;
}

vrp_valueData_t vrp_valueByIndex(const valueReportProtocol_t* vrp, uint8_t index)
{
	if(vrp->slotSize < index){
		vrp_valueData_t value; 
		value.Long= 0; 
		return value;
	}
	return vrp->_slotState[index].value;
	
}
//**********************************************************************************************************************
// Private
//**********************************************************************************************************************

bool parseValueReport(const valueReportProtocol_t* vrp, uint8_t sourceAddress, const uint8_t *data, uint8_t size)
{
	uint16_t channel = unpack_uint16(&data[0]);
	
	for(uint8_t i = 0;  i < vrp->slotSize; i++) {
		if(vrp->slots[i].channel == channel){
			
			const vrp_valueSlot_t *slot = &vrp->slots[i];
			vrp_itemState_t *slotState = &vrp->_slotState[i];
			
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

bool parseValueRequest(const valueReportProtocol_t* vrp, uint8_t sourceAddress, const uint8_t *data, uint8_t size)
{
	// in case size is 0 -> send all
	if(size == 0){
		for(uint8_t j = 0; j < vrp->signalSize; j++){
			vrp->_signalState[j].sendSignalPending = true;
		}
		return true;
	}
		
	// look for requested channels
	bool found = false;
	for(uint8_t i = 0; i < size; i+=2)
	{
		uint16_t channel = (data[i]<<8)|data[i+1];
			
		// TODO: optimize
		for(uint8_t j = 0; j < vrp->signalSize; j++)
		{
			if(channel == vrp->signals[j].channel)
			{
				vrp->_signalState[j].sendSignalPending = true;
				found = true;
				break;
			}
		}
	}
	return found;
}

bool parseValueCommands(const valueReportProtocol_t* vrp, uint8_t sourceAddress, const uint8_t *data, uint8_t size)
{
	uint16_t channel = unpack_uint16(&data[0]);
    vrp_valueCommands_t command = (vrp_valueCommands_t) data[2];
    vrp_valueData_t value={.Long = unpack_uint32(&data[3])}; // technically buffer overflow in some cases but data gets overwritten later

    const vrp_valueSignal_t *signal = nullptr;
    uint8_t channelIndex;
    for (uint8_t i = 0; i < vrp->signalSize; i++) {
        if (vrp->signals[i].channel == channel) {
            signal = &vrp->signals[i];
            channelIndex = i;
            break;
        }
    }
    if (signal == nullptr) { // in case channel was not found
        return false;
    }

    if(executeValueCommand(vrp, channelIndex, command, value)) {
        vrp->_signalState[channelIndex].sendSignalPending = true;
        kernel.tickTimer.reset(&(vrp->_signalState[channelIndex].timer));
        if(signal->action != nullptr) {
            signal->action(channel, vrp->_signalState[channelIndex].value);
        }
        return true;
    }

	return false;
}


bool executeValueCommand(const valueReportProtocol_t* vrp, uint8_t channelIndex, vrp_valueCommands_t command, vrp_valueData_t value) {

    vrp_operation_t valueCommand = vrp_decodeOperation(command);

    const vrp_valueSignal_t *signal = &vrp->signals[channelIndex];
    vrp_itemState_t *signalState = &vrp->_signalState[channelIndex];

    if (valueCommand.operation == vrp_operation_setMinimum) {
        signalState->value = signal->min;
        return true;
    }
    if (valueCommand.operation == vrp_operation_setMaximum) {
        signalState->value = signal->max;
        return true;
    }
    vrp_unitType_t signalUnitType = vrp_uomToType(signal->uom);

    if (valueCommand.operation == vrp_operation_invert) {
        switch (signalUnitType) {
            case vrp_unitType_long:
                value.Long = ((signal->max.Long - signal->min.Long) - signalState->value.Long) + signal->min.Long;
                break;
            case vrp_unitType_number:
                value.Number =
                        ((signal->max.Number - signal->min.Number) - signalState->value.Number) + signal->min.Number;
                break;
            case vrp_unitType_undefined:
                return false; // todo: add error
        }
        signalState->value = value;
        return true;
    }

    if (valueCommand.unit == vrp_unitType_undefined) return false;
    if (valueCommand.operation == vrp_operation_undefined) return false;
    if (valueCommand.enforcement == vrp_enforcement_undefined) return false;

    vrp_valueData_t newValue = vrp_convertDataUnitType(valueCommand.unit, signalUnitType, value);

    if (valueCommand.operation == vrp_operation_add){
        if (signalUnitType == vrp_unitType_long) {
            newValue.Long = signalState->value.Long + newValue.Long;
        } else if (signalUnitType == vrp_unitType_number){
            newValue.Number = signalState->value.Number + newValue.Number;
        }
    }else if (valueCommand.operation == vrp_operation_subtract){
        if (signalUnitType == vrp_unitType_long) {
            newValue.Long = signalState->value.Long - newValue.Long;
        } else if (signalUnitType == vrp_unitType_number){
            return false; // this command does not exist
        }
    }

    if (signalUnitType == vrp_unitType_long) {
        if (newValue.Long <= signal->max.Long && newValue.Long >= signal->min.Long) {
            signalState->value = newValue;
            return true;
        }
    } else if (signalUnitType == vrp_unitType_number) {
        if (newValue.Number <= signal->max.Number && newValue.Number >= signal->min.Number) {
            signalState->value = newValue;
            return true;
        }
    }
    // from here if new value is not inside limits
    if(valueCommand.enforcement == vrp_enforcement_reject) {
        return false;
    }

    if(valueCommand.enforcement == vrp_enforcement_clamp) {
        if (signalUnitType == vrp_unitType_long) {
            if (newValue.Long > signal->max.Long) {
                signalState->value = signal->max;
                return true;
            }else if (newValue.Long < signal->min.Long) {
                signalState->value = signal->min;
                return true;
            }
        } else if (signalUnitType == vrp_unitType_number){
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

bool parseValueReportSignalInformationRequest(const valueReportProtocol_t* vrp, uint8_t sourceAddress, const uint8_t *data, uint8_t size)
{
	// in case size is 0 -> send all
	if(size == 0){
		for(uint8_t j = 0; j < vrp->signalSize; j++){
			vrp->_signalState[j].sendInformationPending = true;
		}
		return true;
	}
		
	// look for requested channels
	bool found = false;
	for(uint8_t i = 0; i < size; i+=2)
	{
		uint16_t channel = (data[i]<<8)|data[i+1];
			
		// TODO: optimize
		for(uint8_t j = 0; j < vrp->signalSize; j++)
		{
			if(channel == vrp->signals[j].channel)
			{
				vrp->_signalState[j].sendInformationPending = true;
				found = true;
				break;
			}
		}
	}
	return found;
}

bool parseValueReportSlotInformationRequest(const valueReportProtocol_t* vrp, uint8_t sourceAddress, const uint8_t *data, uint8_t size)
{
	// in case size is 0 -> send all
	if(size == 0){
		for(uint8_t j = 0; j < vrp->slotSize; j++){
			vrp->_slotState[j].sendInformationPending = true;
		}
		return true;
	}
		
	// look for requested channels
	bool found = false;
	for(uint8_t i = 0; i < size; i+=2)
	{
		uint16_t channel = (data[i]<<8)|data[i+1];
			
		// TODO: optimize
		for(uint8_t j = 0; j < vrp->slotSize; j++)
		{
			if(channel == vrp->slots[j].channel)
			{
				vrp->_slotState[j].sendInformationPending = true;
				found = true;
				break;
			}
		}
	}
	return found;
}

bool sendValueReportSignalInformation(const vrp_valueSignal_t *stateSignal)
{
	bus_message_t msg;
	if(!kernel.bus.getMessageSlot(&msg)) return false; // Abort if TX buffer full
		
	kernel.bus.writeHeader(&msg, BROADCAST, busProtocol_valueReportProtocol, vrp_cmd_signalInformationReport, busPriority_low);
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

bool sendValueReportSlotInformation(const vrp_valueSlot_t *stateSlot)
{
	bus_message_t msg;
	if(!kernel.bus.getMessageSlot(&msg)) return false; // Abort if TX buffer full
		
	kernel.bus.writeHeader(&msg, BROADCAST, busProtocol_valueReportProtocol, vrp_cmd_slotInformationReport, busPriority_low);
	kernel.bus.pushWord16(&msg, stateSlot->channel);
	kernel.bus.pushWord16(&msg, stateSlot->timeout);
	kernel.bus.pushString(&msg, &stateSlot->description[0]);
	kernel.bus.send(&msg);
		
	return true;
}

void sendValues(const valueReportProtocol_t *vrp)
{
	for(uint8_t i = 0;  i < vrp->signalSize; i++)
	{
		if(vrp->_signalState[i].sendSignalPending == false) continue;
		
		bus_message_t msg;
		if(!kernel.bus.getMessageSlot(&msg)){
			return; // Abort if TX buffer full
		}
		
		kernel.bus.writeHeader(&msg, BROADCAST, busProtocol_valueReportProtocol, vrp_cmd_valueReport, busPriority_low);
		kernel.bus.pushWord16(&msg, vrp->signals[i].channel);
		kernel.bus.pushWord32(&msg, vrp->_signalState[i].value.Long);
		kernel.bus.send(&msg);
		
		vrp->_signalState[i].sendSignalPending = false;
	}
}

vrp_unitType_t vrp_uomToType(vrp_uom_t uom)
{
	if(uom == uom_long) return vrp_unitType_long;
	else if(uom == uom_number) return vrp_unitType_number;

	// todo: add rest of uom
	return vrp_unitType_long;
}

vrp_valueData_t vrp_convertDataUnitType(vrp_unitType_t inputType, vrp_unitType_t outputType, vrp_valueData_t input)
{
    if(inputType == outputType) return input;

    if(outputType == vrp_unitType_long){
        if(inputType == vrp_unitType_number) {
            vrp_valueData_t output;
            output.Long = (uint32_t) input.Number;
            return output;
        }
    }else if(outputType == vrp_unitType_number) {
        if (inputType == vrp_unitType_long) {
            vrp_valueData_t output;
            output.Number = (float) input.Long;
            return output;
        }
    }

    return input; // in case of unknown conversion, return the original data
}

vrp_operation_t vrp_decodeOperation(uint8_t operation)
{
    vrp_operation_t output = {
        .unit = vrp_unitType_undefined,
        .enforcement = vrp_enforcement_undefined,
        .operation = vrp_operation_undefined
    };

    switch (operation) {
        case vrp_vCmd_setMinimum:
            output.operation = vrp_operation_setMinimum;
            break;

        case vrp_vCmd_setMaximum:
            output.operation = vrp_operation_setMaximum;
            break;

        case vrp_vCmd_invert:
            output.operation = vrp_operation_invert;
            break;

        case vrp_vCmd_setLongValueReject:
            output.operation = vrp_operation_set;
            output.enforcement = vrp_enforcement_reject;
            output.unit = vrp_unitType_long;
            break;

        case vrp_vCmd_setLongValueClamp:
            output.operation = vrp_operation_set;
            output.enforcement = vrp_enforcement_clamp;
            output.unit = vrp_unitType_long;
            break;

        case vrp_vCmd_addLongReject:
            output.operation = vrp_operation_add;
            output.enforcement = vrp_enforcement_reject;
            output.unit = vrp_unitType_long;
            break;

        case vrp_vCmd_addLongClamp:
            output.operation = vrp_operation_add;
            output.enforcement = vrp_enforcement_clamp;
            output.unit = vrp_unitType_long;
            break;

        case vrp_vCmd_subtractLongReject:
            output.operation = vrp_operation_subtract;
            output.enforcement = vrp_enforcement_reject;
            output.unit = vrp_unitType_long;
            break;

        case vrp_vCmd_subtractLongClamp:
            output.operation = vrp_operation_subtract;
            output.enforcement = vrp_enforcement_clamp;
            output.unit = vrp_unitType_long;
            break;


        case vrp_vCmd_setNumberValueReject:
            output.operation = vrp_operation_set;
            output.enforcement = vrp_enforcement_reject;
            output.unit = vrp_unitType_number;
            break;

        case vrp_vCmd_setNumberValueClamp:
            output.operation = vrp_operation_set;
            output.enforcement = vrp_enforcement_clamp;
            output.unit = vrp_unitType_number;
            break;

        case vrp_vCmd_addNumberReject:
            output.operation = vrp_operation_add;
            output.enforcement = vrp_enforcement_reject;
            output.unit = vrp_unitType_number;
            break;

        case vrp_vCmd_addNumberClamp:
            output.operation = vrp_operation_add;
            output.enforcement = vrp_enforcement_clamp;
            output.unit = vrp_unitType_number;
            break;
    }

    return output;
}

#ifdef __cplusplus
}
#endif