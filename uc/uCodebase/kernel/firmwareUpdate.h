/*
 * firmwareUpdate.h
 *
 * Created: 07.10.2018 14:53:44
 *  Author: Christian
 */ 


#ifndef FIRMWAREUPDATE_H_
#define FIRMWAREUPDATE_H_

#include "sam.h"
#include "systemControl.h"

#ifdef __cplusplus
extern "C" {
#endif

void fu_eraseApp(uint8_t sourceAddress, uint8_t *data, uint8_t size);
void fu_writeAppData(uint8_t sourceAddress, uint8_t *data, uint8_t size);

void fu_handler(sysControlData_t *systemControl, sysStatusData_t *systemStatus, appState_t *appStatePtr);

#ifdef __cplusplus
}
#endif

#endif /* FIRMWAREUPDATE_H_ */