#ifndef MODBUSMASTER_H_
#define MODBUSMASTER_H_

#include "sam.h"
#include "common/kernel.h"

#include "utility/cobs_u8.h"
#include "common/io_same51jx.h"
#include "driver/SAMx5x/rs485.h"

typedef void (*lb_onReceive_t)(const uint8_t *data, uint8_t size, bool error);

typedef enum {
	modbusMaster_fc_readCoils = 0x01,
	modbusMaster_fc_readDiscreteInputs = 0x02, 
	modbusMaster_fc_readHoldingRegisters = 0x03, 
	modbusMaster_fc_readInputRegister = 0x04, 
	modbusMaster_fc_writeSingleCoil = 0x05, 
	modbusMaster_fc_writeSingleRegister = 0x06, 
	modbusMaster_fc_readExceptionStatus = 0x07, 
	modbusMaster_fc_diagnostic = 0x08, 
	modbusMaster_fc_getComEventCounter = 0x0B, 
	modbusMaster_fc_getComEventLog = 0x0C, 
	modbusMaster_fc_writeMultipleCoils = 0x0F, 
	modbusMaster_fc_writeMultipleRegisters = 0x10, 
	modbusMaster_fc_reportServerId = 0x11,
	modbusMaster_fc_readFileRecord = 0x14, 
	modbusMaster_fc_writeFileRecord = 0x15, 
	modbusMaster_fc_maskWriteRegister = 0x16, 
	modbusMaster_fc_readWriteMultipleRegisters = 0x17, 
	modbusMaster_fc_readFifiQueue = 0x18
} modbusMaster_functionCode_t;

typedef struct  modbusMaster_reading_s modbusMaster_reading_t;

typedef void (*modbusMaster_onReadingCompleted_t)(const modbusMaster_reading_t *reading);

struct modbusMaster_reading_s{
	uint8_t deviceAddress;
	modbusMaster_functionCode_t functionCode;
	uint16_t registerAddress;
	float *reading;
	uint16_t interval;
	modbusMaster_onReadingCompleted_t callback;	
	uint32_t _timer;
};


#define MODBUS_MASTER_BUFFER_SIZE 150
#define MODBUS_MASTER_TIMEOUT 5000 // in ms

typedef struct{
	rs485_t rs485;
	
	modbusMaster_reading_t *readings;
	uint8_t readings_size;
	uint8_t readings_index;
	
	uint32_t com_timer;
	lb_onReceive_t onReceive;

	uint8_t txData[MODBUS_MASTER_BUFFER_SIZE];
	bytearray_t txBuffer;

	uint8_t rxData[MODBUS_MASTER_BUFFER_SIZE];
	bytearray_t rxBuffer;
	
}modbusMaster_t;

void modbusMaster_init(const kernel_t *kernel, modbusMaster_t *modbusMaster, Sercom *sercom_p);
void modbusMaster_handler(const kernel_t *kernel, modbusMaster_t *modbusMaster);

void modbusMaster_attachReadings(const kernel_t *kernel, modbusMaster_t *modbusMaster, 	modbusMaster_reading_t *readings, uint8_t readings_size);

void modbusMaster_rxInterrupt(modbusMaster_t *modbusMaster);
void modbusMaster_txInterrupt(modbusMaster_t *modbusMaster);

uint32_t modbusMaster_request(uint8_t deviceAddress, uint8_t *data, uint8_t size);

#endif /* MODBUSMASTER_H_ */