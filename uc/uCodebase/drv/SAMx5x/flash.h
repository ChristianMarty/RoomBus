/*
 * Flash.h
 *
 * Created: 11.11.2018 18:31:02
 *  Author: Christian
 */ 


#ifndef FLASH_H_
#define FLASH_H_

#include "sam.h"
#include <stdbool.h>

#define FLASH_KEY 0xA5
#define FLASH_ERASE_BLOCK_CMD 0x01
#define FLASH_WRITE_PAGE_CMD 0x03
#define FLASH_WRITE_QUAD_WORD_CMD 0x04
#define FLASH_SWRST 0x10
#define FLASH_PAGE_BUFFER_CLEAR 0x15
#define FLASH_SBPDIS 0x1A

#define FLSH_WRITE_COMMAND(command) NVMCTRL->CTRLB.reg  = (NVMCTRL_CTRLB_CMDEX_KEY|command)

#define NVMCTRL_REGIONS_NUM 32

#define NVM_MEMORY ((volatile uint32_t *)FLASH_ADDR)

typedef enum{
	FLASH_NO_ERROR,
	FLASH_ADDRESS_OUT_OF_RANGE,
	FLSHE_ADDRESS_UNALIGNED,
	FLASH_LOCKED,
	
	FLASH_ERROR_SIZE
}flash_error_t;

