/*
 * busDefines.h
 *
 * Created: 03.06.2018 21:11:07
 *  Author: Christian
 */ 


#ifndef BUS_DEFINES_H_
#define BUS_DEFINES_H_

#include <stdint.h>
#include <stdbool.h>

#define BUS_BROADCAST_ADDRESS 127

typedef enum {
	busProtocol_deviceManagementProtocol = 0,
	busProtocol_logging = 1,
	busProtocol_fileTransferProtocol = 2,
	
	busProtocol_triggerProtocol = 8, 
	busProtocol_eventProtocol = 9,
	busProtocol_stateReportProtocol = 10,
	busProtocol_valueReportProtocol = 11,
	busProtocol_serialBridgeProtocol = 12
}busProtocol_t;

typedef enum {
	busPriority_reserved = 0,
	busPriority_high = 1,
	busPriority_normal = 2,
	busPriority_low = 3
} busPriority_t;

typedef struct {
	uint8_t length;
	uint8_t bufferIndex;
}bus_message_t;


#endif /* BUS-DEFINES_H_ */