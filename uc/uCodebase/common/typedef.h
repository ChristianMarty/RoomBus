//**********************************************************************************************************************
// FileName : typedef.c
// FilePath : uCodebase/common/
// Author   : Christian Marty
// Date		: 26.05.2024
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#ifndef TYPEDEF_H_
#define TYPEDEF_H_

#include <stdint.h>
#include <stdbool.h>
#include "main.h"

#ifndef TEST_RUN
	#include "sam.h"
	#include "driver/SAMx5x/pin.h"
#endif

#define ARRAY_LENGTH(array) (sizeof(array)/sizeof(array[0]))

#define BUS_BROADCAST_ADDRESS 127

typedef enum {
	busProtocol_deviceManagementProtocol = 0,
	busProtocol_logging = 1,
	busProtocol_fileTransferProtocol = 2,
	
	busProtocol_triggerSystemProtocol = 8, 
	busProtocol_eventSystemProtocol = 9,
	busProtocol_stateSystemProtocol = 10,
	busProtocol_valueSystemProtocol = 11,
	busProtocol_serialBridgeProtocol = 12
}busProtocol_t;

typedef enum {
	busPriority_immediate = 0,
	busPriority_high = 1,
	busPriority_normal = 2,
	busPriority_low = 3
}busPriority_t;


typedef uint8_t busCommand_t;

typedef struct {
	uint8_t sourceAddress;
	busProtocol_t protocol;
	busCommand_t command;
	bool broadcast;
	uint8_t data[64];
	uint8_t length;
}bus_rxMessage_t;

 
#ifndef TEST_RUN

typedef struct {
	uint8_t length;	// the length of the data in the buffer
	uint8_t bufferIndex; // the index of the selected buffer
}bus_txMessage_t;

#else

typedef struct {
    uint8_t destinationAddress; 
    busProtocol_t protocol;
    busCommand_t command;
    busPriority_t priority;
    uint8_t data[64];
    uint8_t length;
}bus_txMessage_t;

#endif

#endif /* TYPEDEF_H_ */