
#ifndef BISTABLE_RELAY_H_
#define BISTABLE_RELAY_H_

#include "sam.h"
#include "common/typedef.h"
#include "common/kernel.h"
#include "driver/SAMx5x/pin.h"

#define RELAIS_SWITCH_ON_TIME 125
#define RELAIS_SWITCH_OFF_TIME 125

typedef enum  {
	bsRelay_idle,
	
	bsRelay_initOn,
	bsRelay_turnOn,
	bsRelay_isOn,
	
	bsRelay_initOff,
	bsRelay_turnOff,
	bsRelay_isOff
}bsRelay_step_t;

typedef enum  {
	bsRelay_unknown,
	bsRelay_transitioning,
	bsRelay_on,
	bsRelay_off
}bsRelay_state_t;

typedef struct{
	pin_port_t onPinPortNr;
	uint8_t onPinNr;
	pin_port_t offPinPortNr;
	uint8_t offPinNr;
	
	bsRelay_step_t relayStep;
	bsRelay_state_t relayState;
	uint32_t pulsTimer;
}bsRelay_t;

void bsRelay_init(bsRelay_t *relay, pin_port_t onPortNr, uint8_t onPinNr, pin_port_t offPortNr, uint8_t offPinNr);

void bsRelay_set(bsRelay_t *relay, bool state);
void bsRelay_toggle(bsRelay_t *relay);
bsRelay_state_t bsRelay_state(bsRelay_t *relay);

void bsRelay_handler(bsRelay_t *relay);

#endif /* BISTABLE_RELAY_H_ */