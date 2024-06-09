
#ifndef I2C_MODUL_H_
#define I2C_MODUL_H_

#include "sam.h"
#include "kernel/kernel.h"
#include "drv/SAMx5x/pin.h"
#include "kernel/busController_IO.h"

void i2cModul_init(const kernel_t *kernel);
void i2cModul_handler(const kernel_t *kernel);

static inline void i2cModul_xpEnable(bool enable)
{
	pin_setOutput(IO_D16,enable); // Enable expander port
}

#endif /* I2C_MODUL_H_ */