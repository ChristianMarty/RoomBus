/*
 * eeprom.h
 *
 * Created: 16.10.2018 23:39:33
 *  Author: Christian
 */ 


#ifndef EEPROM_SETTINGS_SEEPSZ
	// SmartEEPROM Virtual Size: 512 Byte
	#define EEPROM_SETTINGS_SEEPSZ  1
	#define EEPROM_SETTINGS_SEESBLK 4
#endif

#ifndef EEPROM_H_
#define EEPROM_H_

#define EEPROM __attribute__((section(".eemem")))

#include "sam.h"
#include "fuse.h"

#ifdef __cplusplus
	extern "C" {
#endif

#define FLASH_KEY 0xA5
#define FLASH_USEER_CMD 0x37

static inline void eeprom_init(void)
{
	// Moved to fuse.h
	/* 
	fuseUserPage_t userPage = fuse_getUserPage();
	
	// If smartEEPROM is not enabled -> enable smartEEPRM
	if(userPage.bit.SEEPSZ != EEPROM_SETTINGS_SEEPSZ || userPage.bit.SEESBLK != EEPROM_SETTINGS_SEESBLK)
	{
		userPage.bit.SEEPSZ = EEPROM_SETTINGS_SEEPSZ;
		userPage.bit.SEESBLK = EEPROM_SETTINGS_SEESBLK;
		
		fuse_setUserPage(userPage);
	}//*/
}

static inline void eeprom_writeByte(uint8_t data, volatile uint8_t *address)
{
	while (NVMCTRL->SEESTAT.bit.BUSY);
	*address = data;
	while (NVMCTRL->SEESTAT.bit.BUSY);
}

static inline uint8_t eeporm_readByte(volatile uint8_t *address)
{
	while (NVMCTRL->SEESTAT.bit.BUSY);
	return *address;
}

static inline void eeprom_writeWord(uint16_t data, volatile uint16_t *address)
{
	while (NVMCTRL->SEESTAT.bit.BUSY);
	*address = data;
}

static inline uint16_t eeporm_readWord(volatile uint16_t *address)
{
	while (NVMCTRL->SEESTAT.bit.BUSY);
	return *address;
}

static inline void eeprom_readArray(uint8_t *destAddress, volatile uint8_t *eepormAddress, uint16_t size)
{
	if(size > FLASH_PAGE_SIZE) size = FLASH_PAGE_SIZE;
	
	for (uint8_t i = 0; i<size;i++)
	{
		destAddress[i] = eeporm_readByte(eepormAddress+i);
	}
}

static inline void eeprom_writeArray(uint8_t *sourceAddress, volatile uint8_t *eepormAddress, uint16_t size)
{
	NVMCTRL->CTRLB.reg = ((FLASH_KEY<<8)|FLASH_USEER_CMD);
	NVMCTRL->SEECFG.bit.WMODE =1;
	
	if(size > FLASH_PAGE_SIZE) size = FLASH_PAGE_SIZE;
	
	for (uint8_t i = 0; i<size;i++)
	{
		eeprom_writeByte(sourceAddress[i], &eepormAddress[i]);
	}
}


#ifdef __cplusplus
	}
#endif

#endif /* EEPROM_H_ */