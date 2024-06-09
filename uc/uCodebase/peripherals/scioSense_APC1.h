
#ifndef SCIOSENSE_APC1_H_
#define SCIOSENSE_APC1_H_

#include "sam.h"
#include "kernel/kernel.h"
#include "drv/SAMx5x/uart.h"

typedef struct{
	uint16_t frameHeader;
	uint16_t frameLenght;
	float pm1_0;
	float pm2_5;
	float pm10;
	float pm1_0Air;
	float pm2_5Air;
	float pm10Air;
	float count0_3um;
	float count0_5um;
	float count1_0um;
	float count2_5um;
	float count5_0um;
	float count10um;
	float tvoc;
	float eco2;
	float t_compensated;
	float rh_compensated;
	float t_raw;
	float rh_raw;
	float airQualityIndex;
	uint8_t  version;
	uint8_t  errorCode;
	uint16_t checksum;
}scioSense_apc1_measurmentData_t;

typedef void (*scioSense_apc1_onMeasurmentDataUpdate_t)(scioSense_apc1_measurmentData_t *measurmentData);

typedef struct{
	Sercom *sercom_p;
	scioSense_apc1_measurmentData_t *measurmentData;
	scioSense_apc1_onMeasurmentDataUpdate_t onMeasurmentDataUpdate;
	
	uart_c uartCom;
	uint32_t readTimer;
	uint8_t rxData[70];
	uint8_t rxIndex;
	
}scioSense_apc1_t;


void scioSense_apc1_init(const kernel_t *kernel, scioSense_apc1_t *apc1);
void scioSense_apc1_handler(const kernel_t *kernel, scioSense_apc1_t *apc1);

bool scioSense_apc1_decode_measurmentData(uint8_t *rxData, scioSense_apc1_measurmentData_t *measurmentData);


void scioSense_apc1_onRx(scioSense_apc1_t *apc1);
void scioSense_apc1_onTx(scioSense_apc1_t *apc1);

#endif /* SCIOSENSE_APC1_H_ */