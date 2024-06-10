#include "valueReportProtocol.h"
#include "string.h"


void vrp_parseMetaDataRequest(const kernel_t *kernel, uint8_t sourceAddress, const uint8_t *data, uint8_t size, const valueReportProtocol_t* vrp);
void vrp_parseValueRequest(const kernel_t *kernel, uint8_t sourceAddress, const uint8_t *data, uint8_t size, const valueReportProtocol_t* vrp);
void vrp_parseValueCommand(const kernel_t *kernel, uint8_t sourceAddress, const uint8_t *data, uint8_t size, const valueReportProtocol_t* vrp);
void vrp_parseValueReport(const kernel_t *kernel, uint8_t sourceAddress, const uint8_t *data, uint8_t size, const valueReportProtocol_t* vrp);


void vrp_init(const kernel_t *kernel, const valueReportProtocol_t* vrp)
{
	
}

void vrp_mainHandler(const kernel_t *kernel, const valueReportProtocol_t* vrp)
{
	
}

void vrp_receiveHandler(const kernel_t *kernel, uint8_t sourceAddress, uint8_t command, const uint8_t *data, uint8_t size, const valueReportProtocol_t* vrp)
{
	if(command == vrp_cmd_metaDataRequest) vrp_parseMetaDataRequest(kernel, sourceAddress, data, size, vrp);
	else if(command == vrp_cmd_valueRequest) vrp_parseValueRequest(kernel, sourceAddress, data, size, vrp);
	else if(command == vrp_cmd_valueCommand) vrp_parseValueCommand(kernel, sourceAddress, data, size, vrp);
	else if(command == vrp_cmd_valueReport) vrp_parseValueReport(kernel, sourceAddress, data, size, vrp);
}

void vrp_sendMetaData(const kernel_t *kernel, const valueReportProtocol_t* vrp)
{
	for(uint8_t j = 0; j <vrp->valueSize; j++)
	{
		bus_message_t msg;
		kernel->bus.getMessageSlot(&msg);
		kernel->bus.writeHeader(&msg,BUS_BROADCAST_ADDRESS, busProtocol_valueReportProtocol, vrp_cmd_metaDataReport, busPriority_low);
		kernel->bus.pushByte(&msg, vrp->value[j].channel);
		uint8_t tmp = vrp->value[j].uom;
		if(vrp->value[j].readOnly) tmp |= 0x80;
		kernel->bus.pushByte(&msg, tmp);
		kernel->bus.pushWord32(&msg, *((uint32_t*)&vrp->value[j].min));
		kernel->bus.pushWord32(&msg, *((uint32_t*)&vrp->value[j].max));
		kernel->bus.pushString(&msg,  &vrp->value[j].description[0]);
		kernel->bus.send(&msg);
	}
}

void vrp_parseValueReport(const kernel_t *kernel, uint8_t sourceAddress, const uint8_t *data, uint8_t size, const valueReportProtocol_t* vrp)
{
	uint8_t channel = data[0];
	
	uint32_t temp = 0;
	temp =  ((uint32_t)data[1]<<24);
	temp |= ((uint32_t)data[2]<<16);
	temp |= ((uint32_t)data[3]<<8);
	temp |= ((uint32_t)data[4]);
	
	float value = *((float*)&temp);
	
	for(uint8_t j = 0; j <vrp->slaveValueSize; j++)
	{
		if(channel== vrp->slaveValue[j].channel)
		{
			*vrp->slaveValue[j].value = value;
			if(vrp->slaveValue[j].vrp_valueChange != nullptr) vrp->value[j].vrp_valueChange(j);
			break;
		}
	}
}


void vrp_parseValueCommand(const kernel_t *kernel, uint8_t sourceAddress, const uint8_t *data, uint8_t size, const valueReportProtocol_t* vrp)
{
	uint8_t channel = data[0];
	valueCommand_t command = (valueCommand_t)(data[1]&0x0F);
	
	uint32_t temp = 0;

	temp =  ((uint32_t)data[2]<<24);
	temp |= ((uint32_t)data[3]<<16);
	temp |= ((uint32_t)data[4]<<8);
	temp |= ((uint32_t)data[5]);
	
	float value = *((float*)&temp);
	
	
	for(uint8_t j = 0; j <vrp->valueSize; j++)
	{
		if(channel== vrp->value[j].channel)
		{
			switch(command)
			{
				case  vrp_setToValue: *vrp->value[j].value = value; break;
				case  vrp_addValue: *vrp->value[j].value += value; break;
			}
			
			
			if(vrp->value[j].vrp_valueChange != nullptr) vrp->value[j].vrp_valueChange(j);
			break;
		}
	}
}

