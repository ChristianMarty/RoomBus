//**********************************************************************************************************************
// FileName : qspi.h
// FilePath : uCodebase/driver/SAMx5x/kernel/
// Author   : Christian Marty
// Date		: 14.02.2019
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#ifndef QSPI_H_
#define QSPI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "common/typedef.h"

#define QSPI_START_ADDRESS 0x04000000
#define QSPI_BLOCK_SIZE 4096

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