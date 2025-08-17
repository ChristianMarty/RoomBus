//**********************************************************************************************************************
// FileName : firmwareUpdate.c
// FilePath : kernel/
// Author   : Christian Marty
// Date		: 07.10.2018
// Website  : www.christian-marty.ch/RoomBus
//**********************************************************************************************************************
#ifndef FIRMWARE_UPDATE_H_
#define FIRMWARE_UPDATE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "systemControl.h"

void firmwareUpdate_handler(systemControl_t *sysControl);

void firmwareUpdate_eraseApp(uint8_t sourceAddress, uint8_t *data, uint8_t size);
void firmwareUpdate_writeAppData(uint8_t sourceAddress, uint8_t *data, uint8_t size);

#ifdef __cplusplus
}
#endif

#endif /* FIRMWARE_UPDATE_H_ */