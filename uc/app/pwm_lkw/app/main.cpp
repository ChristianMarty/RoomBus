/*
 * app.cpp
 *
 * Created: 20.04.2019 20:05:30
 * Author : Christian
 */ 


#include "sam.h"
#include "kernel/kernel.h"
#include "kernel/busController_IO.h"

#include "drv/SAMx5x/pin.h"
#include "utility/string.h"

#include "addOn/i2cModul_rev2.h"
#include "drv/SAMx5x/i2cMaster.h"

#include "Raumsteuerung/valueReportProtocol.h"

int main(const kernel_t *kernel);
void onReceive(const kernel_t *kernel, uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size);
kernel_t const *_kernel;

__attribute__((section(".appHeader"))) appHead_t appHead ={
/*appCRC	 */ 0xAABBCCDD, // Will be written by Bootload tool
/*appSize	 */ 0xEEFF0000, // Will be written by Bootload tool
/*appRevMaj	 */ 0x01,
/*appRevMin	 */ 0x00,
/*appName[60]*/ "I2C PCA9634PW",
/*main		 */ main,
/*onRx		 */ onReceive
};


uint8_t ememData[256];

float ch[8];

uint8_t out[8];
bool update = false;

void vrp_valueChange(uint8_t index)
{
	if(index > sizeof(out)) return;
	
	out[index] = (uint8_t) ch[index];
	
	update = true;
	
}

vrp_valueItem_t valueItemList[]={
	{ 0x00, false, ((float)(0.0f)), ((float)(225.0f)), &ch[0], UOM_RAW_UINT32, "CH 1",vrp_valueChange},
	{ 0x01, false, ((float)(0.0f)), ((float)(225.0f)), &ch[1], UOM_RAW_UINT32, "CH 2",vrp_valueChange},
	{ 0x02, false, ((float)(0.0f)), ((float)(225.0f)), &ch[2], UOM_RAW_UINT32, "CH 3",vrp_valueChange},
	{ 0x03, false, ((float)(0.0f)), ((float)(225.0f)), &ch[3], UOM_RAW_UINT32, "CH 4",vrp_valueChange},
	{ 0x04, false, ((float)(0.0f)), ((float)(225.0f)), &ch[4], UOM_RAW_UINT32, "CH 5",vrp_valueChange},
	{ 0x05, false, ((float)(0.0f)), ((float)(225.0f)), &ch[5], UOM_RAW_UINT32, "CH 6",vrp_valueChange},
	{ 0x06, false, ((float)(0.0f)), ((float)(225.0f)), &ch[6], UOM_RAW_UINT32, "CH 7",vrp_valueChange},
	{ 0x07, false, ((float)(0.0f)), ((float)(225.0f)), &ch[7], UOM_RAW_UINT32, "CH 8",vrp_valueChange}
};
#define valueItemListSize (sizeof(valueItemList)/sizeof(vrp_valueItem_t)) 

valueReportProtocol_t valueReportProtocol=
{
	.value = valueItemList,
	.valueSize = valueItemListSize
};


void onReceive(const kernel_t *kernel, uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size)
{
	if(protocol == busProtocol_valueReportProtocol) vrp_receiveHandler(kernel,sourceAddress,command,data,size,&valueReportProtocol);
}


i2cMaster_c i2c;

void i2cMbInterrupt(void)
{
	i2c.MbInterrupt();
}

void i2cSbInterrupt(void)
{
	i2c.SbInterrupt();
}

void i2cError(void)
{
	i2c.ErrorInterrupt();
}

uint8_t initData[] = {0x80,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF};

i2cMaster_c::i2c_transactionNumber_t transNo = 0;

int main(const kernel_t *kernel)
{
	_kernel = kernel;
	
	// App init Code
	if(kernel->kernelSignals->initApp)
	{
		Reset_Handler();
		
		vrp_init(kernel, &valueReportProtocol);
		
		i2cModul_init(kernel);		
		i2c.init(kernel,SERCOM5,kernel->clk_1MHz,10,true);
		kernel->nvic.assignInterruptHandler(SERCOM5_0_IRQn,i2cMbInterrupt);
		kernel->nvic.assignInterruptHandler(SERCOM5_1_IRQn,i2cSbInterrupt);
		kernel->nvic.assignInterruptHandler(SERCOM5_3_IRQn,i2cError);
		
		pin_enablePeripheralMux(IO_D12, PIN_FUNCTION_C);
		pin_enablePeripheralMux(IO_D13, PIN_FUNCTION_C);
		pin_enablePeripheralMux(IO_D14, PIN_FUNCTION_C);
		pin_enablePeripheralMux(IO_D15, PIN_FUNCTION_C);
		
		pin_enableOutput(IO_D14);
		pin_enableOutput(IO_D15);
		pin_enableInput(IO_D12);
		pin_enableInput(IO_D13);
				
		kernel->appSignals->appReady = true;
		
		// Init PCA9634
		
		transNo = i2c.transaction(0x00,0x80,&initData[0],sizeof(initData),0,0,0);
	}

	// Main code here
	if(kernel->appSignals->appReady == true)
	{
		i2c.handler();
		vrp_mainHandler(kernel,&valueReportProtocol);
		
		if(update == true && !i2c.busy())
		{
			transNo = i2c.transaction(0x00,0xA2,&out[0],8,0,0,0);
			update = false;
		}
		
		if(i2c.isUnreachable()) i2c.closeTransaction(transNo);
		else if(!i2c.busy()) i2c.closeTransaction(transNo);
		else if(i2c.hasError()) i2c.reset();
	}
	
	// App deinit code
    if(kernel->kernelSignals->shutdownApp)
    {
		//kernel->eemem_write(&ememData[0]);
		kernel->appSignals->shutdownReady = true;
    }
}