
#ifndef I2C_MODUL_H_
#define I2C_MODUL_H_

#include "common/kernel.h"


void i2cModul_init(void);
void i2cModul_handler(void);

static inline void i2cModul_xpEnable(bool enable)
{
	pin_setOutput(IO_D16,enable); // Enable expander port
}

#endif /* I2C_MODUL_H_ */