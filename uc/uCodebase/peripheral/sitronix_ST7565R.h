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
	ST7565R_voltageRegulatorInternalResistorRatio = 0x20,
	
	ST7565R_powerControl = 0x28,
	
	ST7565R_line0 = 0x40,
	
	ST7565R_electronicVolumeMode = 0x81,
	
	ST7565R_adcSelectNormal = 0xA0,
	ST7565R_adcSelectReverse = 0xA1,
	
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
	
	
	
	
}st7565r_commands_t;

#ifdef __cplusplus
}
#endif

#endif /* SITRONIX_ST7565R_H_ */
