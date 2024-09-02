
#ifndef AMS_TSL2561_H_
#define AMS_TSL2561_H_

#include "sam.h"
#include "kernel/kernel.h"
#include "drv/SAMx5x/i2cMaster.h"

typedef enum {
	tsl2561_state_pre_init = 0,
	tsl2561_state_pre_init_pending,
	tsl2561_state_init,
	tsl2561_state_init_pending,
	tsl2561_state_idle,
	tsl2561_state_read,
	tsl2561_state_read_pending,
	tsl2561_state_error = 0xFF
} tsl2561_state_t;

typedef struct{
	float illuminance;
	float brightness;
}ams_TSL2561_measurmentData_t;

typedef void (*ams_TSL2561_onMeasurmentDataUpdate_t)(ams_TSL2561_measurmentData_t *measurmentData);

typedef struct{
	Sercom *sercom_p;
	ams_TSL2561_measurmentData_t *measurmentData;
	ams_TSL2561_onMeasurmentDataUpdate_t onMeasurmentDataUpdate;
	
	i2cMaster_c i2c;
	i2cMaster_c::i2c_transactionNumber_t i2cTransaction;
	
	tsl2561_state_t state;
	
	uint8_t tx[5];
	uint8_t rx[16];
	
	uint32_t readTimer;

}ams_TSL2561_t;


void ams_TSL2561_init(const kernel_t *kernel, ams_TSL2561_t *tsl2561);
void ams_TSL2561_handler(const kernel_t *kernel, ams_TSL2561_t *tsl2561);

uint32_t ams_TSL2561_decode_illuminance(uint16_t ch0, uint16_t ch1);
uint8_t ams_TSL2561_decode_brightness(uint16_t ch0, uint16_t ch1);

void tsl2561_onMbInterrupt(ams_TSL2561_t *tsl2561);
void tsl2561_onSbInterrupt(ams_TSL2561_t *tsl2561);
void tsl2561_onErrorInterrupt(ams_TSL2561_t *tsl2561);

#endif /* AMS_TSL2561_H_ */