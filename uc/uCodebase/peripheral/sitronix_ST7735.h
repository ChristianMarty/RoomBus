//**********************************************************************************************************************
// FileName : sitronix_ST7535.h
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


typedef enum
{
	LCD_DELAY = 0x80,
	
	LCD_NOP = 0x00,
	LCD_SWRESET = 0x01,
	LCD_RDDID = 0x04,
	LCD_RDDST = 0x09,

	LCD_SLPIN = 0x10,
	LCD_SLPOUT = 0x11,
	LCD_PTLON = 0x12,
	LCD_NORON = 0x13,

	LCD_INVOFF = 0x20,
	LCD_INVON = 0x21,
	LCD_DISPOFF = 0x28,
	LCD_DISPON = 0x29,
	LCD_RAMRD = 0x2E,
	LCD_CASET = 0x2A,
	LCD_RASET = 0x2B,
	LCD_RAMWR = 0x2C,

	LCD_PTLAR = 0x30,
	LCD_MADCTL = 0x36,
	LCD_COLMOD = 0x3A,

	LCD_FRMCTR1 = 0xB1,
	LCD_FRMCTR2 = 0xB2,
	LCD_FRMCTR3 = 0xB3,
	LCD_INVCTR = 0xB4,
	LCD_DISSET5 = 0xB6,

	LCD_PWCTR1 = 0xC0,
	LCD_PWCTR2 = 0xC1,
	LCD_PWCTR3 = 0xC2,
	LCD_PWCTR4 = 0xC3,
	LCD_PWCTR5 = 0xC4,
	LCD_VMCTR1 = 0xC5,

	LCD_RDID1 = 0xDA,
	LCD_RDID2 = 0xDB,
	LCD_RDID3 = 0xDC,
	LCD_RDID4 = 0xDD,

	LCD_GMCTRP1 = 0xE0,
	LCD_GMCTRN1 = 0xE1,

	LCD_PWCTR6 = 0xFC,
} sitronixST7535_commands_t;

void sitronixST7535_fill(void);
void sitronixST7535_init(void);

#ifdef __cplusplus
}
#endif

#endif /* SITRONIX_ST7565R_H_ */