#ifdef __cplusplus
extern "C" {
	#endif
	
	static inline bool flash_isReady(void)
	{
		return NVMCTRL->STATUS.bit.READY;
	}
	
	static inline void flash_reset(void)
	{
		FLSH_WRITE_COMMAND(FLASH_SWRST);
		while(!flash_isReady());
		
		FLSH_WRITE_COMMAND(FLASH_SBPDIS);
		
		while(!flash_isReady());
		
		NVMCTRL->CTRLA.bit.SUSPEN = 1;
		
	}
	
	static inline uint32_t flash_getBlockSize(void)
	{
		return NVMCTRL_BLOCK_SIZE;
	}
	
	static inline uint32_t flash_getPageSize(void)
	{
		return NVMCTRL_PAGE_SIZE;
	}

	static inline bool flash_isLocked(uint32_t address)
	{
		if(address >= FLASH_SIZE) return FLASH_ADDRESS_OUT_OF_RANGE;
		
		uint8_t region_id;

		region_id = address / (FLASH_SIZE / NVMCTRL_REGIONS_NUM);
		
		return !(NVMCTRL->RUNLOCK.reg & (1 << region_id));
	}
	
	
	
	static inline flash_error_t flash_eraseBlock(uint32_t address)
	{
		NVMCTRL->INTFLAG.reg |= 1;
		if(address >= FLASH_SIZE) return FLASH_ADDRESS_OUT_OF_RANGE;
		if(flash_isLocked(address)) return FLASH_LOCKED;
		
		while(!flash_isReady());
		
		NVMCTRL->ADDR.reg = address;
		
		FLSH_WRITE_COMMAND(NVMCTRL_CTRLB_CMD_EB);
		
		while(!flash_isReady());
		NVMCTRL->INTFLAG.reg |= 1;
		
		return FLASH_NO_ERROR;
	}
	
	static inline flash_error_t drv_flash_eraseUserPage(uint32_t address)
	{
		while(!flash_isReady());

		NVMCTRL->ADDR.reg = address;
		FLSH_WRITE_COMMAND(NVMCTRL_CTRLB_CMD_EP);
		
		return FLASH_NO_ERROR;
	}
	
	static inline flash_error_t flash_write(uint32_t address, uint8_t *data, uint32_t size)
	{
		NVMCTRL->INTFLAG.reg = 0x0FF; // clear all flags
		if(address >= FLASH_SIZE) return FLASH_ADDRESS_OUT_OF_RANGE;
		if(flash_isLocked(address)) return FLASH_LOCKED;
		if((address % 4) != 0) return FLSHE_ADDRESS_UNALIGNED;
		
		uint32_t *ptr_read    = (uint32_t *)data;
		uint32_t  nvm_address = address / 4;
		
		while(!flash_isReady());
		
		FLSH_WRITE_COMMAND(NVMCTRL_CTRLB_CMD_PBC);
		
		while(!flash_isReady());

		/* Writes to the page buffer. Must be 32 bits, perform manual copy to ensure alignment */
		for(uint32_t i = 0; i < size; i += 4)
		{
			NVM_MEMORY[nvm_address++] = *ptr_read;
			ptr_read++;
		}

		while(!flash_isReady());
		
		NVMCTRL->ADDR.reg = address;
		FLSH_WRITE_COMMAND(NVMCTRL_CTRLB_CMD_WP);
		
		return FLASH_NO_ERROR;
	}
	
	static inline flash_error_t flash_read(uint32_t address, uint8_t *data, uint32_t size)
	{
		if(address >= FLASH_SIZE) return FLASH_ADDRESS_OUT_OF_RANGE;
		
		uint8_t *nvm_addr = (uint8_t *)NVM_MEMORY;
		uint32_t i;

		while(!flash_isReady());

		for (i = 0; i < size; i++)
		{
			data[i] = nvm_addr[address + i];
		}
		
		return FLASH_NO_ERROR;
	}
	
	static inline void flash_writeQuadWord(uint32_t address, uint32_t *data)
	{
		
		while(!NVMCTRL->STATUS.bit.READY);
		
		FLSH_WRITE_COMMAND(NVMCTRL_CTRLB_CMD_PBC);
		
		while(!NVMCTRL->STATUS.bit.READY);
		
		for(uint8_t i = 0; i < 4; i++)
		{
			//*(temp+i) = data[i];
			NVM_MEMORY[(address/4)+i] = data[i];
		}
		
		while(!NVMCTRL->STATUS.bit.READY);
		
		//NVMCTRL->ADDR.bit.ADDR = address/4;
		FLSH_WRITE_COMMAND(NVMCTRL_CTRLB_CMD_WP);
		
		while(!NVMCTRL->STATUS.bit.READY);
	}
	
	
	
	static inline void flash_writeWord(uint32_t address, uint32_t data)
	{
		NVMCTRL->INTFLAG.reg = 0xFFFF;
		NVMCTRL->CTRLB.reg = ((FLASH_KEY<<8)|FLASH_PAGE_BUFFER_CLEAR);
		while(!NVMCTRL->STATUS.bit.READY);
		
		uint32_t *temp = (uint32_t *)(address);
		*temp = data;
		// copied from ASF
	//	NVM_MEMORY[address / 4] = data;

		NVMCTRL->CTRLB.reg = ((FLASH_KEY<<8)|FLASH_WRITE_PAGE_CMD);
		while(!NVMCTRL->STATUS.bit.READY);

	}
	
	static inline void flash_writePage(uint32_t address, uint8_t *data, uint32_t size)
	{
		if(size > flash_getPageSize()) return;
		
		NVMCTRL->INTFLAG.reg = 0x7FF; // Clear interrupt flags
		NVMCTRL->CTRLA.bit.WMODE = 0x03; // Set write mode to Auto 
		
		while(!NVMCTRL->STATUS.bit.READY);
		FLSH_WRITE_COMMAND(NVMCTRL_CTRLB_CMD_PBC);
		while(!NVMCTRL->STATUS.bit.READY);
		
		uint32_t *wordData = (uint32_t*)data;
		uint32_t wordAddr = address/4;
		
		for(uint32_t i = 0; i < size; i += 4)
		{
			NVM_MEMORY[wordAddr++] = *(wordData++);
		}
		
		while(!NVMCTRL->STATUS.bit.READY);
		//NVMCTRL->ADDR.bit.ADDR = address;
		FLSH_WRITE_COMMAND(NVMCTRL_CTRLB_CMD_WP);
		while(!NVMCTRL->STATUS.bit.READY);
	}
	
	static inline flash_error_t flash_eraseUserPage(uint32_t address)
	{
		while(!flash_isReady());

		NVMCTRL->ADDR.reg = address;
		FLSH_WRITE_COMMAND(NVMCTRL_CTRLB_CMD_EP);
		
		return FLASH_NO_ERROR;
	}

	static inline flash_error_t flash_writeUserPage(uint32_t address, uint8_t *data, uint32_t size)
	{
		//if(address >= FLASH_SIZE) return FLASH_ADDRESS_OUT_OF_RANGE;
		if((address % 16) != 0) return FLSHE_ADDRESS_UNALIGNED;
		
		uint32_t *ptr_read    = (uint32_t *)data;
		uint32_t  nvm_address = address / 4;
		
		while(!flash_isReady());
		
		FLSH_WRITE_COMMAND(NVMCTRL_CTRLB_CMD_PBC);
		
		while(!flash_isReady());

		/* Writes to the page buffer must be 32 bits, perform manual copy to ensure alignment */
		for(uint32_t i = 0; i < size; i += 4)
		{
			NVM_MEMORY[nvm_address++] = *ptr_read;
			ptr_read++;
		}

		while(!flash_isReady());
		
		NVMCTRL->ADDR.reg = address;
		FLSH_WRITE_COMMAND(NVMCTRL_CTRLB_CMD_WQW);
		
		return FLASH_NO_ERROR;
		
	}
	
	#ifdef __cplusplus
}
#endif

#endif /* FLASH_H_ */