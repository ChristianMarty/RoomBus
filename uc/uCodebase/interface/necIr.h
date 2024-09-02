
#ifndef NEC_IR_H_
#define NEC_IR_H_

#include "sam.h"
#include "common/kernel.h"
#include "driver/SAMx5x/pin.h"


void necIR_init(pin_port_t portNr, uint8_t pinNr);

void necIR_transmitCode(uint8_t customCode, uint8_t dataCode);

bool necIR_busy(void);


#endif /* NEC_IR_H_ */