#ifndef HDMI_MATRIX_SWITCH_H_
#define HDMI_MATRIX_SWITCH_H_

#include "sam.h"
#include "kernel.h"
#include "necIr.h"

void hdmi_init(kernel_t *kernel, pin_port_t portNr, uint8_t pinNr);

void hdmi_handler(kernel_t *kernel);

void hdmi_setMatrix(uint8_t out, uint8_t in);

#endif /* HDMI_MATRIX_SWITCH_H_ */