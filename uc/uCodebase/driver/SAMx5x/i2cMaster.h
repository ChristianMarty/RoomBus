
#ifndef I2C_MASTER_H_
#define I2C_MASTER_H_

#include "sam.h"
#include "driver/SAMx5x/genericClockController.h"
#include "common/kernel.h"

#define  I2C_MASTER_TIMEOUT 1000

class i2cMaster_c
{
	public:
	
	typedef uint8_t i2c_transactionNumber_t;
	
	void init(Sercom *sercom_p, gclk_generator_t clkGenerator, uint8_t clockDevisor, bool fourWire);
	void handler(void);
	
	i2c_transactionNumber_t transaction(uint8_t address, uint8_t command, uint8_t* txData, uint8_t txSize, uint8_t* rxData, uint8_t rxSize, i2c_transactionNumber_t transactionNumber);
	
	void reset(void);
	void resetError(void);
	bool hasError(void);
	bool isUnreachable(void);
	
	void MbInterrupt(void);
	void SbInterrupt(void);
	void ErrorInterrupt(void);
	
	bool busy(void);
	
	i2c_transactionNumber_t getCurrentTransactionNumber(void);
	i2c_transactionNumber_t getNextTransactionNumber(void);
	void closeTransaction(i2c_transactionNumber_t transactionNumber);
	
private:
	
	typedef enum {idel,txAddress,txCommand,txData,rxData,unreachable,error} i2cMaster_status_t;
	
	Sercom *_sercom_p;
	
	volatile i2cMaster_status_t _status;
	
	volatile uint8_t _bufferPos;
	
	uint8_t _command;
	uint8_t _address;
	
	uint8_t *_txData;
	uint8_t _txDataSize;
	
	uint8_t *_rxData;
	uint8_t _rxDataSize;
	
	uint8_t _clockDevisor;
	
	i2c_transactionNumber_t _transactionNumber;
	
	i2c_transactionNumber_t _lastTransactionNumber;

	gclk_generator_t _clkGenerator;
	
	uint32_t _timeoutTimer;
	
	
};
#endif /* I2C_MASTER_H_ */