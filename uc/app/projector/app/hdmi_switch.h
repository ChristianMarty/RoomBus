#ifndef HDMI_MATRIX_SWITCH_H_
#define HDMI_MATRIX_SWITCH_H_

#include "sam.h"
#include "kernel/kernel.h"
#include "kernel/busController_IO.h"

void hdmi_init(const kernel_t *kernel, pin_port_t portNr, uint8_t pinNr);

void hdmi_handler(const kernel_t *kernel);

void hdmi_setMatrix(uint8_t out, uint8_t in);

void hdmi_turnOff(const kernel_t *kernel);

bool hdmi_getSwitchOn(const kernel_t *kernel);
bool hdmi_getSwitchOff(const kernel_t *kernel);


#endif /* HDMI_MATRIX_SWITCH_H_ */