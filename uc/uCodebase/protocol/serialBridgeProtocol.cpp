//**********************************************************************************************************************
// FileName : serialBridgeProtocol.cpp
// FilePath : protocol/
// Author   : Christian Marty
// Date		: 15.09.2024
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#include "serialBridgeProtocol.h"

#ifdef __cplusplus
extern "C" {
#endif
	
bool _sendPortInfo(uint8_t destinationAddress, const serialBridgeProtocol_t *sbp);
bool _handleData(uint8_t port, const uint8_t *data, uint8_t size, const serialBridgeProtocol_t *sbp);

bool sbp_receiveHandler(const serialBridgeProtocol_t* sbp, const bus_rxMessage_t *message)
{
	switch(message->command)
	{
		case sbp_cmd_portInfoReqest:	return _sendPortInfo(message->sourceAddress, sbp);
		case sbp_cmd_data:				return _handleData(0, message->data, message->length, sbp); 
	}
	return false;
}

void sbp_sendData(const serialBridgeProtocol_t* sbp, uint8_t destinationAddress, uint8_t port, sbp_status_t state, const uint8_t *data, uint8_t size)
{
	bus_txMessage_t msg;
	if(kernel.bus.getMessageSlot(&msg) == false) return; // Abort if TX buffer full
		
	kernel.bus.getMessageSlot(&msg);
	kernel.bus.writeHeader(&msg,destinationAddress, busProtocol_serialBridgeProtocol, sbp_cmd_data, busPriority_low);
	kernel.bus.pushByte(&msg,port);
	kernel.bus.pushByte(&msg,state);
	kernel.bus.pushArray(&msg, &data[0],size);
	kernel.bus.send(&msg);
}

bool _sendPortInfo(uint8_t destinationAddress, const serialBridgeProtocol_t *sbp)
{
	bus_txMessage_t msg;
	if(kernel.bus.getMessageSlot(&msg) == false) return false; // Abort if TX buffer full
		
	kernel.bus.getMessageSlot(&msg);
	kernel.bus.writeHeader(&msg,destinationAddress, busProtocol_serialBridgeProtocol, sbp_cmd_data, busPriority_low);
	kernel.bus.pushByte(&msg,sbp->portSize);
	for(uint8_t i = 0; i< sbp->portSize; i++)
	{
		kernel.bus.pushByte(&msg,sbp->ports[i].portType);
	}
	kernel.bus.send(&msg);
	
	return true;	
}

bool _handleData(uint8_t port, const uint8_t *data, uint8_t size,  const serialBridgeProtocol_t *sbp)
{
	if(port > sbp->portSize) return false;

	sbp->ports[port].onReceive(data,size);
	
	return true;
}


#ifdef __cplusplus
}
#endif