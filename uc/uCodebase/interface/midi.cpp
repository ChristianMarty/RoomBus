
#include "midi.h"


void midi_init(midi_t *midi)
{
	midi->uartCom.initUart(kernel.clk_16MHz, midi->sercom_p, 31250, uart_c::none);
}

void midi_handler(midi_t *midi)
{
	if((midi->rxIndex == midi->rxMsgSize) && (midi->rxIndex!= 0))
	{
		for(uint8_t i= 0; i< midi->controllerChangeSize; i++)
		{
			if(midi->controllerChange[i].command == midi->rxData[1])
			{
				*midi->controllerChange[i].value = midi->rxData[2];
				midi->controllerChange[i].controllerChange(i);
			}
		}
		
		midi->rxIndex = 0;
	}
}

bool midi_sendControllerChange(midi_t *midi, uint8_t channel, uint8_t command, uint8_t value)
{
	if(midi->uartCom.txBusy()) return false;
	
	midi->txData[0] = (0xB0 | (channel&0x0F));
	midi->txData[1] = (command&0x7F);
	midi->txData[2] = (value&0x7F);
	
	midi->uartCom.sendData(&midi->txData[0],3);
	
	return true;
}

void midi_onRx(midi_t *midi)
{
	uint8_t data = midi->uartCom.RxInterrupt();
	
	if(data & 0x80) 
	{
		midi->rxIndex = 0;
		if((data & 0xF0) == 0xB0) midi->rxMsgSize=3;
	}
	midi->rxData[midi->rxIndex] = data;
	midi->rxIndex++;	
}

void midi_onTx(midi_t *midi)
{
	midi->uartCom.TxInterrupt();
}



