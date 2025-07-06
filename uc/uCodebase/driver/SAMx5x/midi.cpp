//**********************************************************************************************************************
// FileName : midi.cpp
// FilePath : driver/SAMx5x/
// Author   : Christian Marty
// Date		: 04.01.2025
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#include "midi.h"
#include "sercom.h"
#include "common/kernel.h"

#include "utility/string.h"

void midi_init(midi_t *midi, Sercom *sercom)
{
	midi->_sercom = sercom;
	midi->_runningStatus = 0;
	
	ringbufferU8_init(&midi->_rxBuffer, &midi->_rxData[0], sizeof(midi->_rxData));
	ringbufferU8_init(&midi->_txBuffer, &midi->_txData[0], sizeof(midi->_txData));
	
	sercom_initClock(kernel.clk_16MHz, sercom);
	
	sercom->USART.CTRLA.reg = 0x40120084;
	sercom->USART.CTRLB.reg = 0x00030000;
	
	sercom->USART.INTENSET.bit.RXC = true;
	sercom->USART.INTENSET.bit.TXC = true;
	
	sercom_setBaudRate(sercom,16000000, 31250);
	sercom->USART.CTRLA.bit.ENABLE = true;
}

uint8_t _midi_sizeFromCommand(midiCommand_t command)
{
	switch(command&0xF0){
		case midi_noteOff:
		case midi_noteOn:
		case midi_polyphonicKeyPressure:
		case midi_controlChange:
		case midi_pitchBendChange:
			return 3;
		case midi_programChange:
		case midi_channelPressure:
			return 2;
			
		case midi_systemCommonMessages:
			switch(command&0x0F){
				case midi_songPosition:
					return 3;
					
				case midi_songSelect:
				case midi_timeCodeQuarterFrame:
					return 2;
					
				case midi_tuneRequest:
					return 1;
					
				case midi_startOfExclusive:
				case midi_endOfExclusive:
				default:	
					return 0;
			}
		default:
			return 0;
	}
}

void _midi_messageHandler(midi_t *midi, uint8_t byte1, uint8_t byte2,  uint8_t byte3)
{
	if((byte1&0xF0) == midi_controlChange)
	{	
		for(uint8_t i= 0; i< midi->controllerChangeSize; i++)
		{
			if(midi->controllerChange[i].channel == (byte1&0x0F) && midi->controllerChange[i].command == byte2)
			{	
				*midi->controllerChange[i].value = byte3;
				midi->controllerChange[i].controllerChange(i);
			}
		}
	}
}

void midi_handler(midi_t *midi)
{
	while(uint8_t count = ringbufferU8_count(&midi->_rxBuffer)){	
		
		uint8_t statusByte = ringbufferU8_get(&midi->_rxBuffer, 0);
		
		if(((statusByte & 0x80) == 0) && (midi->_runningStatus == 0)){
			ringbufferU8_read(&midi->_rxBuffer); // remove non-commands at the beginning 
		}else{
			
			if(statusByte & 0x80){
				midi->_runningStatus = ringbufferU8_read(&midi->_rxBuffer);
			}else{
				count++; // in case of running status
			}
			
			uint8_t messageSize = _midi_sizeFromCommand((midiCommand_t)midi->_runningStatus);
			
			if(messageSize > count){
				return;
			}
			
			if(messageSize == 0){
				midi->_runningStatus = 0; // unsupported
			}else if(messageSize == 1){
				_midi_messageHandler(midi,midi->_runningStatus,0,0);
			}else if(messageSize == 2){
				_midi_messageHandler(midi,midi->_runningStatus,ringbufferU8_read(&midi->_rxBuffer),0);
			}else if(messageSize == 3){
				_midi_messageHandler(midi,midi->_runningStatus,ringbufferU8_read(&midi->_rxBuffer),ringbufferU8_read(&midi->_rxBuffer));
			}
		}
	}
}

static inline void _midi_sendByte(midi_t *midi, uint8_t data)
{
	midi->_sercom->USART.DATA.reg = data;
}

bool midi_sendControllerChange(midi_t *midi, uint8_t channel, uint8_t command, uint8_t value)
{	
	if(ringbufferU8_space(&midi->_txBuffer) < 3){
		return false;
	}
	
	ringbufferU8_write(&midi->_txBuffer, (midi_controlChange | (channel&0x0F)));
	ringbufferU8_write(&midi->_txBuffer, (command&0x7F));
	ringbufferU8_write(&midi->_txBuffer, (value&0x7F));
	
	if(midi->_sercom->USART.INTFLAG.bit.DRE){
		midi->_sercom->USART.DATA.reg = ringbufferU8_read(&midi->_txBuffer);
	}
	
	return true;
}

void midi_rxInterrupt(midi_t *midi)
{
	uint8_t data = midi->_sercom->USART.DATA.reg;
	if(!ringbufferU8_full(&midi->_rxBuffer)){
		ringbufferU8_write(&midi->_rxBuffer, data);
	}
	midi->_sercom->USART.INTFLAG.bit.RXC = true;
}

void midi_txInterrupt(midi_t *midi)
{
	if(ringbufferU8_count(&midi->_txBuffer)){
		midi->_sercom->USART.DATA.reg = ringbufferU8_read(&midi->_txBuffer);
	}else{
		midi->_sercom->USART.INTFLAG.bit.TXC= true;
	}
}
