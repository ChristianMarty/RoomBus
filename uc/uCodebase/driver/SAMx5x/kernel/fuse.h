//**********************************************************************************************************************
// FileName : fuse.h
// FilePath : uCodebase/driver/SAMx5x/kernel
// Author   : Christian Marty
// Date		: 11.11.2018
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#ifndef FUSE_H_
#define FUSE_H_

#include "flash.h"
#include "eeprom.h"

#include "common/typedef.h"

#define NVM_USER_PAGE_ADDR  0x00804000
#define NVM_SOFTWARE_CALIBRATION_AREA_ADDR 0x00800080

// Datasheet Page 57
typedef struct fuseUserPage {
	uint32_t BOD33Disable	  :  1;
	uint32_t BOD33Level		  :  8;
	uint32_t BOD33Action	  :  2;
	uint32_t BOD33Hysteresis  :  4;
	uint32_t BOD12Calibration : 11;
	uint32_t NVMBOOT		  :  4;
	uint32_t Reserved0		  :  2;
	
	uint32_t SEESBLK		  :  4;
	uint32_t SEEPSZ			  :  3;
	uint32_t RAMECCDIS		  :  1;
	uint32_t Reserved1		  :  8;
	uint32_t WDTEnable		  :  1;
	uint32_t WDTAlwaysOn	  :  1;
	uint32_t WDTPeriod		  :  4;
	uint32_t WDTWindow		  :  4;
	uint32_t WDTEWOFFSET	  :  4;
	uint32_t WDTWEN			  :  1;
	uint32_t Reserved2		  :  1;
	
	uint32_t NVMLOCKS		  : 32;
	
	uint32_t fourthWord		  : 32;
} fuseUserPage_bits_t;


typedef union {
	fuseUserPage_bits_t bit;
	uint32_t byte[4];
} fuseUserPage_t;


// Datasheet Page 59
typedef struct fuseSoftwareCalibration {
	uint32_t AC_BIAS		  :  2;
	uint32_t ADC0_BIASCOMP	  :  3;
	uint32_t ADC0_BIASREFBUF  :  3;
	uint32_t ADC0_BIASR2R	  :  3;
	uint32_t Reserved0		  :  5;
	uint32_t ADC1_BIASCOMP    :  3;
	uint32_t ADC1_BIASREFBUF  :  3;
	uint32_t ADC1_BIASR2R	  :  3;
	uint32_t Reserved1		  :  7;
	
	uint32_t USB_TRANSN		  :  5;
	uint32_t USB_TRANSP		  :  5;
	uint32_t USB_TRIM		  :  3;
	uint32_t Reserved2		  : 19;

} fuseSoftwareCalibration_t;


static inline fuseSoftwareCalibration_t fuse_getSoftwareCalibration(void)
{
	return *(fuseSoftwareCalibration_t*)(NVM_SOFTWARE_CALIBRATION_AREA_ADDR);	
}

static inline fuseUserPage_t fuse_getUserPage(void)
{
	return *(fuseUserPage_t*)(NVM_USER_PAGE_ADDR);		
}

static inline void fuse_setUserPage(fuseUserPage_t data)
{
	flash_eraseUserPage(NVM_USER_PAGE_ADDR);
	flash_writeUserPage(NVM_USER_PAGE_ADDR, (uint8_t*)&data, sizeof(fuseUserPage_t));
}

