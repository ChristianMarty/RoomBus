
#include "addOn/i2cModul_rev2.h"
#include "driver/SAMx5x/i2cMaster.h"
#include "driver/SAMx5x/pin.h"
#include "common/io_same51jx.h"


void i2cModul_init(void)
{
	pin_enableOutput(IO_D16); // init expander port 
	pin_setOutput(IO_D16, false); // turn expander port off
	
	pin_enableOutput(IO_D17); // init I2C port enable
	pin_setOutput(IO_D17, true); // turn I2C port enable on
}

void i2cModul_handler(void)
{
	
}