
#ifndef NEXTION_H_
#define NEXTION_H_

#include "sam.h"
#include "kernel/kernel.h"


void nextion_init(const kernel_t *kernel);
void nextion_handler(const kernel_t *kernel);

void nextion_resetCom(void);

void nextion_sendCommand(const kernel_t *kernel, uint8_t *command);

void nextion_onTouchEvent_callback(const kernel_t *kernel, uint8_t page, uint8_t component); 


#endif /* NEXTION_H_ */