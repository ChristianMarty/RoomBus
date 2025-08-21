//**********************************************************************************************************************
// FileName : kernel.h
// FilePath : common/
// Author   : Christian Marty
// Date		: 16.08.2024
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#ifndef KERNEL_INTERFACE_H_
#define KERNEL_INTERFACE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "typedef.h"

#include "kernel/littleFS/lfs.h"
#include "driver/SAMx5x/genericClockController.h"

// BUS Special Addresses
#define BROADCAST BUS_BROADCAST_ADDRESS
#define UNFILTERED 0xFF

typedef struct{
	 uint32_t appReady : 1;
	 uint32_t shutdownReady :1;
	 uint32_t appError : 1;
} appSignals_t;

typedef struct{
	uint32_t initApp : 1;
	uint32_t shutdownApp : 1;
	uint32_t ledDisabled : 1;
	uint32_t administratorAccess : 1;
	uint32_t identify : 1;
	uint32_t kernelError :1;
} kernelSignals_t;

typedef bool (*bus_getMessageSlot_t)(bus_message_t *message);
typedef void (*bus_writeHeader_t)(bus_message_t *message, uint8_t destAddr, busProtocol_t protocol, uint8_t command, busPriority_t priority);
typedef void (*bus_pushByte_t)(bus_message_t *message, uint8_t data);
typedef void (*bus_pushWord16_t)(bus_message_t *message, uint16_t data);
typedef void (*bus_pushWord24_t)(bus_message_t *message, uint32_t data);
typedef void (*bus_pushWord32_t)(bus_message_t *message, uint32_t data);
typedef void (*bus_pushArray_t)(bus_message_t *message, const uint8_t *data, uint8_t dataSize);
typedef void (*bus_pushString_t)(bus_message_t *message, const char *data);
typedef bool (*bus_send_t)(bus_message_t *message);

typedef void (*interruptHandler_t)(void);

typedef void (*wdt_clear_t)(void);

typedef void (*nvic_assignInterruptHandler_t)(IRQn_Type interruptVector, interruptHandler_t handler);
typedef void (*nvic_removeInterruptHandler_t)(IRQn_Type interruptVector);
typedef void (*nvic_setInterruptPriority_t)(IRQn_Type interruptVector, uint8_t priority);

typedef void (*sysTick_resetDelayCounter_t)(uint32_t *counter);
typedef bool (*sysTick_delay1ms_t)(uint32_t *counter, uint32_t delay);

typedef void (*eemem_read_t)(uint8_t *data);
typedef void (*eemem_write_t)(uint8_t *data);

typedef int (*file_open_t)(lfs_file_t *file, const char *path, int flags);
typedef int (*file_close_t)(lfs_file_t *file);
typedef lfs_ssize_t (*file_read_t)(lfs_file_t *file,void *buffer, lfs_size_t size);
typedef lfs_ssize_t (*file_write_t)(lfs_file_t *file, const void *buffer, lfs_size_t size);
typedef lfs_soff_t (*file_seek_t)(lfs_file_t *file, lfs_soff_t off, int whence);
typedef int (*file_rewind_t)(lfs_file_t *file);
typedef lfs_soff_t (*file_size_t)(lfs_file_t *file);
typedef int (*file_mount_t)(void);
typedef int (*file_unmount_t)(void);

typedef void (*logMessage_t)(const char *message);
typedef void (*logWarning_t)(const char *message);
typedef void (*logError_t)(const char *message);

typedef uint32_t (*rand_t)(void);
typedef uint32_t (*rand_inRange_t)(uint32_t min, uint32_t max);

typedef struct {
	
	uint8_t kernelRevMaj;
	uint8_t kernelRevMin;
	const uint8_t *deviceName;
	const uint8_t *hardwareName;
	
	kernelSignals_t *kernelSignals;
	appSignals_t *appSignals;
	
	gclk_generator_t clk_1MHz;
	gclk_generator_t clk_16MHz;
	
	struct {
		bus_getMessageSlot_t getMessageSlot;
		bus_writeHeader_t writeHeader;
		bus_pushByte_t pushByte;
		bus_pushWord16_t pushWord16;
		bus_pushWord24_t pushWord24;
		bus_pushWord32_t pushWord32;
		bus_pushArray_t pushArray;
		bus_pushString_t pushString;
		bus_send_t send;
	}bus;
	
	struct {
		sysTick_resetDelayCounter_t reset;
		sysTick_delay1ms_t delay1ms;
	}tickTimer;
	
	struct {
		nvic_assignInterruptHandler_t assignInterruptHandler;
		nvic_removeInterruptHandler_t removeInterruptHandler;
		nvic_setInterruptPriority_t setInterruptPriority;
	}nvic;
	
	struct {
		eemem_read_t read;
		eemem_write_t write;
	}eemem;
	
	struct {
		file_open_t open;
		file_close_t close;
		file_read_t read;
		file_write_t write;
		file_seek_t seek;
		file_rewind_t rewind;
		file_size_t size;
	}file;
	
	struct {
		logMessage_t message;
		logWarning_t warning;
		logError_t error;	
	}log;
	
	struct{
		rand_t rand;
		rand_inRange_t inRange;
	}rand;
	
	struct{
		wdt_clear_t clear;
	}wdt;
		
}kernel_t;

typedef int (*applicationMain_t)(void);
typedef bool (*onReceive_t)(const bus_rxMessage_t *message);

typedef struct {
	uint32_t appCRC;
	uint32_t appSize;
	uint8_t appRevMaj;
	uint8_t appRevMin;
 	uint8_t appName[60];
 	applicationMain_t appRun;
 	onReceive_t onRx;
}applicationHeader_t;

#ifdef __cplusplus
}
#endif
#endif /* KERNEL_INTERFACE_H_ */

#ifndef KERNEL_H_
#define KERNEL_H_

extern kernel_t kernel;

#endif /* KERNEL_H_ */