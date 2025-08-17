//**********************************************************************************************************************
// FileName : directMemoryAccess.c
// FilePath : uCodebase/driver/SAMx5x/
// Author   : Christian Marty
// Date		: 14.02.2019
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#ifndef DIRECT_MEMORY_ACCESS_H_
#define DIRECT_MEMORY_ACCESS_H_
#include "sam.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	DMA_DISABLE = 0x00,
	DMA_RTC_TIMESTAMP = 0x01,
	DMA_DSU_DCC0   = 0x02,
	DMA_DSU_DCC1   = 0x03,
	DMA_SERCOM0_RX = 0x04,
	DMA_SERCOM0_TX = 0x05,
	DMA_SERCOM1_RX = 0x06,
	DMA_SERCOM1_TX = 0x07,
	DMA_SERCOM2_RX = 0x08,
	DMA_SERCOM2_TX = 0x09,
	DMA_SERCOM3_RX = 0x0A,
	DMA_SERCOM3_TX = 0x0B,
	DMA_SERCOM4_RX = 0x0C,
	DMA_SERCOM4_TX = 0x0D,
	DMA_SERCOM5_RX = 0x0E,
	DMA_SERCOM5_TX = 0x0F,
	DMA_SERCOM6_RX = 0x10,
	DMA_SERCOM6_TX = 0x11,
	DMA_SERCOM7_RX = 0x12,
	DMA_SERCOM7_TX = 0x13,
	DMA_CAN0_DEBUG = 0x14,
	DMA_CAN1_DEBUG = 0x15,
	
	DMA_TCC0_OVF  = 0x16,
	DMA_TCC0_MC_0 = 0x17,
	DMA_TCC0_MC_1 = 0x18,
	DMA_TCC0_MC_2 = 0x19,
	DMA_TCC0_MC_3 = 0x1A,
	DMA_TCC0_MC_4 = 0x1B,
	DMA_TCC0_MC_5 = 0x1C,
	
	DMA_TCC1_OVF  = 0x1D,
	DMA_TCC1_MC_0 = 0x1E,
	DMA_TCC1_MC_1 = 0x1F,
	DMA_TCC1_MC_2 = 0x20,
	DMA_TCC1_MC_3 = 0x21,
	
	DMA_TCC2_OVF  = 0x22,
	DMA_TCC2_MC_0 = 0x23,
	DMA_TCC2_MC_1 = 0x24,
	DMA_TCC2_MC_2 = 0x25,
	
	DMA_TCC3_OVF  = 0x26,
	DMA_TCC3_MC_0 = 0x27,
	DMA_TCC3_MC_1 = 0x28,
	
	DMA_TCC4_OVF  = 0x29,
	DMA_TCC4_MC_0 = 0x2A,
	DMA_TCC4_MC_1 = 0x2B,

	DMA_TC0_OVF  = 0x2C,
	DMA_TC0_MC_0 = 0x2D,
	DMA_TC0_MC_1 = 0x2E,
	
	DMA_TC1_OVF  = 0x2F,
	DMA_TC1_MC_0 = 0x30,
	DMA_TC1_MC_1 = 0x31,
	
	DMA_TC2_OVF  = 0x32,
	DMA_TC2_MC_0 = 0x33,
	DMA_TC2_MC_1 = 0x34,
	
	DMA_TC3_OVF  = 0x35,
	DMA_TC3_MC_0 = 0x36,
	DMA_TC3_MC_1 = 0x37,
	
	DMA_TC4_OVF  = 0x38,
	DMA_TC4_MC_0 = 0x39,
	DMA_TC4_MC_1 = 0x3A,
	
	DMA_TC5_OVF  = 0x3B,
	DMA_TC5_MC_0 = 0x3C,
	DMA_TC5_MC_1 = 0x3D,
	
	DMA_TC6_OVF  = 0x3E,
	DMA_TC6_MC_0 = 0x3F,
	DMA_TC6_MC_1 = 0x40,
	
	DMA_TC7_OVF  = 0x41,
	DMA_TC7_MC_0 = 0x42,
	DMA_TC7_MC_1 = 0x43,
	
	DMA_ADC0_RESRDY = 0x44,
	DMA_ADC0_SEQ = 0x45,
	DMA_ADC1_RESRDY = 0x46,
	DMA_ADC1_SEQ = 0x47,
	DMA_DAC0_EMPTY = 0x48,
	DMA_DAC1_EMPTY = 0x49,
	DMA_DAC0_RESRDY = 0x4A,
	DMA_DAC1_RESRDY = 0x4B,
	DMA_I2S0_RX = 0x4C,
	DMA_I2S1_RX = 0x4D,
	DMA_I2S0_TX = 0x4E,
	DMA_I2S1_TX = 0x4F,
	
	DMA_PCC_RX = 0x50,
	DMA_AES_WR = 0x51,
	DMA_AES_RD = 0x52,
	DMA_QSPI_RX = 0x53,
	DMA_QSPI_TX = 0x54
}dma_trigger_t;

typedef enum {
	DMA_BLOCK_TRIGGER = 0,
	DMA_BURST_TRIGGER = 2,
	DMA_TRANSACTION_TRIGGER = 3
}dma_triggerAction_t;

typedef enum {
	DMA_CH00,
	DMA_CH01,
	DMA_CH02,
	DMA_CH03,
	DMA_CH04,
	DMA_CH05,
	DMA_CH06,
	DMA_CH07,
	DMA_CH08,
	DMA_CH09,
	DMA_CH10,
	DMA_CH11,
	DMA_CH12,
	DMA_CH13,
	DMA_CH14,
	DMA_CH15,
	DMA_CH16,
	DMA_CH17,
	DMA_CH18,
	DMA_CH19,
	DMA_CH20,
	DMA_CH21,
	DMA_CH22,
	DMA_CH23,
	DMA_CH24,
	DMA_CH25,
	DMA_CH26,
	DMA_CH27,
	DMA_CH28,
	DMA_CH29,
	DMA_CH30,
	DMA_CH31	
}dma_channel_t;

typedef struct {
	DMAC_BTCTRL_Type control;
	DMAC_BTCNT_Type count;
	void *source;
	void *destination;
	uint32_t *next_transfer;
	
}dma_transfer_t;


void dma_initialize(void);

void dma_trigger(dma_channel_t channel);

bool dma_isBusy(dma_channel_t channel);

void dma_channel_initialize(dma_channel_t channel, dma_trigger_t trigger, dma_triggerAction_t action);

dma_transfer_t *dma_getTransfer(dma_channel_t channel);

void dma_channel_clearTransferComplete(dma_channel_t channel);

void dma_transfer_dataToPeripheral(dma_transfer_t *transfer, uint8_t *data, uint16_t size, volatile uint32_t *destination);

#ifdef __cplusplus
}
#endif

#endif /* DIRECT_MEMORY_ACCESS_H_ */