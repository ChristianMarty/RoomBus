
#ifndef MIDI_MODUL_H_
#define MIDI_MODUL_H_

#include "interface/midi.h"

typedef struct{
	midi_controllerChange_t *controllerChange_1;
	uint8_t controllerChangeSize_1;
	
	midi_controllerChange_t *controllerChange_2;
	uint8_t controllerChangeSize_2;
}midiModul_t;


typedef enum { 
	midiModul_output_1,
	midiModul_output_2
}midiModul_output_t;


void midiModul_init(const midiModul_t *midiModul);

void midiModul_handler(void);

void midiModul_sendControllerChange(midiModul_output_t output, uint8_t channel, uint8_t command, uint8_t value);


#endif /* MIDI_MODUL_H_ */