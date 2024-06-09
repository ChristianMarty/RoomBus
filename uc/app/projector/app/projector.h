#ifndef PROJECTOR_H_
#define PROJECTOR_H_

#include "sam.h"
#include "kernel/kernel.h"

#include "kernel/busController_IO.h"

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

void projector_init(const kernel_t *kernel);

void projector_handler(const kernel_t *kernel);

void projector_onOff(bool on);

void projector_blank(bool on);

bool projector_getBlank(void);
projector_onOff_state_t projector_getOnOff(void);

bool projector_getSwitchOn(void);
bool projector_getSwitchOff(void);

#endif /* PROJECTOR_H_ */