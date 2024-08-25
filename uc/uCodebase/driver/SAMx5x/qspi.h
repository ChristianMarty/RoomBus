/*
 * QSPI.h
 *
 * Created: 14.02.2019 23:53:43
 *  Author: Christian
 */ 


#ifndef QSPI_H_
#define QSPI_H_
#include "sam.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


#define EXTFLASH __attribute__((section(".extFlash")))

#define QSPI_START_ADDRESS 0x04000000
//#define QSPI_SIZE 0x1000000
#define QSPI_END_ADDRESS (QSPI_START_ADDRESS+QSPI_SIZE)
#define QSPI_BLOCK_SIZE 4096

typedef struct{
	uint8_t WIP:1;
	uint8_t WEL:1;
	uint8_t BP0:1;
	uint8_t BP1:1;
	uint8_t BP2:1;
	uint8_t BP3:1;
	uint8_t QE:1;
	uint8_t SRWD:1;
}qspi_flash_status_t;

typedef enum{
	qspi_noError,

	qspi_errorSize	
}qspi_error_t;

qspi_error_t qspi_init(void);

qspi_error_t qspi_write(uint8_t *address, const uint8_t *data, uint32_t size);

qspi_error_t qspi_read(uint8_t *address, uint8_t *data, uint32_t size);

qspi_error_t qspi_sectorErase(uint8_t *address);

qspi_error_t qspi_chipErase(void);


qspi_error_t qspi_writeEnable(void);

bool qspi_busy(void);

uint8_t qspi_readStatus1(void);
uint8_t qspi_readStatus2(void);

qspi_error_t qspi_writeStatus2(uint8_t status);

uint16_t qspi_readJedecId(void);
qspi_error_t qspi_resetDevice(void);

#ifdef __cplusplus
}
#endif

#endif /* QSPI_H_ */