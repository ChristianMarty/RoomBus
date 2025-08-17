//**********************************************************************************************************************
// FileName : qspi.c
// FilePath : uCodebase/driver/SAMx5x/kernel/
// Author   : Christian Marty
// Date		: 14.02.2019
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#ifdef __cplusplus
extern "C" {
#endif

#include "qspi.h"

#include "../pin.h"
#include "../genericClockController.h"

#define EXTFLASH __attribute__((section(".extFlash")))

typedef struct{
	uint8_t WIP:1;
	uint8_t WEL:1;
	uint8_t BP0:1;
	uint8_t BP1:1;
	uint8_t BP2:1;
	uint8_t BP3:1;
	uint8_t QE:1;
	uint8_t SRWD:1;
}qspi_flash_status_t;


#define SINGLE_BIT_SPI 0x00 // Instruction: Single-bit SPI / Address-Option: Single-bit SPI / Data: Single-bit SPI
#define DUAL_OUTPUT 0x01	// Instruction: Single-bit SPI / Address-Option: Single-bit SPI / Data: Dual SPI
#define QUAD_OUTPUT 0x02	// Instruction: Single-bit SPI / Address-Option: Single-bit SPI / Data: Quad SPI
#define DUAL_IO 0x03		// Instruction: Single-bit SPI / Address-Option: Dual SPI / Data: Dual SPI
#define QUAD_IO 0x04		// Instruction: Single-bit SPI / Address-Option: Quad SPI / Data: Quad SPI
#define DUAL_CMD 0x05		// Instruction: Dual SPI / Address-Option: Dual SPI / Data: Dual SPI
#define QUAD_CMD 0x06		// Instruction: Quad SPI / Address-Option: Quad SPI / Data: Quad SPI

#define TFRTYPE_READ 0x00
#define TFRTYPE_READMEMORY 0x01
#define TFRTYPE_WRITE 0x02
#define TFRTYPE_WRITEMEMORY 0x03

qspi_error_t qspi_init(void)
{
	pin_enablePeripheralMux(PIN_PORT_A,8,PIN_FUNCTION_H);
	pin_enablePeripheralMux(PIN_PORT_A,9,PIN_FUNCTION_H);
	pin_enablePeripheralMux(PIN_PORT_A,10,PIN_FUNCTION_H);
	pin_enablePeripheralMux(PIN_PORT_A,11,PIN_FUNCTION_H);
	pin_enablePeripheralMux(PIN_PORT_B,10,PIN_FUNCTION_H);
	pin_enablePeripheralMux(PIN_PORT_B,11,PIN_FUNCTION_H);
	
	pin_enableStrongDriver(PIN_PORT_A,8);
	pin_enableStrongDriver(PIN_PORT_A,9);
	pin_enableStrongDriver(PIN_PORT_A,10);
	pin_enableStrongDriver(PIN_PORT_A,11);
	pin_enableStrongDriver(PIN_PORT_B,10);
	pin_enableStrongDriver(PIN_PORT_B,11);
		
	PORT->Group[0].DIRSET.reg = 0xF00;
	PORT->Group[1].DIRSET.reg = 0xC00;
	
	QSPI->CTRLA.bit.SWRST = true; // reset QSPI Peripheral

	QSPI->BAUD.bit.BAUD = 0xFF;
	QSPI->BAUD.bit.CPOL = 0;
	QSPI->BAUD.bit.CPHA = 0;
	
	QSPI->CTRLB.bit.MODE = 0x01; // set QSPI mode
	
	return qspi_noError;
}

qspi_error_t qspi_resetDevice(void)
{
	QSPI_INSTRFRAME_Type instruction;
	
	instruction.bit.INSTREN = 1;
	instruction.bit.ADDREN = 0;
	instruction.bit.DATAEN = 0;
	instruction.bit.WIDTH = SINGLE_BIT_SPI;
	instruction.bit.DUMMYLEN = 0x00;
	instruction.bit.TFRTYPE = TFRTYPE_WRITE;
	
	QSPI->CTRLA.bit.ENABLE = true;
		
	// Enable Reset
	QSPI->INSTRCTRL.reg = 0x66;
	QSPI->INTFLAG.reg = 0x508;
	QSPI->INSTRFRAME.reg = instruction.reg;
	while( !QSPI->INTFLAG.bit.CSRISE);
	
	// Reset
	QSPI->INSTRCTRL.reg = 0x99;
	QSPI->INTFLAG.reg = 0x508;
	QSPI->INSTRFRAME.reg = instruction.reg;
	QSPI->CTRLA.bit.LASTXFER = true;
	while( !QSPI->INTFLAG.bit.CSRISE);
	
	return qspi_noError;
}

qspi_error_t qspi_writeEnable(void)
{
	QSPI_INSTRFRAME_Type instruction;
	
	instruction.bit.INSTREN = 1;
	instruction.bit.ADDREN = 0;
	instruction.bit.DATAEN = 0;
	instruction.bit.WIDTH = SINGLE_BIT_SPI;
	instruction.bit.DUMMYLEN = 0x00;
	instruction.bit.TFRTYPE = TFRTYPE_WRITE;
	
	QSPI->CTRLA.bit.ENABLE = true;
		
	QSPI->INSTRCTRL.reg = 0x06;
	QSPI->INTFLAG.reg = 0x508;
	QSPI->INSTRFRAME.reg = instruction.reg;
	QSPI->CTRLA.bit.LASTXFER = true;
	
	while( !QSPI->INTFLAG.bit.CSRISE);
	
	return qspi_noError;
}

qspi_error_t qspi_writeStatus2(uint8_t status)
{
	qspi_writeEnable();
	
	QSPI_INSTRFRAME_Type instruction;
	
	instruction.bit.INSTREN = 1;
	instruction.bit.ADDREN = 0;
	instruction.bit.DATAEN = 1;
	instruction.bit.WIDTH = SINGLE_BIT_SPI;
	instruction.bit.DUMMYLEN = 0x00;
	instruction.bit.TFRTYPE = TFRTYPE_WRITE;
	
	QSPI->CTRLA.bit.ENABLE = true;
	
	QSPI->INSTRCTRL.reg = 0x31;
	QSPI->INTFLAG.reg = 0x508;
	QSPI->INSTRFRAME.reg = instruction.reg;
	
	*(uint8_t*)(QSPI_START_ADDRESS) = status;
	
	QSPI->CTRLA.bit.LASTXFER = true;
	
	while( !QSPI->INTFLAG.bit.CSRISE);
	
	return qspi_noError;
}

uint8_t qspi_readStatus1(void)
{
	QSPI_INSTRFRAME_Type instruction;
	
	instruction.bit.INSTREN = 1;
	instruction.bit.ADDREN = 0;
	instruction.bit.DATAEN = 1;
	instruction.bit.WIDTH = SINGLE_BIT_SPI;
	instruction.bit.DUMMYLEN = 0x00;
	instruction.bit.TFRTYPE = TFRTYPE_READ;
	
	QSPI->CTRLA.bit.ENABLE = true;
	
	QSPI->INSTRCTRL.reg = 0x05;
	QSPI->INTFLAG.reg = 0x508;
	QSPI->INSTRFRAME.reg = instruction.reg;
	
	uint8_t temp = *(uint8_t*)(QSPI_START_ADDRESS);
	
	QSPI->CTRLA.bit.LASTXFER = true;
	
	while( !QSPI->INTFLAG.bit.CSRISE);
	
	return temp;
}

uint8_t qspi_readStatus2(void)
{	
	QSPI_INSTRFRAME_Type instruction;
		
	instruction.bit.INSTREN = 1;
	instruction.bit.ADDREN = 0;
	instruction.bit.DATAEN = 1;
	instruction.bit.WIDTH = SINGLE_BIT_SPI;
	instruction.bit.DUMMYLEN = 0x00;
	instruction.bit.TFRTYPE = TFRTYPE_READ;
		
	QSPI->CTRLA.bit.ENABLE = true;
		
	QSPI->INSTRCTRL.reg = 0x35;
	QSPI->INTFLAG.reg = 0x508;
	QSPI->INSTRFRAME.reg = instruction.reg;
		
	uint8_t temp = *(uint8_t*)(QSPI_START_ADDRESS);
		
	QSPI->CTRLA.bit.LASTXFER = true;
		
	while( !QSPI->INTFLAG.bit.CSRISE);
		
	return temp;
}

bool qspi_busy(void)
{
	uint8_t status = qspi_readStatus1();
	if(status & 0x01) return true;
	else return false;
}

qspi_error_t qspi_write(uint8_t *address, const uint8_t *data, uint32_t size)
{
	qspi_writeEnable();
	
	QSPI_INSTRFRAME_Type instruction;
	
	instruction.bit.DATAEN = 1;
	instruction.bit.INSTREN = 1;
	instruction.bit.ADDREN = 1;
	instruction.bit.WIDTH = QUAD_OUTPUT;
	instruction.bit.DUMMYLEN = 0x00; 
	instruction.bit.TFRTYPE = TFRTYPE_WRITEMEMORY; // Write Memory transfer
		
	QSPI->CTRLA.bit.ENABLE = true;
	
	QSPI->INSTRCTRL.reg = 0x32;
	QSPI->INTFLAG.reg = 0x508;
	QSPI->INSTRFRAME.reg = instruction.reg;
	
	for(uint32_t i = 0; i<size; i++)
	{
		address[i] = data[i];
	}
	
	QSPI->CTRLA.bit.LASTXFER = true;
	
	while( !QSPI->INTFLAG.bit.CSRISE);
	
	return qspi_noError;
} 

qspi_error_t qspi_read(uint8_t *address, uint8_t *data, uint32_t size)
{
	QSPI_INSTRFRAME_Type instruction;
	
	instruction.bit.DATAEN = 1;
	instruction.bit.INSTREN = 1;
	instruction.bit.OPTCODEEN = 1;
	instruction.bit.ADDREN = 1;
	instruction.bit.WIDTH = QUAD_IO; 
	instruction.bit.DUMMYLEN = 0x05; 
	instruction.bit.TFRTYPE = TFRTYPE_READMEMORY; // Read Memory transfer
	
	QSPI->CTRLA.bit.ENABLE = true;
	
	QSPI->INSTRCTRL.bit.INSTR = 0xEB;
	QSPI->INSTRCTRL.bit.OPTCODE = 0xFF;
	QSPI->INTFLAG.reg = 0x508;	
	QSPI->INSTRFRAME.reg = instruction.reg;
	
	for(uint32_t i = 0; i<size; i++)
	{
		data[i] = address[i];
	}
	
	QSPI->CTRLA.bit.LASTXFER = true;
	
	while( !QSPI->INTFLAG.bit.CSRISE);
	
	return qspi_noError;
}

qspi_error_t qspi_sectorErase(uint8_t *address)
{
	qspi_writeEnable();
	
	QSPI->CTRLA.bit.ENABLE = true;
	QSPI->INSTRCTRL.reg =  0x20;
	
	QSPI_INSTRFRAME_Type instruction;
	
	instruction.bit.INSTREN = 1;
	instruction.bit.ADDREN = 1;
	instruction.bit.DATAEN = 0;
	instruction.bit.WIDTH = SINGLE_BIT_SPI;
	instruction.bit.DUMMYLEN = 0x00; 
	instruction.bit.TFRTYPE = TFRTYPE_WRITE; 
	//address[0] = 0xFF;
	QSPI->INSTRADDR.reg = (uint32_t)(address)-QSPI_START_ADDRESS;
	QSPI->INSTRFRAME.reg = instruction.reg;

	QSPI->INTFLAG.reg = 0x508;
	QSPI->CTRLA.bit.LASTXFER = true;

	while( !QSPI->INTFLAG.bit.CSRISE);
	
	return qspi_noError;
}

qspi_error_t qspi_chipErase(void)
{
	qspi_writeEnable();
	
	QSPI_INSTRFRAME_Type instruction;
	
	instruction.bit.INSTREN = 1;
	instruction.bit.ADDREN = 0;
	instruction.bit.DATAEN = 0;
	instruction.bit.WIDTH = SINGLE_BIT_SPI;
	instruction.bit.DUMMYLEN = 0x00;
	instruction.bit.TFRTYPE = TFRTYPE_WRITE;
	
	QSPI->CTRLA.bit.ENABLE = true;
		
	QSPI->INSTRCTRL.reg = 0x60;
	QSPI->INTFLAG.reg = 0x508;
	QSPI->INSTRFRAME.reg = instruction.reg;
	QSPI->CTRLA.bit.LASTXFER = true;
	
	while( !QSPI->INTFLAG.bit.CSRISE);
	
	return qspi_noError;
}

uint16_t qspi_readJedecId(void)
{
	QSPI_INSTRFRAME_Type instruction;
	
	instruction.bit.INSTREN = 1;
	instruction.bit.ADDREN = 0;
	instruction.bit.DATAEN = 1;
	instruction.bit.WIDTH = SINGLE_BIT_SPI;
	instruction.bit.DUMMYLEN = 0x00;
	instruction.bit.TFRTYPE = TFRTYPE_READ;
	
	QSPI->CTRLA.bit.ENABLE = true;
		
	QSPI->INSTRCTRL.reg = 0x9F;
	QSPI->INTFLAG.reg = 0x508;
	QSPI->INSTRFRAME.reg = instruction.reg;
	
	uint32_t temp = *(uint32_t*)(QSPI_START_ADDRESS);
	
	QSPI->CTRLA.bit.LASTXFER = true;
	
	while( !QSPI->INTFLAG.bit.CSRISE);
	
	return (uint16_t)(temp>>8);
}

#ifdef __cplusplus
}
#endif