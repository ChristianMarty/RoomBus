#ifndef PROJECTOR_H_
#define PROJECTOR_H_

#include "sam.h"
#include "kernel.h"

#include "busController2_IO .h"

typedef enum{
	projector_on,
	projector_off,
	projector_turnOn,
	projector_turnOff
} projector_onOff_state_t;

void projector_init(kernel_t *kernel);

void projector_handler(kernel_t *kernel);

void projector_onOff(bool on);

void projector_blank(bool on);

projector_onOff_state_t projector_getOnOff(void);

#endif /* PROJECTOR_H_ */