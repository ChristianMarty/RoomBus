//**********************************************************************************************************************
// FileName : sitronix_ST7565R.h
// FilePath : peripheral/
// Author   : Christian Marty
// Date		: 11.12.2024
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#ifndef SITRONIX_ST7565R_H_
#define SITRONIX_ST7565R_H_

#ifdef __cplusplus
extern "C" {
#endif


typedef enum{
	ST7565R_setUpperColumnAddress = 0x00,
	ST7565R_setLowerColumnAddress = 0x10,
	
	ST7565R_voltageRegulatorInternalResistorRatio0 = 0x20,
	ST7565R_voltageRegulatorInternalResistorRatio1 = 0x21,
	ST7565R_voltageRegulatorInternalResistorRatio2 = 0x22,
	ST7565R_voltageRegulatorInternalResistorRatio3 = 0x23,
	ST7565R_voltageRegulatorInternalResistorRatio4 = 0x24,
	ST7565R_voltageRegulatorInternalResistorRatio5 = 0x25,
	ST7565R_voltageRegulatorInternalResistorRatio6 = 0x26,
	ST7565R_voltageRegulatorInternalResistorRatio7 = 0x27,
	
	ST7565R_powerControl = 0x28,
	
	ST7565R_startLine0 = 0x40,
	
	ST7565R_electronicVolumeModeSet = 0x81,
	
	ST7565R_driverDirectionNormal = 0xA0,
	ST7565R_driverDirectionReverse = 0xA1,
	
	ST7565R_lcdBias1d9= 0xA2,
	ST7565R_lcdBias1d7= 0xA3,
	
	ST7565R_allNormal = 0xA4,
	ST7565R_allOn = 0xA5,
	
	ST7565R_displayNormal= 0xA6,
	ST7565R_displayReverse= 0xA7,
	
	ST7565R_off = 0xAE,
	ST7565R_on = 0xAF,
	
	ST7565R_indicatorOff = 0xAC,
	ST7565R_indicatorOn = 0xDC,
	
	
	
	ST7565R_page0 = 0xB0,
	ST7565R_page1 = 0xB1,
	ST7565R_page2 = 0xB2,
	ST7565R_page3 = 0xB3,
	ST7565R_page4 = 0xB4,
	ST7565R_page5 = 0xB5,
	ST7565R_page6 = 0xB6,
	ST7565R_page7 = 0xB7,
	ST7565R_page8 = 0xB8,
	
	ST7565R_commonOutputModeNormal = 0xC0,
	ST7565R_commonOutputModeReverse = 0xC8,
	
	
	
}st7565r_commands_t;

#ifdef __cplusplus
}
#endif

#endif /* SITRONIX_ST7565R_H_ */
