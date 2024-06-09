/*
 * system.h
 *
 * Created: 12.01.2019 21:38:44
 *  Author: Christian
 */ 


#ifndef SYSTEM_H_
#define SYSTEM_H_

#include "sam.h"

#ifdef __cplusplus
extern "C" {
#endif

void system_init(void);
void system_reboot(void);
void system_configure(void);

#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_H_ */