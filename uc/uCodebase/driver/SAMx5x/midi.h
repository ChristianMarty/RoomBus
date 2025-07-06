//**********************************************************************************************************************
// FileName : midi.h
// FilePath : driver/SAMx5x/
// Author   : Christian Marty
// Date		: 04.01.2025
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#ifndef MIDI_H_
#define MIDI_H_

#include "sam.h"
#include "genericClockController.h"
#include "utility/ringbuffer_u8.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*midi_controllerChangeChange_t)(uint8_t index);

typedef struct{
	uint8_t channel;
	uint8_t command;
	uint8_t *value;
	
	midi_controllerChangeChange_t controllerChange;
}midi_controllerChange_t;


typedef enum {
	midi_noteOff = 0x80,
	midi_noteOn = 0x90,
	midi_polyphonicKeyPressure = 0xA0,
	midi_controlChange = 0xB0,
	midi_programChange = 0xC0,
	midi_channelPressure = 0xD0,
	midi_pitchBendChange = 0xE0,
	midi_systemCommonMessages = 0xF0
} midiCommand_t;

typedef enum {
	midi_startOfExclusive = 0x00,
	midi_timeCodeQuarterFrame = 0x01,
	midi_songPosition = 0x02,
	midi_songSelect = 0x03,
	midi_tuneRequest  = 0x06,
	midi_endOfExclusive = 0x07
} midi_systemCommonCommand_t;

typedef struct {
	midi_controllerChange_t *controllerChange;
	uint8_t controllerChangeSize;
	
	Sercom *_sercom;
	
	uint8_t _txData[3*10];
	ringbufferU8_t _txBuffer;
	
	uint8_t _rxData[3*10];
	ringbufferU8_t _rxBuffer;
	uint8_t _runningStatus;
} midi_t;

void midi_init(midi_t *midi, Sercom *sercom);
void midi_handler(midi_t *midi);

bool midi_sendControllerChange(midi_t *midi, uint8_t channel, uint8_t command, uint8_t value);

void midi_rxInterrupt(midi_t *midi);
void midi_txInterrupt(midi_t *midi);

#ifdef __cplusplus
}
#endif

#endif /* MIDI_H_ */