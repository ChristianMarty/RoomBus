#ifndef HDMI_MATRIX_SWITCH_H_
#define HDMI_MATRIX_SWITCH_H_

#include "sam.h"
#include "common/kernel.h"
#include "common/io_same51jx.h"

void hdmi_init(pin_port_t portNr, uint8_t pinNr);

void hdmi_handler(void);

void hdmi_setMatrix(uint8_t out, uint8_t in);

void hdmi_turnOff(void);

bool hdmi_getSwitchOn(void);
bool hdmi_getSwitchOff(void);


#endif /* HDMI_MATRIX_SWITCH_H_ */