/*
 * app.cpp
 *
 * Created: 20.04.2019 20:05:30
 * Author : Christian
 */ 


#include "sam.h"
#include "kernel/kernel.h"
#include "kernel/busDefines.h"
#include "kernel/busController_IO.h"

#include "drv/SAMx5x/uart.h"
#include "drv/SAMx5x/pin.h"
#include "utility/softCRC.h"
#include "utility/string.h"
#include "Raumsteuerung/oneWire.h"

#include "Raumsteuerung/valueReportProtocol.h"

int main(const kernel_t *kernel);
void onReceive(const kernel_t *kernel, uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size);
kernel_t const *_kernel;

__attribute__((section(".appHeader"))) appHead_t appHead ={
/*appCRC	 */ 0xAABBCCDD, // Will be written by Bootload tool
/*appSize	 */ 0xEEFF0000, // Will be written by Bootload tool
/*appRevMaj	 */ 0x01,
/*appRevMin	 */ 0x00,
/*appName[60]*/ "DS18B10 Test",
/*main		 */ main,
/*onRx		 */ onReceive
};

#define OFF_DELAY 30000
#define STATE_SEND_INTERVAL 60000

uint32_t offTimer;
uint32_t sendStateTimer;

uint8_t ememData[256];

float temperature[4];

vrp_valueItem_t valueItemList[]={
	{ 0x00, true, ((float)(-100.0f)), ((float)(127.0f)), &temperature[0], UOM_RAW_FLOAT, "Temperature 1",nullptr},
	{ 0x01, true, ((float)(-100.0f)), ((float)(127.0f)), &temperature[1], UOM_RAW_FLOAT, "Temperature 2",nullptr},
	{ 0x02, true, ((float)(-100.0f)), ((float)(127.0f)), &temperature[2], UOM_RAW_FLOAT, "Temperature 3",nullptr},
	{ 0x03, true, ((float)(-100.0f)), ((float)(127.0f)), &temperature[3], UOM_RAW_FLOAT, "Temperature 4",nullptr}
};
#define valueItemListSize (sizeof(valueItemList)/sizeof(vrp_valueItem_t))

valueReportProtocol_t valueReportProtocol={
	.value = valueItemList,
	.valueSize = valueItemListSize
};

void onReceive(const kernel_t *kernel, uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size)
{	
	if(protocol == busProtocol_valueReportProtocol) vrp_receiveHandler(kernel,sourceAddress,command,data,size,&valueReportProtocol);
}

/*
uint32_t search_timer;
uint8_t seach_step;

char idStr[] = "0x0000000000000000";

oneWire_device_t devices[10];
uint8_t devicesCount = 0;


void read_rom(void)
{
	uint8_t txData[] = {0x33};
	transaction(&txData[0], 1 ,8);
}

uint64_t uid;
uint8_t last_branch = 0;
uint8_t bit_pos = 0;
bool branch_value = true;

void search_next(const kernel_t *kernel)
{
	seach_step = 0;
	bit_pos = 0;
}

void start_search(const kernel_t *kernel)
{
	kernel->log.message("OneWire search started");
	
	search_next(kernel);
	
	devicesCount = 0;
	last_branch = 0;
	
	branch_value = true;
}

void finish_search(const kernel_t *kernel)
{
	kernel->log.message("OneWire search finished");
	
	for(uint8_t i = 0; i<devicesCount; i++)
	{
		string_uInt64ToHex(devices[i].romcode, &idStr[2]);
		
		kernel->log.message(idStr);
	}
}

void serach_bit(const kernel_t *kernel)
{
	if(rxBufferPos == 2)
	{
		bit_pos++;
		
		if((rxBuffer[0] == 0xFF && rxBuffer[1] == 0xFF) || bit_pos > 80) //No devices participating in search.
		{
			devices[devicesCount] = oneWire_device_init(uid);
			devicesCount++;
			
			if(last_branch != 0)search_next(kernel);
			else finish_search(kernel);
			
			branch_value = !branch_value;
		}
		else 
		{
			uid = (uid >>1);
			if(rxBuffer[0] != 0xFF && rxBuffer[1] != 0xFF) // There are both 0s and 1s in the current bit position of the participating ROM numbers. This is a discrepancy.
			{
				if(branch_value)uid |= 0x8000000000000000;
				last_branch = bit_pos;
				
				send_bit(branch_value);
				
			}
			else if(rxBuffer[0] == 0xFF && rxBuffer[1] != 0xFF) // 	There are only 1s in the bit of the participating ROM numbers.
			{
				uid |= 0x8000000000000000;
				send_bit(true);
			
			}
			else if(rxBuffer[0] != 0xFF && rxBuffer[1] == 0xFF) // There are only 0s in the bit of the participating ROM numbers.
			{
				send_bit(false);
			}	
		}
		
	}	
}

void run_serch(const kernel_t *kernel)
{
	if(seach_step > 2)  serach_bit(kernel);
	if(!kernel->tickTimer.delay1ms(&search_timer,2)) return;
	
	if(seach_step == 0) 
	{
		send_reset();
		seach_step++;
		uid = 0;
		
	}
	else if(seach_step == 1)
	{
		uint8_t txData[] = {0xF0};
		transaction(&txData[0], 1 ,0);
		seach_step++;
	}
	else if(seach_step == 2)
	{
		txBuffer[0] = 0xFF;
		txBuffer[1] = 0xFF;
		
		rxBufferPos = 0;
		
		uartCom.sendData(&txBuffer[0], 2);
		seach_step++;
	}
}
*/

