
#include "driver/SAMx5x/pin.h"
#include "common/io_same51jx.h"
#include "common/kernel.h"

#include "midiModul.h"

midi_t midi_1;
midi_t midi_2;

void midi_1_onRx(void)
{
	midi_rxInterrupt(&midi_1);
}

void midi_1_onTx(void)
{
	midi_txInterrupt(&midi_1);
}

void midi_2_onRx(void)
{
	midi_rxInterrupt(&midi_2);
}

void midi_2_onTx(void)
{
	midi_txInterrupt(&midi_2);
}

void midiModul_init(const midiModul_t *midiModul)
{
	midi_init(&midi_1, SERCOM5);
	midi_init(&midi_2, SERCOM2);
	
	midi_1.controllerChange = midiModul->controllerChange_1;
	midi_1.controllerChangeSize = midiModul->controllerChangeSize_1;
	
	midi_2.controllerChange = midiModul->controllerChange_2;
	midi_2.controllerChangeSize = midiModul->controllerChangeSize_2;
	
	pin_enablePeripheralMux(IO_D12, PIN_FUNCTION_C);
	pin_enablePeripheralMux(IO_D13, PIN_FUNCTION_C);
	kernel.nvic.assignInterruptHandler(SERCOM5_2_IRQn,midi_1_onRx);
	kernel.nvic.assignInterruptHandler(SERCOM5_1_IRQn,midi_1_onTx);
	
	pin_enablePeripheralMux(IO_D04, PIN_FUNCTION_C);
	pin_enablePeripheralMux(IO_D05, PIN_FUNCTION_C);
	kernel.nvic.assignInterruptHandler(SERCOM2_2_IRQn,midi_2_onRx);
	kernel.nvic.assignInterruptHandler(SERCOM2_1_IRQn,midi_2_onTx);
	
	pin_enableOutput(IO_D17);
	pin_setOutput(IO_D17, true);	
}

void midiModul_handler(void)
{
	midi_handler(&midi_1);
	midi_handler(&midi_2);	
}

void midiModul_sendControllerChange(midiModul_output_t output, uint8_t channel, uint8_t command, uint8_t value)
{
	switch(output){
		case midiModul_output_1: midi_sendControllerChange(&midi_1, channel, command, value); break;
		case midiModul_output_2: midi_sendControllerChange(&midi_2, channel, command, value); break;
	}
	
}