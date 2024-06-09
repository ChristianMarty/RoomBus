

#include "stateReportProtocol.h"
#include "string.h"


void srp_receiveHandler(const kernel_t *kernel, uint8_t sourceAddress, uint8_t command, const uint8_t *data, uint8_t size, const stateReportProtocol_t *srp)
{
	switch(command)
	{
		case srp_cmd_group0Report :srp_parseGroup0Report(kernel,sourceAddress,data,size,srp); break;
		case srp_cmd_stateReportRequest: srp_stateReportRequestHandler(kernel, sourceAddress, data, size, srp); break;
		case srp_cmd_stateReportChannelNameRequest: srp_stateReportChannelNameRequestHandler(kernel, sourceAddress, data, size, srp);break;
		
	}
}

void srp_stateReportRequestAll(const kernel_t *kernel, const stateReportProtocol_t *srp)
{
	bus_message_t msg;
	kernel->bus.getMessageSlot(&msg);
	
	uint8_t sourceAddressLast = srp->states[0].sourceAddress;
	kernel->bus.writeHeader(&msg,sourceAddressLast, busProtocol_stateReportProtocol, srp_cmd_stateReportRequest, busPriority_low);
	
	for(uint8_t j = 0; j <srp->stateSize; j++)
	{
		if(sourceAddressLast != srp->states[j].sourceAddress)
		{
			kernel->bus.send(&msg);
			sourceAddressLast = srp->states[j].sourceAddress;
			
			kernel->bus.getMessageSlot(&msg);
			kernel->bus.writeHeader(&msg,sourceAddressLast, busProtocol_stateReportProtocol, srp_cmd_stateReportRequest, busPriority_low);	
		}
		
		kernel->bus.pushByte(&msg, srp->states[j].channel);	
	}
	
	kernel->bus.send(&msg);
	
	
}


void srp_stateReportRequestHandler(const kernel_t *kernel, uint8_t sourceAddress, const uint8_t *data, uint8_t size, const stateReportProtocol_t *srp)
{
	if(size == 0)
	{
		srp_sendGroupReport(kernel, srp);
		return;
	}
	
	bus_message_t msg;
	kernel->bus.getMessageSlot(&msg);
	kernel->bus.writeHeader(&msg,BUS_BROADCAST_ADDRESS, busProtocol_stateReportProtocol, srp_cmd_individualStateReport, busPriority_low);
	
	for(uint8_t i = 0; i < size; i++)
	{
		for(uint8_t j = 0; j <srp->channelSize; j++)
		{
			if(data[i] == srp->channels[j].channel)
			{
				kernel->bus.pushByte(&msg, srp->channels[j].channel);
				kernel->bus.pushByte(&msg, (uint8_t)*srp->channels[j].state);
			}
		}
	}
	
	kernel->bus.send(&msg);
}

void srp_stateReportChannelNameRequestHandler(const kernel_t *kernel, uint8_t sourceAddress, const uint8_t *data, uint8_t size, const stateReportProtocol_t *srp)
{
	for(uint8_t i = 0; i < size; i++)
	{
		for(uint8_t j = 0; j <srp->channelSize; j++)
		{
			if(data[i] == srp->channels[j].channel)
			{
				bus_message_t msg;
				kernel->bus.getMessageSlot(&msg);
				kernel->bus.writeHeader(&msg,BUS_BROADCAST_ADDRESS, busProtocol_stateReportProtocol, srp_cmd_stateReportChannelNameReporting, busPriority_low);
				kernel->bus.pushByte(&msg, data[i]);
				kernel->bus.pushString(&msg, &srp->channels[j].description[0]);
				kernel->bus.send(&msg);
				break;
			}
		}
	}
}

void srp_sendGroupReport(const kernel_t *kernel, const stateReportProtocol_t *srp)
{
	bus_message_t msg;
	kernel->bus.getMessageSlot(&msg);
	kernel->bus.writeHeader(&msg,BUS_BROADCAST_ADDRESS, busProtocol_stateReportProtocol, srp_cmd_group0Report, busPriority_low);
	
	uint8_t temp;
	
	for(uint8_t i = 0; i < srp->channelSize; i += 2)
	{
		temp  = 0;
		temp  = ((uint8_t)*srp->channels[i].state & 0x0F);
		if(i+1 < srp->channelSize) temp |= (((uint8_t)*srp->channels[i+1].state & 0x0F)<<4);
		else temp |= (((uint8_t)srp_state_undefined & 0x0F)<<4);
		
		kernel->bus.pushByte(&msg, temp);
	}
	
	kernel->bus.send(&msg);
	
	// TODO: Add support for >64 channels
}

void srp_parseGroup0Report(const kernel_t *kernel, uint8_t sourceAddress, const uint8_t *data, uint8_t size, const stateReportProtocol_t *srp)
{
	bool hasChange = false;
	
	for(uint8_t i = 0; i< size; i++)
	{
		srp_state_t state0 = (srp_state_t)(data[i]&0x0F);
		srp_state_t state1 = (srp_state_t)((data[i]>>4)&0x0F);
		
		for(uint8_t j = 0; j< srp->stateSize; j++)
		{
			if((srp->states[j].sourceAddress == sourceAddress)&&(srp->states[j].channel == i*2))
			{
				if(*(srp->states[j].state) !=  state0) hasChange = true;
				*(srp->states[j].state) = state0;
				break;
			}
		}
		
		for(uint8_t j = 0; j< srp->stateSize; j++)
		{
			if((srp->states[j].sourceAddress == sourceAddress)&&(srp->states[j].channel == i*2+1))
			{
				if(*(srp->states[j].state) !=  state1) hasChange = true;
				*(srp->states[j].state) = state1;
				break;
			}
		}
	}
	
	if(hasChange) tsp_onStateChange_callback(kernel);
}

void srp_parseGroup1Report(const kernel_t *kernel, uint8_t sourceAddress, const uint8_t *data, uint8_t size, const stateReportProtocol_t *srp)
{
	// TODO: Add support
}

void srp_parseIndividualStateReport(const kernel_t *kernel, uint8_t sourceAddress, const uint8_t *data, uint8_t size, const stateReportProtocol_t *srp)
{
	bool hasChange = false;
	
	for(uint8_t i = 0; i < size; i += 2)
	{
		uint8_t channel = data[i];
		srp_state_t state = (srp_state_t)data[i+1];
		
		for(uint8_t j = 0; j< srp->stateSize; j++)
		{
			if((srp->states[j].sourceAddress == sourceAddress)&&(srp->states[j].channel == channel))
			{
				if(*srp->states[j].state !=  state) hasChange = true;
				*srp->states[j].state = state;
				break;
			}
		}
	}

	if(hasChange) tsp_onStateChange_callback(kernel);
}
