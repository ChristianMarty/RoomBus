
#ifndef SERIAL_BRIDGE_PROTOCOL_H_
#define SERIAL_BRIDGE_PROTOCOL_H_

#include "sam.h"
#include "kernel/kernel.h"

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


void sbp_receiveHandler(const kernel_t *kernel, uint8_t sourceAddress, uint8_t command, const uint8_t *data, uint8_t size, const serialBridgeProtocol_t* sbp);


void sbp_sendData(const kernel_t *kernel, uint8_t destinationAddress, uint8_t port, sbp_status_t state, const uint8_t *data, uint8_t size);


#endif /* SERIAL_BRIDGE_PROTOCOL_H_ */