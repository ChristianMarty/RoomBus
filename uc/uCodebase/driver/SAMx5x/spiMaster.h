
#ifndef SPI_MASTER_H_
#define SPI_MASTER_H_

#include "sam.h"
#include "drv/SAMx5x/genericClockController.h"
#include "kernel/kernel.h"

#define  I2C_MASTER_TIMEOUT 1000


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
} lcd_commands_t;

class spiMaster_c
{
	public:
	
	void init(const kernel_t *kernel, Sercom *sercom_p, gclk_generator_t clkGenerator, uint8_t clockDevisor);
	
	void handler(void);
	
	void send(uint32_t data);

	
	void interruptHandler_0(void);
	void interruptHandler_1(void);
	void interruptHandler_2(void);
	void interruptHandler_3(void);

	bool busy(void);
	
	private:

	Sercom *_sercom_p;
	const kernel_t *_kernel;
	gclk_generator_t _clkGenerator;
	uint8_t _clockDevisor;
	
	bool _busy;
	
};
#endif /* SPI_MASTER_H_ */