
#ifndef VALUE_REPORT_PROTOCOL_H_
#define VALUE_REPORT_PROTOCOL_H_

#include "sam.h"
#include "kernel/kernel.h"


typedef enum {
	vrp_cmd_valueReport  = 0,
	vrp_cmd_valueCommand  = 1,
	vrp_cmd_metaDataReport = 2,
	vrp_cmd_valueRequest = 3,
	vrp_cmd_metaDataRequest= 4
}vrp_commands_t;

/*enum datatype{
		VRP_UINT8,
		VRP_UINT16,
		VRP_UINT32,
		VRP_UINT64,
		VRP_INT8,
		VRP_INT16,
		VRP_INT32,
		VRP_INT64,
		VRP_BOOL,
		VRP_FLOAT
	};*/

typedef enum{
	UOM_RAW_FLOAT,
	UOM_RAW_UINT32,
	UOM_ABSOLUTE_TIME,
	UOM_RELATIVE_TIME,
	UOM_TEMPERATURE
}vrp_uom_t;


typedef enum{
	vrp_setToMinimum = 0x00,
    vrp_setToMaximum = 0x01,
    vrp_invert = 0x02,
    vrp_setToValue = 0x03,
	vrp_addValue = 0x04
}valueCommand_t;


typedef void (*vrp_valueChange_t)(uint8_t index);

typedef struct{
	const uint8_t channel;
	bool readOnly;
	float min;
	float max;
	float *value;
	vrp_uom_t uom;
	const char *description;
	vrp_valueChange_t vrp_valueChange;
	
}vrp_valueItem_t;

typedef struct{
	const uint8_t sourceAddress;
	const uint8_t channel;
	float *value;
	vrp_valueChange_t vrp_valueChange;
	
}vrp_valueSlaveItem_t;

typedef struct{
	const vrp_valueItem_t* value;
	const uint8_t valueSize;
	
	const vrp_valueSlaveItem_t* slaveValue;
	const uint8_t slaveValueSize;
}valueReportProtocol_t;

void vrp_init(const kernel_t *kernel, const valueReportProtocol_t* vrp);

void vrp_mainHandler(const kernel_t *kernel, const valueReportProtocol_t* vrp) ;



bool vrp_receiveHandler(const kernel_t *kernel, const valueReportProtocol_t* vrp, uint8_t sourceAddress, uint8_t command, const uint8_t *data, uint8_t size);

void vrp_sendValues(const kernel_t *kernel, const valueReportProtocol_t* vrp);

void vrp_sendValueReport(const kernel_t *kernel, uint8_t channel, float value);

void vrp_sendValueCommmand(const kernel_t *kernel, uint8_t channel, valueCommand_t command, float value);

#endif /* VALUE_REPORT_PROTOCOL_H_ */