uint16_t readLoop = 0;

float outValOld[2];


uint32_t valueUpdateTimer;
uint32_t readTimer1;


float readings[4];


oneWire_readTemperature_t sensors[] ={
	{0x6100080224D45610,&readings[0],0,500},
	{0xD603109794430428,&readings[1],0,500}
};
#define sensorListSize (sizeof(sensors)/sizeof(oneWire_readTemperature_t)) 

int main(const kernel_t *kernel)
{
	_kernel = kernel;
	
	// App init Code
	if(kernel->kernelSignals->initApp)
	{
		Reset_Handler();
		
		pin_enableOutput(IO_D17);
		pin_setOutput(IO_D17, true);
		
		oneWire_init(kernel);
		
		// Uart
		pin_enablePeripheralMux(IO_D12, PIN_FUNCTION_C);
		pin_enablePeripheralMux(IO_D13, PIN_FUNCTION_C);
		kernel->nvic.assignInterruptHandler(SERCOM5_2_IRQn,oneWire_onRx);
		kernel->nvic.assignInterruptHandler(SERCOM5_1_IRQn,oneWire_onTx);
		
		
		/*kernel->eemem_read(&ememData[0]);*/

		kernel->tickTimer.reset(&readTimer1);
		kernel->tickTimer.reset(&valueUpdateTimer);
		readLoop = 0;
		
		kernel->appSignals->appReady = true;
	}

	// Main code here
	if(kernel->appSignals->appReady == true)
	{
		
		oneWire_readRun(kernel,sensors,sensorListSize);
		
		if(kernel->tickTimer.delay1ms(&valueUpdateTimer,1000))
		{
			for(uint8_t i = 0; i<sensorListSize; i++)
			{
				if(readings[i] != temperature[i])
				{
					vrp_sendValueReport(kernel,i,readings[i]);
					temperature[i] = readings[i];
					
					/*
					char tempStr[10];
					kernel->log.message( string_append(string_fromInt32((uint32_t)temperature[i]*10,1,&tempStr[0]), " °C")); //*/
				}
			}
		}
		
		//run_serch(kernel);
		/*
		if(kernel->tickTimer.delay1ms(&readTimer1,2))
		{
			if(readLoop == 0)
			{
				oneWire_sendReset();
			}
			else if(readLoop == 1)
			{
				oneWire_readRomCode();
			}
			else if(readLoop == 2)
			{
				uint64_t id = oneWire_parseRomData(oneWire_getRxBuffer());
				
				string_uInt64ToHex(id,&idStr[2]);
				
				kernel->log.message(idStr);
			}
			
			readLoop++;
			if(readLoop > 500) readLoop = 0;
		}
		//*/
			
	}
	
	// App deinit code
    if(kernel->kernelSignals->shutdownApp)
    {
		//kernel->eemem_write(&ememData[0]);
		kernel->appSignals->shutdownReady = true;
    }
}