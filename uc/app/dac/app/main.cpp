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

#include "drv/SAMx5x/dac.h"
#include "drv/SAMx5x/dma.h"
#include "drv/SAMx5x/timer.h"

#include "Raumsteuerung/valueReportProtocol.h"

int main(const kernel_t *kernel);
void onReceive(const kernel_t *kernel, uint8_t sourceAddress, busProtocol_t protocol, uint8_t command, const uint8_t *data, uint8_t size);
kernel_t const *_kernel;

__attribute__((section(".appHeader"))) appHead_t appHead ={
/*appCRC	 */ 0xAABBCCDD, // Will be written by Bootload tool
/*appSize	 */ 0xEEFF0000, // Will be written by Bootload tool
/*appRevMaj	 */ 0x01,
/*appRevMin	 */ 0x00,
/*appName[60]*/ "DAC",
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
	{ 0x00, false, ((float)(0.0f)), ((float)(225.0f)), &ch[0], UOM_RAW_UINT32, "CH 1",vrp_valueChange}
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


dac_t dac;

dma_transfer_t *trns;

lfs_file_t file;

uint16_t val = 0;

#define DAC_PAGE_SIZE (256*2)

uint8_t dacData0[DAC_PAGE_SIZE];
uint8_t dacData1[DAC_PAGE_SIZE];

volatile uint8_t page;
volatile bool updatePage = false;
volatile uint32_t dacIndex;

void timerInterrupt(void)
{
	TC0->COUNT16.INTFLAG.bit.OVF = true;
	TC0->COUNT16.COUNT.reg = 0xFFFF - 60;

	uint8_t smpl;
	if(page == 0) smpl = dacData0[dacIndex];
	else smpl = dacData1[dacIndex];
	dacIndex++;
	
	dac_channel_set(&dac,dac_channel_t::DAC_0,smpl<<4);
	while(DAC->SYNCBUSY.bit.DATA0 == true);
	
	if(dacIndex >= DAC_PAGE_SIZE)
	{
		if(page == 0) page = 1;
		else page = 0;
		
		dacIndex = 0;
		
	}
	
	if(dacIndex == DAC_PAGE_SIZE /2)
	{
		updatePage = true;
	}
	
	
	
//	if(val == 0) val = 0xFFF;
//	else val = 0;
	
}



int main(const kernel_t *kernel)
{
	_kernel = kernel;
	
	// App init Code
	if(kernel->kernelSignals->initApp)
	{
		Reset_Handler();
		
		vrp_init(kernel, &valueReportProtocol);
		
		/*dma_init();
		dma_channel_init(DMA_CH00,DMA_DAC0_EMPTY,DMA_BURST_TRIGGER)
		
		trns = dma_getTransfer(DMA_CH00);
		dma_transfer_dataToPeripheral(trns, &txBuffer[0],514,&SERCOM5->USART.DATA.reg);*/
		
		//pin_enableOutput(IO_A00);
		//pin_setOutput(IO_A00, true); // Reset Low
		
		int err = kernel->file.open(&file, "dac", LFS_O_RDONLY);
		if(err != LFS_ERR_OK) return 0;
		
		kernel->file.seek(&file,0xC0,LFS_SEEK_CUR);
		
		
		dac_init(&dac, kernel->clk_16MHz, true, false);
		//dac_channel_set(&dac,dac_channel_t::DAC_0, val);
		
		TC0->COUNT16.COUNT.reg = 0xFFFF - 125;
		timer_init(TC0, kernel->clk_1MHz, TIMER_DIV1);
		
		timer_overflowInterrupt(TC0, true);
		
		kernel->nvic.assignInterruptHandler(TC0_IRQn, timerInterrupt);
		kernel->nvic.setInterruptPriority(TC0_IRQn, 6);
		
		kernel->file.read(&file, &dacData0[0], DAC_PAGE_SIZE);
		kernel->file.read(&file, &dacData1[0], DAC_PAGE_SIZE);
		page = 0;
		updatePage = false;
		dacIndex = 0;
		
		kernel->appSignals->appReady = true;
	}

	// Main code here
	if(kernel->appSignals->appReady == true)
	{
		if(updatePage == true)
		{
			if(page == 0) kernel->file.read(&file, &dacData1[0], DAC_PAGE_SIZE);
			else if(page == 1) kernel->file.read(&file, &dacData0[0], DAC_PAGE_SIZE);
			updatePage = false;
		}
		
		
		//val++;
		//dac_channel_set(&dac,dac_channel_t::DAC_0,0);
	//	while(DAC->SYNCBUSY.bit.DATA0 == true);
		
	//	dac_channel_set(&dac,dac_channel_t::DAC_0,0xFFF);
		//while(DAC->SYNCBUSY.bit.DATA0 == true);
	}
	
	// App deinit code
    if(kernel->kernelSignals->shutdownApp)
    {
		kernel->file.close(&file);
		timer_deinit(TC0);
		//kernel->eemem_write(&ememData[0]);
		kernel->appSignals->shutdownReady = true;
    }
}



