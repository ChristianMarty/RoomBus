/*
 * dma.c
 *
 * Created: 14.02.2019 23:53:32
 *  Author: Christian
 */ 

#include "dma.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

dma_transfer_t  dma_active_transfer[32] __attribute__((aligned (16)));
dma_transfer_t  dma_transfer[32] __attribute__((aligned (16)));

void dma_init(void)
{
	DMAC->BASEADDR.reg = (uint32_t)&dma_transfer;
	DMAC->WRBADDR.reg = (uint32_t)&dma_active_transfer;
	
	DMAC->CTRL.reg = 0x0F02; // Enable
}

void dma_trigger(dma_channel_t channel)
{
	DMAC->SWTRIGCTRL.reg |= (1<<channel);
}

bool dma_isBusy(dma_channel_t channel)
{
	return DMAC->Channel[channel].CHSTATUS.bit.BUSY;
}

void dma_channel_init(dma_channel_t channel, dma_trigger_t trigger, dma_triggerAction_t action)
{
	// reset channel
	DMAC->Channel[channel].CHCTRLA.bit.ENABLE = false;
	DMAC->Channel[channel].CHCTRLA.bit.SWRST = true;
	
	while(DMAC->Channel[channel].CHCTRLA.bit.SWRST);
	
	DMAC->Channel[channel].CHCTRLA.bit.TRIGSRC = trigger;
	DMAC->Channel[channel].CHCTRLA.bit.TRIGACT = action;
	
	DMAC->Channel[channel].CHPRILVL.bit.PRILVL = 0; 
	
	//DMAC->Channel[channel].CHINTENSET.bit.TCMPL = true;
	
	DMAC->Channel[channel].CHCTRLA.bit.ENABLE = true;
}

void dma_channel_clearTransferComplete(dma_channel_t channel)
{
	DMAC->Channel[channel].CHINTFLAG.bit.TCMPL = true;
}

dma_transfer_t *dma_getTransfer( dma_channel_t channel)
{
	return &dma_transfer[channel];
}

void dma_transfer_dataToPeripheral(dma_transfer_t *transfer, uint8_t *data, uint16_t size, volatile uint32_t *destination)
{
	transfer->source = (void*)(data+size);
	transfer->destination = (void*)(destination);
	transfer->count.bit.BTCNT = size;
	transfer->next_transfer = 0;
	transfer->control.bit.STEPSEL = 1;
	transfer->control.bit.DSTINC = false;
	transfer->control.bit.SRCINC = true;
	transfer->control.bit.VALID = true;
}

#ifdef __cplusplus
}
#endif