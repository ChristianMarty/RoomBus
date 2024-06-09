/*
 * app.cpp
 *
 * Created: 20.04.2019 20:05:30
 * Author : Christian
 */ 

#include "sam.h"
#include "kernel.h"

#include "genericClockController.h"
#include "busController_IO.h"
#include "i2cModul_rev2.h"
#include "i2cMaster.h"

#include "triggerProtocol.h"
#include "stateReportProtocol.h"

#define DISPLAY_ADDRESS 0x27

int main(const kernel_t *kernel);
void onReceive(const kernel_t *kernel, uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size);

__attribute__((section(".appHeader"))) appHead_t appHead ={
/*appCRC	 */ 0xAABBCCDD, // Will be written by Bootload tool
/*appSize	 */ 0xEEFF0000, // Will be written by Bootload tool
/*appRevMaj	 */ 0x01,
/*appRevMin	 */ 0x00,
/*appName[60]*/ "LCD Test",
/*main		 */ main,
/*onRx		 */ onReceive
};

i2cMaster_c i2c;

uint32_t timer1;


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

void onReceive(const kernel_t *kernel, uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size)
{

}

uint8_t i;


/*
* Bit 0: RS ?
* Bit 1: RW ?
* Bit 2: E ?
* Bit 3: Back light
* Bit 4: Data 4 ?
* Bit 5: Data 5 ?
* Bit 6: Data 6 ?
* Bit 7: Data 7 ?
*/

bool instruction = true;
bool backlight = true; 

void lcd_setPort(uint8_t byte)
{
	i2c.transaction(DISPLAY_ADDRESS,byte,0,0,0,0);
}

void lcd_clk(void)
{
	uint8_t temp = 4;
	i2c.transaction(DISPLAY_ADDRESS,0x00,&temp,1,0,0);
}

uint8_t tempTx[6] = {0x00, 0x04, 0x00,0x00, 0x04, 0x00};
	
void lcd_writeByte(uint8_t byte)
{
	tempTx[0] = (byte & 0xF0);
	tempTx[1] = 0x04 | tempTx[0];
	tempTx[2] = 0x00 | tempTx[0];
	
	//tempTx[0] |= 0x04;
	
	for(uint8_t j = 0; j < 3;  j++)
	{
		if(instruction) tempTx[j] &= 0xFE;
		else tempTx[j] |= 0x01;
		
		if(backlight) tempTx[j] |= 0x08;
		else tempTx[j] &= 0xF7;
		
		tempTx[j] &= 0xFD; // Data write mode
	}

	i2c.transaction(DISPLAY_ADDRESS,tempTx[0],&tempTx[1],2,0,0);
	
}

void lcd_sendByte(uint8_t byte)
{
	//uint8_t temp[12] = {(byte & 0xF0),0xFF, 0x04,0xFF, 0x00,0xFF,(byte<<4),0xFF, 0x04,0xFF, 0x00,0xFF};
		
	tempTx[0] = (byte & 0xF0);
	tempTx[1] = 0x04 | tempTx[0];
	tempTx[2] = 0x00 | tempTx[0];
	tempTx[3] = (byte<<4);
	tempTx[4] = 0x04 | tempTx[3];
	tempTx[5] = 0x00 | tempTx[3];
	
	//tempTx[0] |= 0x04;
	//tempTx[3] |= 0x04;
	
	
	for(uint8_t j = 0; j < 6;  j++)
	{
		if(instruction) tempTx[j] &= 0xFE;
		else tempTx[j] |= 0x01;
		
		if(backlight) tempTx[j] |= 0x08;
		else tempTx[j] &= 0xF7;
		
		tempTx[j] &= 0xFD; // Data write mode
	}

	i2c.transaction(DISPLAY_ADDRESS,tempTx[0],&tempTx[1],5,0,0);
}

void lcd_init(void)
{
	
	/*if(i)
	uint8_t temp[] = {0x00,0x};
	
	if(i == 0) i = 0x08;
	else i = 0; 

	//_I2C->transaction(_address,03,&initData[0],5,0,0);
	i2c.transaction(0x26,i,0,0,0,0);*/
	//i++;
}

int main(const kernel_t *kernel)
{
	// App Init Code
	if(kernel->kernelSignals->initApp)
	{
		Reset_Handler();
		
		MCLK->APBDMASK.bit.SERCOM5_ = true;
		gclk_peripheralEnable(GCLK_SERCOM5_CORE,kernel->clk_1MHz);
		 
		kernel->assignInterruptHandler(SERCOM5_0_IRQn,i2cMbInterrupt);
		kernel->assignInterruptHandler(SERCOM5_1_IRQn,i2cSbInterrupt);
		kernel->assignInterruptHandler(SERCOM5_3_IRQn,i2cError);
		
		/*pin_enableOutput(IO_D14);
		pin_enableOutput(IO_D15);
		pin_enableInput(IO_D12);
		pin_enableInput(IO_D13);*/
				
		pin_enablePeripheralMux(IO_D12, PIN_FUNCTION_C);
		pin_enablePeripheralMux(IO_D13, PIN_FUNCTION_C);
		pin_enablePeripheralMux(IO_D14, PIN_FUNCTION_C);
		pin_enablePeripheralMux(IO_D15, PIN_FUNCTION_C);

		i2c.init(SERCOM5,10,true);
		
		i2cModul_init(kernel);
		 
		kernel->appSignals->appReady = true;
		
		kernel->tickTimer_reset(&timer1);	
	}
	

//----- Main -------------------------------------------------------------------------------------------------	
	
	if(kernel->appSignals->appReady == true) 
	{	
		if(kernel->tickTimer_delay1ms(&timer1, 10))
		{
			if(i == 0)
			{
				lcd_setPort(0);	
				i++;		
			}
			else if(i < 6)
			{
				i++;
			}
			else if(i == 6)
			{
				instruction = true;
				lcd_writeByte(0x38); // Set 4 Bit mode
				i++;
			}
			else if(i == 7)
			{
				lcd_sendByte(0x28); // Set 4 Bit mode
				i++;
			}
			else if(i == 8)
			{
				lcd_sendByte(0x28);
				i++;
			}
			else if(i == 9)
			{
				lcd_sendByte(0x0F);
				i++;
			}
			else if(i == 10)
			{
				lcd_sendByte(0x01);
				i++;
			}
			else if(i == 11)
			{
				lcd_sendByte(0x04);
				i++;
			}
			else if(i == 12)
			{
				instruction = false;
				lcd_sendByte('A');
				i++;
			}
			else if(i == 13)
			{
				lcd_sendByte('B');
				i++;
			}
			else if(i == 14)
			{
				lcd_sendByte('C');
				i++;
			}
			else if(i == 15)
			{
				lcd_sendByte('D');
				i++;
			}	
		}
	}
	
	// App Deinit code
    if(kernel->kernelSignals->shutdownApp)
    {
		//kernel->eemem_write(&ememData[0]);
		kernel->appSignals->shutdownReady = true;
    }
}