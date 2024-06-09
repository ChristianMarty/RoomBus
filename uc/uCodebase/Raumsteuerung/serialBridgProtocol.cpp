

#include "serialBridgProtocol.h"
#include "string.h"


void _sendPortInfo(const kernel_t *kernel, uint8_t destinationAddress, const serialBridgeProtocol_t *sbp);
void _handleData(const kernel_t *kernel, uint8_t port, const uint8_t *data, uint8_t size, const serialBridgeProtocol_t *sbp);


void sbp_receiveHandler(const kernel_t *kernel, uint8_t sourceAddress, uint8_t command, const uint8_t *data, uint8_t size, const serialBridgeProtocol_t* sbp)
{
	switch(command)
	{
		case sbp_cmd_portInfoReqest:	_sendPortInfo(kernel, sourceAddress, sbp); break;
		case sbp_cmd_data:				_handleData(kernel, 0, data, size,  sbp);
	}
}

void sbp_sendData(const kernel_t *kernel, uint8_t destinationAddress, uint8_t port, sbp_status_t state, const uint8_t *data, uint8_t size)
{
	bus_message_t msg;
	kernel->bus.getMessageSlot(&msg);
	kernel->bus.writeHeader(&msg,destinationAddress, busProtocol_serialBridgeProtocol, sbp_cmd_data, busPriority_low);
	kernel->bus.pushByte(&msg,port);
	kernel->bus.pushByte(&msg,state);
	kernel->bus.pushArray(&msg, &data[0],size);
	kernel->bus.send(&msg);
}

void _sendPortInfo(const kernel_t *kernel, uint8_t destinationAddress, const serialBridgeProtocol_t *sbp)
{
	bus_message_t msg;
	kernel->bus.getMessageSlot(&msg);
	kernel->bus.writeHeader(&msg,destinationAddress, busProtocol_serialBridgeProtocol, sbp_cmd_data, busPriority_low);
	kernel->bus.pushByte(&msg,sbp->portSize);
	for(uint8_t i = 0; i< sbp->portSize; i++)
	{
		kernel->bus.pushByte(&msg,sbp->ports[i].portType);
	}
	kernel->bus.send(&msg);	
}

void _handleData(const kernel_t *kernel, uint8_t port, const uint8_t *data, uint8_t size,  const serialBridgeProtocol_t *sbp)
{
	if(port > sbp->portSize) return;

	sbp->ports[port].onReceive(data,size);
}