static inline void fuse_initialize(void)
{
	fuseUserPage_t data = fuse_getUserPage();
	bool write = false;
	if(data.bit.SEEPSZ != EEPROM_SETTINGS_SEEPSZ || data.bit.SEESBLK != EEPROM_SETTINGS_SEESBLK)  write = true;

	// From data sheet; Page 58
	data.bit.BOD33Disable		= 0x01;
	data.bit.BOD33Level			= 0x1C;
	data.bit.BOD33Action		= 0x02;
	data.bit.BOD33Hysteresis	= 0x02;
	//data.bit.BOD12Calibration   = 0x535; // Factory settings - do not change
	data.bit.NVMBOOT			= 0x0F;
	//data.Reserved0		// Factory settings - do not change
	data.bit.SEESBLK			= EEPROM_SETTINGS_SEESBLK; //0x00;
	data.bit.SEEPSZ				= EEPROM_SETTINGS_SEEPSZ; //0x00;
	data.bit.RAMECCDIS			= 0x01;
	//data.Reserved1		// Factory settings - do not change
	data.bit.WDTEnable			= 0x01;
	data.bit.WDTAlwaysOn		= 0x01;
	data.bit.WDTPeriod			= 0x0B; 
	data.bit.WDTWindow			= 0x0B;
	data.bit.WDTEWOFFSET		= 0x0B;
	data.bit.WDTWEN				= 0x00;
	//data.Reserved2		// Factory settings - do not change
	data.bit.NVMLOCKS			= 0xFFFFFFFF;
	data.bit.fourthWord			= 0xFFFFFFFF;
	
	if(write) fuse_setUserPage(data);
}

// Fuse default settings
// User Word 0 : 0xFE9A9239
// User Word 1 : 0xAEECFF80
// User Word 2 : 0xFFFFFFFF

//SW0_WORD_0 = 0x1FF07FD (valid)
//SW0_WORD_1 = 0x1B29 (valid)
//TEMP_LOG_WORD_0 = 0x196881D (valid)
//TEMP_LOG_WORD_1 = 0xD55AE304 (valid)
//TEMP_LOG_WORD_2 = 0xFF931B44 (valid)
//USER_WORD_0 = 0xFE9A9238 (valid)
//USER_WORD_1 = 0xAEECFF94 (valid)
//USER_WORD_2 = 0xFFFFFFFF (valid)


static inline void fuse_restoreDefault(void)
{
	fuseUserPage_t data = fuse_getUserPage();
	
	// From data sheet; Page 58
	data.bit.BOD33Disable		= 0x01;
	data.bit.BOD33Level			= 0x1C;
	data.bit.BOD33Action		= 0x01;
	data.bit.BOD33Hysteresis	= 0x02;
	//data.bit.BOD12Calibration   = 0x535; // Factory settings - do not change
	data.bit.NVMBOOT			= 0x0F;
	//data.Reserved0		// Factory settings - do not change
	data.bit.SEESBLK			= 0x04; //0x00;
	data.bit.SEEPSZ				= 0x01; //0x00;
	data.bit.RAMECCDIS			= 0x01;
	//data.Reserved1		// Factory settings - do not change
	data.bit.WDTEnable			= 0x00;
	data.bit.WDTAlwaysOn		= 0x00;
	data.bit.WDTPeriod			= 0x0B; 
	data.bit.WDTWindow			= 0x0B;
	data.bit.WDTEWOFFSET		= 0x0B;
	data.bit.WDTWEN				= 0x00;
	//data.Reserved2		// Factory settings - do not change
	data.bit.NVMLOCKS			= 0xFFFFFFFF;
	data.bit.fourthWord			= 0xFFFFFFFF;
	
	fuse_setUserPage(data);
	
	
	/*uint8_t defaultData[32] = { 0x39, 0x92, 0x9A, 0xFE, 
								0x80, 0xFF, 0xEC, 0xAE, 
								0xFF, 0xFF, 0xFF, 0xFF, 
								0xFF, 0xFF, 0xFF, 0xFF, 
								0xFF, 0xFF, 0xFF, 0xFF, 
								0xFF, 0xFF, 0xFF, 0xFF,
								0xFF, 0xFF, 0xFF, 0xFF,
								0xFF, 0xFF, 0xFF, 0xFF };
								
	fuse_setUserPage(&defaultData[0]);
	
	//flash_eraseUserPage(NVM_USER_PAGE_ADDR);
	//flash_writeUserPage(NVM_USER_PAGE_ADDR, &defaultData[0], sizeof(defaultData));*/
}

#endif /* FUSE_H_ */