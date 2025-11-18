//**********************************************************************************************************************
// FileName : serialBridgeProtocol.h
// FilePath : protocol/
// Author   : Christian Marty
// Date		: 15.09.2024
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#ifndef SERIAL_BRIDGE_PROTOCOL_H_
#define SERIAL_BRIDGE_PROTOCOL_H_

/* EXAMPLE *************************************************************************************************************


***********************************************************************************************************************/
#include "common/typedef.h"
#include "common/kernel.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	sbp_cmd_data = 0,
	sbp_cmd_portInfo = 4,
	sbp_cmd_portInfoReqest = 5
}sbp_command_t;

typedef enum {
	sbp_type_uart = 0,
	sbp_type_i2c = 1,
	sbp_type_lightBus = 2
}sbp_portType_t;


typedef enum {
	sbp_state_ok,
	sbp_state_crcError
}sbp_status_t;

typedef void (*sbp_onReceive_t)(const uint8_t *data, uint8_t size);

typedef struct{
	const sbp_portType_t portType;
	const sbp_onReceive_t onReceive;
}sbp_port_t;

typedef struct{
	const sbp_port_t* ports;
	const uint8_t portSize;
}serialBridgeProtocol_t;


bool sbp_receiveHandler(const serialBridgeProtocol_t* sbp, const bus_rxMessage_t *message);

void sbp_sendData(const serialBridgeProtocol_t* sbp, uint8_t destinationAddress, uint8_t port, sbp_status_t state, const uint8_t *data, uint8_t size);

#ifdef __cplusplus
}
#endif

#endif /* SERIAL_BRIDGE_PROTOCOL_H_ */