void vrp_parseMetaDataRequest(const kernel_t *kernel, uint8_t sourceAddress, const uint8_t *data, uint8_t size, const valueReportProtocol_t* vrp)
{
	if(size == 0)
	{
		vrp_sendMetaData(kernel, vrp);
		return;
	}
	for(uint8_t i = 0; i < size; i++)
	{
		for(uint8_t j = 0; j <vrp->valueSize; j++)
		{
			if(data[i] == vrp->value[j].channel)
			{
				bus_message_t msg;
				kernel->bus.getMessageSlot(&msg);
				kernel->bus.writeHeader(&msg,sourceAddress, busProtocol_valueReportProtocol, vrp_cmd_metaDataReport, busPriority_low);
				kernel->bus.pushByte(&msg, vrp->value[j].channel);
				uint8_t tmp = vrp->value[j].uom;
				if(vrp->value[j].readOnly) tmp |= 0x80;
				kernel->bus.pushByte(&msg, tmp);
				kernel->bus.pushWord32(&msg, *((uint32_t*)&vrp->value[j].min));
				kernel->bus.pushWord32(&msg, *((uint32_t*)&vrp->value[j].max));
				kernel->bus.pushString(&msg,  &vrp->value[j].description[0]);
				kernel->bus.send(&msg);
				break;
			}
		}
	}
}


void vrp_sendValues(const kernel_t *kernel, const valueReportProtocol_t* vrp)
{
	for(uint8_t j = 0; j <vrp->valueSize; j++)
	{
		vrp_sendValueReport(kernel, vrp->value[j].channel, *vrp->value[j].value);
		/*bus_message_t msg;
		kernel->bus.getMessageSlot(&msg);
		kernel->bus.writeHeader(&msg, BUS_BROADCAST_ADDRESS, busProtocol_valueReportProtocol, vrp_cmd_valueReport, busPriority_low);
		
		kernel->bus.pushByte(&msg, vrp->value[j].channel);
		kernel->bus.pushWord32(&msg, *((uint32_t*)&vrp->value[j].value));
		
		kernel->bus.send(&msg);*/
	}
}

void vrp_parseValueRequest(const kernel_t *kernel, uint8_t sourceAddress, const uint8_t *data, uint8_t size, const valueReportProtocol_t* vrp)
{
	if(size == 0)
	{
		vrp_sendValues(kernel, vrp);
		return;
	}
	
	for(uint8_t i = 0; i < size; i++)
	{
		for(uint8_t j = 0; j <vrp->valueSize; j++)
		{
			if(data[i] == vrp->value[j].channel)
			{
				bus_message_t msg;
				kernel->bus.getMessageSlot(&msg);
				kernel->bus.writeHeader(&msg, BUS_BROADCAST_ADDRESS, busProtocol_valueReportProtocol, vrp_cmd_valueReport, busPriority_low);
				
				kernel->bus.pushByte(&msg, vrp->value[j].channel);
				kernel->bus.pushWord32(&msg, *((uint32_t*)&vrp->value[j].value));
				
				kernel->bus.send(&msg);
			}
		}
	}
}

void vrp_sendValueReport(const kernel_t *kernel, uint8_t channel, float value)
{
	bus_message_t msg;
	kernel->bus.getMessageSlot(&msg);

	kernel->bus.writeHeader(&msg, BUS_BROADCAST_ADDRESS, busProtocol_valueReportProtocol, vrp_cmd_valueReport, busPriority_low);
	
	kernel->bus.pushByte(&msg, channel);
	kernel->bus.pushWord32(&msg, *((uint32_t*)&value));
	
	kernel->bus.send(&msg);
}

void vrp_sendValueCommmand(const kernel_t *kernel, uint8_t channel, valueCommand_t command, float value)
{
	bus_message_t msg;
	kernel->bus.getMessageSlot(&msg);

	kernel->bus.writeHeader(&msg, BUS_BROADCAST_ADDRESS, busProtocol_valueReportProtocol, vrp_cmd_valueCommand, busPriority_normal);
	
	kernel->bus.pushByte(&msg, channel);
	kernel->bus.pushByte(&msg, command);
	kernel->bus.pushWord32(&msg, *((uint32_t*)&value));
	
	kernel->bus.send(&msg);
}
