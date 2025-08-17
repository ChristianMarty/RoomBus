
#ifndef SPI_MASTER_H_
#define SPI_MASTER_H_

#include "sam.h"
#include "driver/SAMx5x/genericClockController.h"
#include "common/kernel.h"

#define  I2C_MASTER_TIMEOUT 1000

class spiMaster_c
{
public:
	
	void init(Sercom *sercom_p, gclk_generator_t clkGenerator, uint8_t clockDevisor);
	
	void handler(void);
	
	void send(uint32_t data);

	
	void interruptHandler_0(void);
	void interruptHandler_1(void);
	void interruptHandler_2(void);
	void interruptHandler_3(void);

	bool busy(void);
	
private:

	Sercom *_sercom_p;
	gclk_generator_t _clkGenerator;
	uint8_t _clockDevisor;
	
	bool _busy;
	
};
#endif /* SPI_MASTER_H_ */