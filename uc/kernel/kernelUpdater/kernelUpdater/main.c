/*
 * kernelUpdater.c
 *
 * Created: 21.06.2021 22:10:47
 * Author : Christian
 */ 


#include "sam.h"
#include "driver/SAMx5x/kernel/watchDogTimer.h"
#include "driver/SAMx5x/kernel/flash.h"

#define KERNEL_START_ADDRESS 0x00000000
#define NEW_KERNEL_START_ADDRESS  ((FLASH_SIZE/2) + (KERNEL_START_ADDRESS/2))
#define NEW_KERNEL_LENGTH_OFFSET 0x10

int main(void);

typedef struct {
	uint32_t appCRC;
	uint32_t appSize;
	uint8_t appRevMaj;
	uint8_t appRevMin;
	uint8_t appName[60];
	void* appRun;
	void* onRx;
}appHead_t;

__attribute__((section(".appHeader"))) appHead_t appHead ={
	/*appCRC	 */ 0xAABBCCDD, // Will be written by Bootload tool
	/*appSize	 */ 0xEEFF0000, // Will be written by Bootload tool
	/*appRevMaj	 */ 0x01,
	/*appRevMin	 */ 0x00,
	/*appName[60]*/ "Kernel updater",
	/*main		 */ main,
	/*onRx		 */ 0
};

typedef struct {
	uint32_t kernelSectionEndAddress;
	uint32_t kernelDataStartAddress;
	uint32_t kernelDataSize;
}kernelHead_t;

__attribute__((section(".kernelHeader"))) kernelHead_t kernelHead ={
	/*kernelSectionEndAddress*/ 0x0000FFFF,
	/*kernelDataStartAddress */ 0x00010C00,
	/*kernelDataSize	     */ 0xABABABAB // Will be written by Bootload tool
};

int main(void)
{
	watchDogTimer_off();
	watchDogTimer_clear();
	__disable_irq();
	
	uint8_t blocksToErase = kernelHead.kernelSectionEndAddress / flash_getBlockSize();
	
	// Erase old kernel
	for(uint32_t i = 0; i<=blocksToErase; i++){
		flash_eraseBlock((i*flash_getBlockSize()) );
	}

	// Copy new kernel
	for(uint32_t i= 0; i< kernelHead.kernelDataSize; i+=512){
		flash_writePage((KERNEL_START_ADDRESS+i),&((uint8_t*)kernelHead.kernelDataStartAddress)[i],512);
	}
	
	__NVIC_SystemReset();
	
	while (1);
}
