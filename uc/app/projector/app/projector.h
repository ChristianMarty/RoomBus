#ifndef PROJECTOR_H_
#define PROJECTOR_H_

#include "sam.h"
#include "common/kernel.h"
#include "common/io_same51jx.h"

typedef enum{
	projector_on,
	projector_off,
	projector_turnOn,
	projector_turningOn,
	projector_turningOff,
	projector_turnOff,
	projector_switchOn,
	projector_switchOff
	
} projector_onOff_state_t;

void projector_init(void);

void projector_handler(void);

void projector_onOff(bool on);

void projector_blank(bool on);

bool projector_getBlank(void);
projector_onOff_state_t projector_getOnOff(void);

bool projector_getSwitchOn(void);
bool projector_getSwitchOff(void);

#endif /* PROJECTOR_H_ */