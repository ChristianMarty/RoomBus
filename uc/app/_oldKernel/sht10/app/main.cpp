/*
 * app.cpp
 *
 * Created: 20.04.2019 20:05:30
 * Author : Christian
 */ 


#include "sam.h"
#include "kernel.h"
#include "busController_IO.h"

#include "pin.h"
#include "string.h"

#include "i2cModul_rev2.h"
#include "shtxx.h"

#include "valueReportProtocol.h"

int main(const kernel_t *kernel);
void onReceive(const kernel_t *kernel, uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size);
kernel_t const *_kernel;

__attribute__((section(".appHeader"))) appHead_t appHead ={
/*appCRC	 */ 0xAABBCCDD, // Will be written by Bootload tool
/*appSize	 */ 0xEEFF0000, // Will be written by Bootload tool
/*appRevMaj	 */ 0x01,
/*appRevMin	 */ 0x00,
/*appName[60]*/ "SHT10",
/*main		 */ main,
/*onRx		 */ onReceive
};

uint8_t ememData[256];

float temperature;
float humidity;

uint32_t readTimer;

vrp_valueItem_t valueItemList[]={
	{ 0x00, true, ((float)(0.0f)), ((float)(225.0f)), &temperature, UOM_RAW_FLOAT, "Temperature",nullptr},
	{ 0x01, true, ((float)(0.0f)), ((float)(225.0f)), &humidity, UOM_RAW_FLOAT, "Humidity",nullptr}
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


sht_c sht;

	
int main(const kernel_t *kernel)
{
	_kernel = kernel;
	
	// App init Code
	if(kernel->kernelSignals->initApp)
	{
		Reset_Handler();
		
		pin_enableOutput(IO_D14);
		pin_enableOutput(IO_D15);
		pin_enableInput(IO_D12);
		pin_enableInput(IO_D13);
				
		i2cModul_init(kernel);

		kernel->tickTimer.reset(&readTimer);
		kernel->appSignals->appReady = true;
	}

	// Main code here
	if(kernel->appSignals->appReady == true)
	{
		sht.handler(kernel);
		
		static bool reading = false;
		
		if(kernel->tickTimer.delay1ms(&readTimer, 1000))
		{
			temperature = sht.getTemperature();
			vrp_sendValueReport(kernel,0,temperature);
			
			humidity = sht.getHumidity();
			vrp_sendValueReport(kernel,1,humidity);
			
			if(reading)sht.readTemperature();
			else sht.readHumidity();
			
			reading  = !reading;
			
			
		}
		
	}
	
	// App deinit code
    if(kernel->kernelSignals->shutdownApp)
    {
		//kernel->eemem_write(&ememData[0]);
		kernel->appSignals->shutdownReady = true;
    }
}